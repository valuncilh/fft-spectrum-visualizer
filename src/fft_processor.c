#define _USE_MATH_DEFINES
#include "fft_processor.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

void generate_signal(double* signal, int size) {
    for (int i = 0; i < size; i++) {
        double t = (double)i / size;
        signal[i] = sin(2 * M_PI * 5 * t) + 0.5 * sin(2 * M_PI * 12 * t) + 0.3 * sin(2 * M_PI * 20 * t);
    }
}

void compute_fft(double* signal, int size, double* magnitudes) {
    // Прямое ДПФ (медленно, но надёжно)
    for (int k = 0; k < size / 2; k++) {
        double sum_real = 0.0;
        double sum_imag = 0.0;
        for (int n = 0; n < size; n++) {
            double angle = -2.0 * M_PI * k * n / size;
            sum_real += signal[n] * cos(angle);
            sum_imag += signal[n] * sin(angle);
        }
        magnitudes[k] = sqrt(sum_real * sum_real + sum_imag * sum_imag);
    }
}