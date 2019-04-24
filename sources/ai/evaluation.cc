#include "evaluation.h"
#include "synth/tinysynth.h"
#include "chips/mame_opn2.h"
#include "chips/nuked_opn2.h"
#include "chips/np2_opna.h"
#include "utility/music.h"
#include <algorithm>
#include <cstring>
#include <cmath>
#include <cassert>

namespace ai {

static constexpr unsigned num_filters = 50;
static constexpr unsigned num_coeffs = 16;
static constexpr double window_duration = 25e-3;
static constexpr double hop_duration = 10e-3;

typedef NP2OPNA<> DefaultOPN;
// typedef MameOPN2 DefaultOPN;
// typedef NukedOPN2 DefaultOPN;

Evaluation::Evaluation()
    : reference_(new_fvec(1))
{
    if (!reference_)
        throw std::bad_alloc();
}

void Evaluation::set_reference(fvec_u sound)
{
    reference_ = std::move(sound);
    update_reference_data();
}

void Evaluation::set_sample_rate(double sample_rate)
{
    if (sample_rate_ == sample_rate)
        return;

    sample_rate_ = sample_rate;
    update_reference_data();
}

void Evaluation::set_reference_note(unsigned reference_note)
{
    reference_note_ = reference_note;
}

double Evaluation::evaluate(const FmBank::Instrument &ins) const
{
    const fvec_t *ref = reference_.get();
    unsigned num_frames = ref->length;
    double sample_rate = sample_rate_;
    fvec_u test = generate(ins, num_frames, sample_rate, reference_note_);

    const std::vector<fvec_u> &ref_coeff = reference_data_;
    std::vector<fvec_u> test_coeff = compute_mfcc_coeffs(test.get(), sample_rate);

    size_t num_steps = test_coeff.size();
    assert(num_steps == ref_coeff.size());

    double total_err = 0;
    for (size_t i = 0; i < num_steps; ++i) {
        const smpl_t *ref_step = ref_coeff[i]->data;
        const smpl_t *test_step = test_coeff[i]->data;

        double err = 0;
        for (size_t j = 0; j < num_coeffs; ++j) {
            double dif = test_step[j] - ref_step[j];
            err += dif * dif;
        }

        total_err += err;
    }

    total_err /= num_steps; // average, not sure this is as intended

    double eval;
    double best = 1.0; // XXX check this
    if (total_err > 0) {
        eval = 1.0 / total_err;
        eval = std::min(best, eval);
    }
    else
        eval = best;

    return eval;
}

void Evaluation::update_reference_data()
{
    fvec_t *ref = reference_.get();

    reference_data_ = compute_mfcc_coeffs(ref, sample_rate_);
}

std::vector<fvec_u> Evaluation::compute_mfcc_coeffs(const fvec_t *in, double sample_rate)
{
    unsigned window_length = std::lround(window_duration * sample_rate);
    unsigned hop_length = std::lround(hop_duration * sample_rate);

    std::vector<fvec_u> result;
    result.reserve(1 + in->length / hop_length);

    aubio_mfcc_u mfcc(
        new_aubio_mfcc(window_length, num_filters, num_coeffs, sample_rate));
    if (!mfcc)
        throw std::runtime_error("Cannot create the MFCC object.");

    aubio_pvoc_u pv(new_aubio_pvoc(window_length, hop_length));
    if (!pv)
        throw std::runtime_error("Cannot create the Phase Vocoder object.");

    fvec_u frame(new_fvec(window_length));
    if (!frame)
        throw std::bad_alloc();

    cvec_u spec(new_cvec(window_length));
    if (!spec)
        throw std::bad_alloc();

    for (unsigned ref_pos = 0; ref_pos < in->length; ref_pos += hop_length) {
        fvec_u coeffs(new_fvec(num_coeffs));
        if (!coeffs)
            throw std::bad_alloc();

        unsigned count = std::min(window_length, in->length - ref_pos);
        std::copy(&in->data[ref_pos], &in->data[ref_pos + count], frame->data);
        std::fill(&frame->data[count], &frame->data[window_length], 0);

        aubio_pvoc_do(pv.get(), frame.get(), spec.get());
        aubio_mfcc_do(mfcc.get(), spec.get(), coeffs.get());

        result.push_back(std::move(coeffs));
    }

    return result;
}

fvec_u Evaluation::generate(const FmBank::Instrument &ins, unsigned num_frames, double sample_rate, unsigned note)
{
    fvec_u snd(new_fvec(num_frames));
    TinySynth synth;

    std::memset(&synth, 0, sizeof(TinySynth));

    OPNFamily family;
    switch ((unsigned)sample_rate) {
    case 53267:
        family = OPNChip_OPN2; break;
    case 55466:
        family = OPNChip_OPNA; break;
    default:
        throw std::runtime_error("Cannot find a chip model to match sample rate.");
    }

    DefaultOPN chip(family);
    chip.setRate((unsigned)sample_rate, opn2_getNativeClockRate(family));
    synth.m_chip = &chip;
    synth.m_notenum = note;
    synth.setInstrument(ins);
    synth.noteOn();

    std::unique_ptr<int16_t[]> temp(new int16_t[2 * num_frames]);
    synth.generate(temp.get(), num_frames);

    for (unsigned i = 0; i < num_frames; ++i)
        snd->data[i] = temp[2 * i] / 32768.0;

    #pragma message("XXX Remove this")
    if (0) {
        save_sound_file("/tmp/foobar.wav", snd.get(), sample_rate);
    }
    if (0) {
        FILE *fh = fopen("/tmp/foobar.dat", "w");
        for (unsigned i = 0; i < num_frames; ++i)
            fprintf(fh, "%u %d %d\n", i , temp[2 * i], temp[2 * i + 1]);
        fclose(fh);
    }

    return snd;
}

} // namespace ai
