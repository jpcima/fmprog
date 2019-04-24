#include "music.h"
#include <algorithm>
#include <stdexcept>
#include <cmath>

fvec_u load_sound_file(const char *filename, double *sample_rate)
{
    unsigned hop_size = 1024;

    aubio_source_u source(
        new_aubio_source(filename, 0, hop_size));
    if (!source)
        return nullptr;

    unsigned total_frames = aubio_source_get_duration(source.get());
    fvec_u snd_buf(new_fvec(total_frames));
    if (!snd_buf)
        throw std::bad_alloc();

    fvec_u hop_buf(new_fvec(hop_size));
    if (!hop_buf)
        throw std::bad_alloc();

    unsigned read_frames = 0;
    while (read_frames < total_frames) {
        unsigned count = 0;
        aubio_source_do(source.get(), hop_buf.get(), &count);
        if (count == 0)
            return nullptr;
        count = std::min(count, total_frames - read_frames);
        std::copy(hop_buf->data, &hop_buf->data[count], &snd_buf->data[read_frames]);
        read_frames += count;
    }

    if (sample_rate)
        *sample_rate = aubio_source_get_samplerate(source.get());

    return snd_buf;
}

bool save_sound_file(const char *filename, const fvec_t *sound, double sample_rate)
{
    aubio_sink_u sink(new_aubio_sink(filename, sample_rate));
    if (!sink)
        return false;

    const smpl_t *src = sound->data;
    unsigned src_size = sound->length;

    while (src_size > 0) {
        unsigned count = std::min(src_size, 1024u);

        fvec_u temp(new_fvec(count));
        if (!temp)
            throw std::bad_alloc();
        std::copy(src, src + count, temp->data);

        aubio_sink_do(sink.get(), temp.get(), count);
        src += count;
        src_size -= count;
    }

    return true;
}

fvec_u resample_sound(const fvec_t *in, double src_rate, double dst_rate)
{
    fvec_u out;
    double ratio = dst_rate / src_rate;
    aubio_resampler_u rsm(new_aubio_resampler(ratio, 0));
    if (!rsm)
        throw std::runtime_error("Cannot create the resampler object.");
    out.reset(new_fvec(std::ceil((double)in->length * ratio)));
    if (!out)
        throw std::bad_alloc();
    aubio_resampler_do(rsm.get(), in, out.get());
    return out;
}

unsigned detect_sound_pitch(const fvec_t *sound, double sample_rate)
{
    unsigned src_size = sound->length;

    aubio_pitch_u o(new_aubio_pitch("default", src_size, src_size, sample_rate));
    if (!o)
        throw std::runtime_error("Cannot create the pitch analysis object.");

    fvec_u out(new_fvec(1));
    if (!out)
        throw std::bad_alloc();

    aubio_pitch_do(o.get(), sound, out.get());
    double pitch = out->data[0];
    smpl_t midi = aubio_freqtomidi(pitch);

    unsigned key;
    if (midi <= 0)
        key = 69;
    else
        key = std::min<long>(127, std::lround(midi));
    return key;
}

std::string midi_note_to_string(int key)
{
    const char *notenames[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    int oct = key / 12;
    int notenum = key % 12;
    if (notenum < 0) notenum += 12;
    return notenames[notenum] + std::to_string(oct - 1);
}
