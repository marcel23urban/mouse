#ifndef CARRIERDETECTION_HPP
#define CARRIERDETECTION_HPP

#include <vector>
#include <complex>
#include <execution>

#include "dsp.hpp"
#include "fft.hpp"

/// versucht anhand Leistungsdetektion, Traeger zu erkennen.
class CarrierDetection
{
    struct Peak {
        uint64_t pos_left;
        uint64_t pos_right;
        float magnitude;
    };

    uint64_t _psd_leng, _psd_avg, _threshold_db;
    Psd _psd;
    std::vector<uint64_t> _channel_id;
    std::vector<std::complex<float>> _fft_buffer;
    std::vector<float> _psd_buffer;
    uint64_t _psd_cnt;

public:
    ///@brief
    /// @param psd_leng determines the fft_leng
    /// @param psd_avg amount of overlayed psds for calculating mean
    /// @param threshold_db peak over sourounding area
    CarrierDetection( uint64_t psd_leng, uint64_t psd_avg, uint64_t threshold_db = 6.)
        : _psd_leng( psd_leng), _psd_avg( psd_avg),  _threshold_db( threshold_db), _psd_cnt( 0){
        _psd.setLeng( _psd_leng);
		
    }

    void push( const std::vector<std::complex<float>> &input) {
        _psd.add( input, _psd_buffer);
        if( ++_psd_cnt > 8) {
            checkCarrier();


        }
    }

private:
    void checkCarrier() {
		
	}

};

#endif // CARRIERDETECTION_HPP
