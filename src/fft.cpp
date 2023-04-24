#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <QtMath>
#include "fft.h"

using std::complex;
using std::size_t;
using std::uintmax_t;
using std::vector;


static size_t reverseBits(size_t val, int width);


void fft(vector<complex<double> > &vec, bool inverse) {
    size_t n = vec.size();
    if (n == 0)
        return;
    else if ((n & (n - 1)) == 0)  // Is power of 2
        fftRadix2(vec, inverse);
    else
        fftBluestein(vec, inverse);
}


void fftRadix2(vector<complex<double> > &vec, bool inverse) {
    size_t n = vec.size();
    int levels = 0;
    for (size_t temp = n; temp > 1U; temp >>= 1)
        levels++;
    if (static_cast<size_t>(1U) << levels != n)
        throw std::domain_error("Length is not a power of 2");

    vector<complex<double> > expTable(n / 2);
    for (size_t i = 0; i < n / 2; i++)
        expTable[i] = std::polar(1.0, (inverse ? 2 : -2) * M_PI * i / n);

    for (size_t i = 0; i < n; i++) {
        size_t j = reverseBits(i, levels);
        if (j > i)
            std::swap(vec[i], vec[j]);
    }

    for (size_t size = 2; size <= n; size *= 2) {
        size_t halfsize = size / 2;
        size_t tablestep = n / size;
        for (size_t i = 0; i < n; i += size) {
            for (size_t j = i, k = 0; j < i + halfsize; j++, k += tablestep) {
                complex<double> temp = vec[j + halfsize] * expTable[k];
                vec[j + halfsize] = vec[j] - temp;
                vec[j] += temp;
            }
        }
        if (size == n)
            break;
    }
}


void fftBluestein(vector<complex<double> > &vec, bool inverse) {
    size_t n = vec.size();
    size_t m = 1;
    while (m / 2 <= n) {
        if (m > SIZE_MAX / 2)
            throw std::length_error("Vector too large");
        m *= 2;
    }

    vector<complex<double> > expTable(n);
    for (size_t i = 0; i < n; i++) {
        uintmax_t temp = static_cast<uintmax_t>(i) * i;
        temp %= static_cast<uintmax_t>(n) * 2;
        double angle = (inverse ? M_PI : -M_PI) * temp / n;
        expTable[i] = std::polar(1.0, angle);
    }

    vector<complex<double> > avec(m);
    for (size_t i = 0; i < n; i++)
        avec[i] = vec[i] * expTable[i];
    vector<complex<double> > bvec(m);
    bvec[0] = expTable[0];
    for (size_t i = 1; i < n; i++)
        bvec[i] = bvec[m - i] = std::conj(expTable[i]);

    vector<complex<double> > cvec = convolve(std::move(avec), std::move(bvec));

    for (size_t i = 0; i < n; i++)
        vec[i] = cvec[i] * expTable[i];
}


vector<complex<double> > convolve(
    vector<complex<double> > xvec,
    vector<complex<double> > yvec) {

    size_t n = xvec.size();
    if (n != yvec.size())
        throw std::domain_error("Mismatched lengths");
    fft(xvec, false);
    fft(yvec, false);
    for (size_t i = 0; i < n; i++)
        xvec[i] *= yvec[i];
    fft(xvec, true);
    for (size_t i = 0; i < n; i++)
        xvec[i] /= static_cast<double>(n);
    return xvec;
}


static size_t reverseBits(size_t val, int width) {
    size_t result = 0;
    for (int i = 0; i < width; i++, val >>= 1)
        result = (result << 1) | (val & 1U);
    return result;
}
