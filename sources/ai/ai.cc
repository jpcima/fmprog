#include "ai.h"
#include "instrument/metaparameter.h"
#include <algorithm>
#include <random>

namespace ai {

Individual Individual::create_random()
{
    Individual x;
    FmBank::Instrument &ins = x.ins_;

    std::mt19937_64 prng{std::random_device{}()};

    for (const MetaParameter &mp : MP_instrument) {
        if (mp.flags & MP_NotAIFeature)
            continue;

        std::uniform_int_distribution<int> dist(mp.min, mp.max);

        mp.set(ins, dist(prng));
    }

    return x;
}

Population Population::create_empty(size_t capacity)
{
    Population pop;
    pop.size_ = 0;
    pop.capacity_ = capacity;
    pop.members_.reset(new Individual[capacity]);
    pop.evaluation_.reset(new double[capacity]());
    pop.status_.reset(new Status[capacity]());
    return pop;
}

Population Population::create_random(size_t size)
{
    Population pop = create_empty(size);
    for (size_t i = 0; i < size; ++i)
        pop.replace_member(i, Individual::create_random());
    return pop;
}

size_t Population::add_member(const Individual &ind)
{
    size_t size = size_;

    if (size == capacity_)
        return ~(size_t)0;

    #pragma message("XXX inefficient")
    size_t index = std::distance(&status_[0],
        std::find(&status_[0], &status_[size], Absent));

    size_ = size + 1;
    members_[index] = ind;
    status_[index] = Unevaluated;
    return index;
}

void Population::replace_member(size_t index, const Individual &ind)
{
    if (status_[index] == Absent)
        ++size_;

    status_[index] = Unevaluated;
    members_[index] = ind;
}

bool Population::remove_member(size_t index)
{
    if (status_[index] == Absent)
        return false;

    status_[index] = Absent;
    --size_;
    return true;
}

const Individual *Population::get_member(size_t index)
{
    if (status_[index] == Absent)
        return nullptr;
    return &members_[index];
}

void Population::clear_evaluation()
{
    for (size_t i = 0, n = size_; i < n; ++i) {
        if (status_[i] != Absent)
            status_[i] = Unevaluated;
    }
}

bool Population::set_evaluation(size_t index, double ev)
{
    if (status_[index] == Absent)
        return false;

    status_[index] = Evaluated;
    evaluation_[index] = ev;
    return true;
}

double Population::get_evaluation(size_t index)
{
    if (status_[index] != Evaluated)
        return 0;

    return evaluation_[index];
}

} // namespace ai
