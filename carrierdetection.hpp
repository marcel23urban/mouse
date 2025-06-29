#ifndef CARRIERDETECTION_HPP
#define CARRIERDETECTION_HPP

#include <vector>
#include <complex>
#include <execution>
#include <chrono>

#include "conditionalsafequeue.hpp"
#include "dsp.hpp"
#include "fft.hpp"
#include "peakdetection.hpp"


/// @brief holds one signal and its metadata
struct Carrier {
    double origin_freq;
    double samp_rate;
    double band_width;
    std::time_t start_time;
    std::vector<std::complex<float>> buffer;
};



/// Carrierdetection .
class CarrierDetection {

public:
    ///@brief
    /// @param psd_leng determines the fft_leng
    /// @param psd_avg amount of overlayed psds for calculating mean
    /// @param threshold_db peak over sourounding area
    CarrierDetection( uint64_t psd_leng, uint64_t psd_avg, uint64_t threshold_db = 6.)
        : _psd_leng( psd_leng), _psd_avg( psd_avg),  _threshold_db( threshold_db), _psd_cnt( 0){
        _psd.setLeng( _psd_leng);

    }

    // @brief add Data to internal buffer and starts detection
    void dataIn( const std::vector<std::complex<float>> &input) {
        _puff.push( input);
    }

    /// @brief return Peaks if exists
    std::vector<Peak> getPeaks() const { return _peaks;}

private:
    // poll input_buffer, add to sample_buffer, consume as long as...
    void process() {
        std::vector<std::complex<float>> data( _fft.leng()), fft( _fft.leng());
        std::vector<float> psd( _fft.leng());

        while( _is_processing) {
            // pull samples from receive buffer
            _puff.try_pop( data);
            // append to logical structure
            _samples.insert( _samples.end(), data.begin(), data.end());
            data.clear();

            uint64_t consumed_samples = 0;
            while(( _samples.size() - consumed_samples) >= _fft.leng()) {
                _fft.fft( _samples.data() + consumed_samples, fft.data());

                std::transform( std::execution::par_unseq, fft.begin(), fft.end(),
                               psd.begin(), []( const std::complex<float> &samp)
                               { return std::real( std::norm( samp));});

                checkCarrier( psd);
                consumed_samples += _overl_step;
            }
            // discard all consumed samples
            _samples.erase( _samples.begin(), _samples.begin() + consumed_samples);

            c
        }
    }

    void checkCarrier( const std::vector<float> &input) {
        std::vector<std::tuple<float, uint64_t, uint64_t>> peaks;
        findPeaks( _psd_buffer, peaks);
        if( peaks.empty()) return;

        // for( auto peak : peaks)
        //     std::cerr << std::get<>

        // check for previsous peaks in range and compare IDs

    }

    /// @brief extract time signal, therefor estimate extraction fft_leng and low-pass filter
    void extractCarriers( std::vector<std::complex<float>> &input, const std::vector<Carrier> &carriers, uint64_t rel_invers_overlap) {

        const uint64_t fft_leng = input.size();
        for( const Carrier &carrier : carriers) {
            // estimate extract_fft_leng of necessary extraction window with the following requirements
            // - rel_inverse_overlap is divider,
            uint64_t extract_fft_leng = Tools::nextPow2( std::ceil( static_cast<double>( fft_leng) * 1.25 * carrier.band_width * 2.));
            while( extract_fft_leng < fft_leng && fft_leng % extract_fft_leng != 0 && fft_leng % rel_invers_overlap != 0) {
                ++extract_fft_leng;
            }
            double rel_extraction_samp_rate = static_cast<double>( extract_fft_leng) / static_cast<double>( fft_leng);

            //            std::iterator<uint64_t> beg = carrier.rel_freq * input.size() - std::ceil( .5 * carrier)
            FFT fft( extract_fft_leng);

        }



    }


    uint64_t _psd_cnt, _psd_leng, _psd_avg, _threshold_db, _rel_inv_overl,
        _overl_step;

    uint64_t _psd_cnt, _psd_leng, _psd_avg, _threshold_db;
    Psd _psd;
    std::vector<uint64_t> _channel_id;
    std::vector<std::complex<float>> _samples;
    std::vector<float> _psd_buffer;
    std::vector<struct Peak> _peaks;
    std::atomic_bool _is_processing;
    ConditionSafeQueue<std::complex<float>> _puff;

    FFT _fft;
    LowPassFilter _lpf;
};

#endif // CARRIERDETECTION_HPP
