#include "algorithm.h"
#include "evaluation.h"
#include "ai.h"
#include "instrument/metaparameter.h"
#include <random>

namespace ai {

GeneticAlgorithm::GeneticAlgorithm()
{
    GeneticData *gdata = new GeneticData;
    gdata_.reset(gdata);
    gdata->eval_.reset(new Evaluation);
    gdata->population_.reset(
        new Population(Population::create_random(GeneticData::population_size)));
}

GeneticAlgorithm::~GeneticAlgorithm()
{
    stop();
}

void GeneticAlgorithm::set_generation_callback(GenCallback callback)
{
    gen_callback_ = callback;
}

void GeneticAlgorithm::set_fitness_callback(FitCallback callback)
{
    fit_callback_ = callback;
}

GeneticData &GeneticAlgorithm::lock(std::unique_lock<std::mutex> &lock)
{
    lock = std::unique_lock<std::mutex>(gmutex_);
    return *gdata_;
}

void GeneticAlgorithm::start()
{
    if (thread_.joinable())
        return;

    quit_ = false;
    pause_ = false;
    thread_ = std::thread([this]() { exec(); });
}

void GeneticAlgorithm::stop()
{
    if (!thread_.joinable())
        return;

    quit_ = true;
    set_paused(false);
    thread_.join();
}

bool GeneticAlgorithm::set_paused(bool p)
{
    std::lock_guard<std::mutex> lock(pause_mutex_);
    bool was_paused = pause_;
    pause_ = p;
    pause_cond_.notify_one();
    return was_paused;
}

void GeneticAlgorithm::toggle_paused()
{
    std::lock_guard<std::mutex> lock(pause_mutex_);
    pause_ = !pause_;
    pause_cond_.notify_one();
}

void GeneticAlgorithm::reinitialize()
{
    std::unique_ptr<Population> pop(
        new Population(Population::create_random(GeneticData::population_size)));

    bool was_paused = set_paused(true);
    std::lock_guard<std::mutex> lock(gmutex_);
    gdata_->population_ = std::move(pop);
    gdata_->generation_num_ = 0;
    set_paused(was_paused);
}

void GeneticAlgorithm::exec()
{
    volatile bool *quit = &quit_;
    volatile bool *pause = &pause_;
    GeneticData &gdata = *gdata_;

    std::mt19937_64 prng{std::random_device{}()};
    static constexpr unsigned pop_size = GeneticData::population_size;

    ai::Individual fittest_ind;

    while (!*quit) {
        if (*pause) {
            std::unique_lock<std::mutex> lock(pause_mutex_);
            if (*pause) {
                pause_cond_.wait(lock);
                continue;
            }
        }

        std::unique_lock<std::mutex> lock(gmutex_);

        ai::Population &pop = *gdata.population_;
        ai::Evaluation &eval = *gdata.eval_;
        size_t generation_num = gdata.generation_num_;

        /* Evaluation */
        #pragma omp parallel for
        for (unsigned i = 0; i < pop_size; ++i) {
            if (!*quit && pop.get_status(i) != Population::Evaluated) {
                const ai::Individual &ind = *pop.get_member(i);
                pop.set_evaluation(i, eval.evaluate(ind.ins_));
            }
        }

        if (*quit)
            return;

        /* Fitness */
        double fitness[GeneticData::population_size];
        unsigned fittest_index = 0;
        {
            double avg = 0;
            for (unsigned i = 0; i < pop_size; ++i)
                avg += pop.get_evaluation(i);
            avg /= GeneticData::population_size;

            for (unsigned i = 0; i < pop_size; ++i) {
                double f = pop.get_evaluation(i) / avg;
                fitness[i] = f;
                if (f > fitness[fittest_index])
                    fittest_index = i;
            }
        }
        fittest_ind = *pop.get_member(fittest_index);

        if (fit_callback_)
            fit_callback_(generation_num, fitness, GeneticData::population_size);

        if (*quit)
            return;

        /* Selection */
        {
            Population next = Population::create_empty(pop_size);
            {
                for (unsigned i = 0; i < pop_size; ++i) {
                    double f = fitness[i];
                    if (f >= 1.0) {
                        next.replace_member(i, *pop.get_member(i));
                        next.set_evaluation(i, pop.get_evaluation(i));
                    }
                }
                for (unsigned i = 0; i < pop_size; ++i) {
                    for (double f = fitness[i]; f > 1.0 && !next.full(); f -= 1.0) {
                        double p = std::uniform_real_distribution<double>(0.0, 1.0)(prng);
                        if (p < f) {
                            size_t index = next.add_member(*pop.get_member(i));
                            next.set_evaluation(index, pop.get_evaluation(i));
                        }
                    }
                }
                if (next.empty()) {
                    /* Kill */
                    pop = Population::create_random(pop_size);
                    gdata.generation_num_ = generation_num + 1;
                    continue;
                }
            }
            pop = std::move(next);
        }

        if (*quit)
            return;

        /* Recombination */
        if (!pop.full()) {
            size_t num_selected = 0;
            const Individual *selected[GeneticData::population_size];

            for (size_t i = 0; i < GeneticData::population_size; ++i) {
                if (const Individual *member = pop.get_member(i))
                    selected[num_selected++] = member;
            }

            for (size_t i = 0; i < GeneticData::population_size; ++i) {
                if (pop.get_member(i))
                    continue;

                const ai::Individual &i1 = *selected[std::uniform_int_distribution<size_t>(0, num_selected - 1)(prng)];
                const ai::Individual &i2 = *selected[std::uniform_int_distribution<size_t>(0, num_selected - 1)(prng)];
                ai::Individual combined = i1;

                for (const MetaParameter &mp : MP_instrument) {
                    if (mp.flags & MP_NotAIFeature)
                        continue;
                    int val1 = mp.get(i1.ins_);
                    int val2 = mp.get(i2.ins_);
                    if (mp.flags & MP_Bits) {
                        /* Bitwise recombination */
                        unsigned v = 0;
                        for (unsigned bit = 1, max = mp.max; bit <= max; bit <<= 1) {
                            double p = std::uniform_real_distribution<double>(0.0, 1.0)(prng);
                            v |= (p < 0.5) ? (val1 & bit) : (val2 & bit);
                        }
                        mp.set(combined.ins_, v);
                    }
                    else {
                        /* Linear recombination */
                        double p = std::uniform_real_distribution<double>(0.0, 1.0)(prng);
                        double v = val1 * p + val2 * (1 - p);
                        mp.set(combined.ins_, std::lround(v));
                    }
                }

                pop.replace_member(i, combined);
            }
        }

        if (*quit)
            return;

        /* Mutation */
        for (unsigned i = 0; i < pop_size; ++i) {
            ai::Individual ind = *pop.get_member(i);

            for (const MetaParameter &mp : MP_instrument) {
                if (mp.flags & MP_NotAIFeature)
                    continue;

                unsigned random = std::uniform_int_distribution<unsigned>(0, 99)(prng);
                if (random != 0)
                    continue;

                mp.set(ind.ins_, std::uniform_int_distribution<unsigned>(mp.min, mp.max)(prng));
            }

            pop.replace_member(i, ind);
        }

        if (*quit)
            return;

        if (gen_callback_)
            gen_callback_(generation_num, fittest_ind);

        gdata.generation_num_ = generation_num + 1;
    }
}

} // namespace ai
