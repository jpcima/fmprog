#pragma once
#include "instrument/bank.h"
#include <vector>
#include <memory>
#include <cstdint>

namespace ai {

struct Individual
{
    FmBank::Instrument ins_ = FmBank::emptyInst();

    static Individual create_random();
};

struct Population
{
    static Population create_empty(size_t capacity);
    static Population create_random(size_t size);

    size_t size() const noexcept { return size_; }
    size_t capacity() const noexcept { return capacity_; }
    bool empty() const noexcept { return size_ == 0; }
    bool full() const noexcept { return size_ == capacity_; }

    size_t add_member(const Individual &ind);
    void replace_member(size_t index, const Individual &ind);
    bool remove_member(size_t index);
    const Individual *get_member(size_t index);

    void clear_evaluation();
    bool set_evaluation(size_t index, double ev);
    double get_evaluation(size_t index);

    enum Status : uint8_t {
        Absent = 0, Unevaluated, Evaluated,
    };

    Status get_status(size_t index) const noexcept { return status_[index]; }

private:
    size_t size_ = 0;
    size_t capacity_ = 0;
    std::unique_ptr<Individual[]> members_;
    std::unique_ptr<double[]> evaluation_;
    std::unique_ptr<Status[]> status_;
};

} // namespace ai
