#pragma once
#include <aubio/aubio.h>
#include <memory>

#define AUBIO_TYPE_WRAP(u, t, d)                        \
    struct t##_deleter {                                \
        void operator()(t *x) const noexcept { d(x); }  \
    };                                                  \
    typedef std::unique_ptr<t, t##_deleter> u;

AUBIO_TYPE_WRAP(fvec_u, fvec_t, del_fvec)
AUBIO_TYPE_WRAP(cvec_u, cvec_t, del_cvec)
AUBIO_TYPE_WRAP(aubio_source_u, aubio_source_t, del_aubio_source)
AUBIO_TYPE_WRAP(aubio_sink_u, aubio_sink_t, del_aubio_sink)
AUBIO_TYPE_WRAP(aubio_resampler_u, aubio_resampler_t, del_aubio_resampler)
AUBIO_TYPE_WRAP(aubio_pitch_u, aubio_pitch_t, del_aubio_pitch)
AUBIO_TYPE_WRAP(aubio_fft_u, aubio_fft_t, del_aubio_fft)
AUBIO_TYPE_WRAP(aubio_pvoc_u, aubio_pvoc_t, del_aubio_pvoc)
AUBIO_TYPE_WRAP(aubio_mfcc_u, aubio_mfcc_t, del_aubio_mfcc)

#undef AUBIO_TYPE_WRAP
