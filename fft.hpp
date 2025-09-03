#ifndef FFT_HPP
#define FFT_HPP

#include "pocketfft_hdronly.h"

/// @brief Wrapper for one-dimensional transformation (c2c, r2c)
class FFT {

    pocketfft::shape_t _shape;             // Shape of the transform
    const pocketfft::stride_t stride_in = {sizeof(std::complex<float>)};   // Stride for input array
    const pocketfft::stride_t stride_out = {sizeof( std::complex<float>)};  // Stride for output array
    const pocketfft::stride_t stride_in_float = { sizeof( float)}
    pocketfft::shape_t axes = {0};              // Axes to transform
public:
    FFT( uint64_t leng = 1024) : _shape( {leng}) {
        setLeng( leng);
    }

    /// @brief return current fft length
    uint64_t leng() const { return _shape.at(0);}

    /// @brief set new fft length
    void setLeng( uint64_t leng) {
        if( leng < 2) throw std::invalid_argument("FEHLER fft leng < 2");
        _shape = { leng};
    }

    void fft( const std::complex<float> *input, std::complex<float> *output) {
        pocketfft::c2c<float>( _shape, stride_in, stride_out, axes, pocketfft::FORWARD,
                              input, output, 1.);
    }
    void fft( const std::vector<std::complex<float>> &input, std::vector<std::complex<float>> &output) {
        fft( input.data(), output.data());
    }

    /// @brief Computes vector of floats to its complex frequency domain
    void fft( const float *input, std::complex<float> *output) {
        pocketfft::r2c<flaot>( _shape, stride_in, stride_out, 0, pocketfft::FORWARD, input, output, 1.);
    }
    /// @brief Computes vector of floats to its complex frequency domain
    void fft( const std::vector<float> &input, std::vector<std::complex<float>> &output) {
        fft( input.data(), output.data());
    }
   


    /// @brief performs complex<float> to complex<float> reverse-fft
    /// @param input data of exact leng samples
    /// @param output fft with input leng
    void ifft( const std::complex<float> *input, std::complex<float> *output) {
        pocketfft::c2c<float>( _shape, stride_in, stride_out, axes, pocketfft::BACKWARD,
                              input, output, 1. / static_cast<double>( leng));
    }
    /// @brief performs complex<float> to complex<float> reverse-fft
    /// @param input data of exact leng samples
    /// @param output fft with input leng
    void ifft( const std::vector<std::complex<float>> &input, std::vector<std::complex<float>> &output) {
        ifft( input.data(), output.data());
    }
};

#endif // FFT_HPP
