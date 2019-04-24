#pragma once
#include "aubio++.h"
#include <string>

fvec_u load_sound_file(const char *filename, double *sample_rate);
bool save_sound_file(const char *filename, const fvec_t *sound, double sample_rate);
fvec_u resample_sound(const fvec_t *in, double src_rate, double dst_rate);
unsigned detect_sound_pitch(const fvec_t *sound, double sample_rate);
std::string midi_note_to_string(int key);
