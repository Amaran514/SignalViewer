
#ifndef FFT_H
#define FFT_H

#include <complex>
#include <vector>

void fft(std::vector<std::complex<double> > &vec, bool inverse);

void fftRadix2(std::vector<std::complex<double> > &vec, bool inverse);

void fftBluestein(std::vector<std::complex<double> > &vec, bool inverse);

std::vector<std::complex<double> > convolve(
    std::vector<std::complex<double> > xvec,
    std::vector<std::complex<double> > yvec);

#endif // FFT_H
