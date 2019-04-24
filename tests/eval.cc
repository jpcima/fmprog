#include "file-formats/format_wohlstand_opn2.h"
#include "ai/evaluation.h"
#include "utility/music.h"
#include "utility/aubio++.h"
#include <memory>
#include <cstdio>
#include <cmath>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: eval <opni-file> <audio-file>\n");
        return 1;
    }

    FmBank::Instrument ins;
    if (WohlstandOPN2().loadFileInst(QString::fromLocal8Bit(argv[1]), ins) != FfmtErrCode::ERR_OK) {
        fprintf(stderr, "Cannot load instrument file.\n");
        return 1;
    }

    double file_sample_rate = 0;
    fvec_u file_sound = load_sound_file(argv[2], &file_sample_rate);
    if (!file_sound) {
        fprintf(stderr, "Cannot load sound file.\n");
        return 1;
    }

    unsigned key = detect_sound_pitch(file_sound.get(), file_sample_rate);
    fprintf(stderr, "Key (detected): %s\n", midi_note_to_string(key).c_str());

    unsigned clock = 7670454;
    //unsigned clock = 7987200;
    double sample_rate = clock / 144.0;

    /* Resample */
    ai::Evaluation eval;
    fvec_t *ref_sound = resample_sound(file_sound.get(), file_sample_rate, sample_rate).release();
    eval.set_reference(fvec_u(ref_sound));
    eval.set_sample_rate(sample_rate);
    eval.set_reference_note(key);

    double score = eval.evaluate(ins);
    printf("%f\n", score);

    #pragma message("XXX Remove this")
    if (0) {
        fvec_u syn_sound = ai::Evaluation::generate(ins, ref_sound->length, sample_rate, key);
        if (1) {
            FILE *fh = fopen("/tmp/sound.dat", "w");
            for (unsigned i = 0, n = ref_sound->length; i < n; ++i)
                fprintf(fh, "%u %f %f\n", i, ref_sound->data[i], syn_sound->data[i]);
            fclose(fh);
        }
        if (1) {
            std::vector<fvec_u> ref_coeffs = ai::Evaluation::compute_mfcc_coeffs(ref_sound, sample_rate);
            std::vector<fvec_u> syn_coeffs = ai::Evaluation::compute_mfcc_coeffs(syn_sound.get(), sample_rate);
            unsigned num_windows = ref_coeffs.size();
            FILE *fh = fopen("/tmp/mfcc.dat", "w");
            for (unsigned idx_window = 0; idx_window < num_windows; ++idx_window) {
                unsigned num_coeffs = ref_coeffs[idx_window]->length;
                for (unsigned idx_coeff = 0; idx_coeff < num_coeffs; ++idx_coeff)
                    fprintf(fh, "%u %u %f %f\n", idx_window, idx_coeff,
                            ref_coeffs[idx_window]->data[idx_coeff],
                            syn_coeffs[idx_window]->data[idx_coeff]);
            }
            fclose(fh);
        }
    }

    return 0;
}
