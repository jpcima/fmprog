#pragma once
#include "ai.h"

namespace ai {

class Evaluation;

struct GeneticData
{
    static constexpr size_t population_size = 100;

    std::unique_ptr<ai::Evaluation> eval_;
    std::unique_ptr<ai::Population> population_;
    size_t generation_num_ = 0;
};

//
struct FitnessRecord
{
    double data[GeneticData::population_size];
};

} // namespace ai
