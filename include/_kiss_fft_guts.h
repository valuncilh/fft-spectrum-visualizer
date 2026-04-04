#ifndef KISS_FFT_GUTS_H
#define KISS_FFT_GUTS_H

#include "kiss_fft.h"
#include <limits.h>

#define MAXFACTORS 32

struct kiss_fft_state {
    int nfft;
    int inverse;
    int factors[2 * MAXFACTORS];
    kiss_fft_cpx twiddles[1];
};

// Определение S_MUL для floating-point
#define S_MUL(a, b) ((a) * (b))

#define C_MUL(m, a, b) \
    do { (m).r = (a).r * (b).r - (a).i * (b).i; \
         (m).i = (a).r * (b).i + (a).i * (b).r; } while (0)

#define C_FIXDIV(c, div) /* nothing for floating point */
#define C_ADD(res, a, b) (res).r = (a).r + (b).r; (res).i = (a).i + (b).i
#define C_SUB(res, a, b) (res).r = (a).r - (b).r; (res).i = (a).i - (b).i
#define C_ADDTO(res, a) (res).r += (a).r; (res).i += (a).i
#define C_SUBFROM(res, a) (res).r -= (a).r; (res).i -= (a).i

#define KISS_FFT_COS(phase) (float)cos(phase)
#define KISS_FFT_SIN(phase) (float)sin(phase)
#define HALF_OF(x) ((x) * 0.5f)

#define kf_cexp(x, phase) \
    do { (x)->r = KISS_FFT_COS(phase); \
         (x)->i = KISS_FFT_SIN(phase); } while (0)

#endif