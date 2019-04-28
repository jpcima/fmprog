#pragma once
#include "algorithm_data.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <functional>

namespace ai {

struct GeneticData;
struct FitnessRecord;
struct Population;
struct Individual;
class Evaluation;

//
class GeneticAlgorithm
{
public:
    GeneticAlgorithm();
    ~GeneticAlgorithm();

    typedef std::function<void(size_t, const Individual &)> GenCallback;
    typedef std::function<void(size_t, const FitnessRecord &)> FitCallback;

    void set_generation_callback(GenCallback callback);
    void set_fitness_callback(FitCallback callback);

    GeneticData &lock(std::unique_lock<std::mutex> &lock);
    void start();
    void stop();
    bool set_paused(bool p);
    void toggle_paused();
    void reinitialize();

private:
    void exec();

private:
    std::unique_ptr<GeneticData> gdata_;
    std::mutex gmutex_;
    std::thread thread_;
    GenCallback gen_callback_;
    FitCallback fit_callback_;
    bool quit_ = false;
    bool pause_ = false;
    std::condition_variable pause_cond_;
    std::mutex pause_mutex_;
};

} // namespace ai
