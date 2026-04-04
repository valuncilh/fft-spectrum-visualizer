#ifndef FFT_PROCESSOR_H
#define FFT_PROCESSOR_H

void generate_signal(double* signal, int size);
void compute_fft(double* signal, int size, double* magnitudes);

#endif