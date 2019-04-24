#pragma once
#include "instrument/bank.h"
#include "utility/aubio++.h"
#include <vector>

namespace ai {

class Evaluation
{
public:
    Evaluation();

    void set_reference(fvec_u sound);
    void set_sample_rate(double sample_rate);
    void set_reference_note(unsigned reference_note);

    double evaluate(const FmBank::Instrument &ins) const;

    static fvec_u generate(const FmBank::Instrument &ins, unsigned num_frames, double sample_rate, unsigned note);
    static std::vector<fvec_u> compute_mfcc_coeffs(const fvec_t *in, double sample_rate);

    const fvec_t &reference() const noexcept { return *reference_; }
    double sample_rate() const noexcept { return sample_rate_; }
    unsigned reference_note() const noexcept { return reference_note_; }

private:
    void update_reference_data();

private:
    fvec_u reference_;
    double sample_rate_ = 44100;
    unsigned reference_note_ = 69;
    std::vector<fvec_u> reference_data_;
};

} // namespace ai
