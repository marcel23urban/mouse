#ifndef FFT_HPP
#define FFT_HPP

#include "pocketfft_hdronly.h"

class FFT {

    pocketfft::shape_t _shape;             // Shape of the transform
    const pocketfft::stride_t stride_in = {sizeof(std::complex<float>)};   // Stride for input array
    const pocketfft::stride_t stride_out = {sizeof( std::complex<float>)};  // Stride for output array
    pocketfft::shape_t axes = {0};              // Axes to transform
public:
    FFT( uint64_t leng = 1024) : _shape( {leng}) {
        setLeng( leng);
    }

    /// @brief performs complex<float> fft
    /// @param input einput data of exact leng samples
    /// @param output fft from input

    uint64_t leng() const { return _shape.at(0);}
    bool setLeng( uint64_t leng) {
        if( leng < 2) throw std::invalid_argument("FEHLER fft leng < 2");
        _shape = { leng};
        return true;
    }

    void fft( const std::complex<float> *input, std::complex<float> *output) {
        pocketfft::c2c<float>( _shape, stride_in, stride_out, axes, pocketfft::FORWARD,
                              input, output, 1.);
    }
    void fft( const std::vector<std::complex<float>> &input, std::vector<std::complex<float>> &output) {
        fft( input.data(), output.data());
    }


    /// @brief performs complex<float> reverse-fft
    /// @param input einput data of exact leng samples
    /// @param output fft from input
    void ifft( const std::complex<float> *input, std::complex<float> *output) {
        pocketfft::c2c<float>( _shape, stride_in, stride_out, axes, pocketfft::BACKWARD,
                              input, output, 1.);
    }
    void ifft( const std::vector<std::complex<float>> &input, std::vector<std::complex<float>> &output) {
        ifft( input.data(), output.data());
    }


};

#endif // FFT_HPP
