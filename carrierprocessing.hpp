#ifndef CARRIERPROCESSING_H
#define CARRIERPROCESSING_H

#include <vector>
#include <complex>
#include <execution>
#include <chrono>

// provides Interface
#include "baseprocessor.hpp"

#include "dsp.hpp"
#include "fft.hpp"
#include "fftwindows.hpp"
#include "peakdetection.hpp"

#ifndef DEBUG
#define DEBUG
#endif


/// @brief holds one signal and its metadata
struct Carrier {
    double origin_freq;     // [Hz]
    double rel_freq;        // bins
    double samp_rate;       // [HZ]
    double band_width;      // [Hz]
    double rel_band_width;  // bins
    std::time_t start_time;
    std::vector<std::complex<float>> samples;
};



/// @brief Carrierdetection inherits from threaded BaseProcessor
///        ongoing in function process()
class CarrierDetection : public BaseProcessor {

public:
    ///@brief
    /// @param psd_leng determines the fft_leng
    /// @param psd_avg amount of overlayed psds for calculating mean
    /// @param threshold_db peak over sourounding area
    CarrierDetection( uint64_t psd_leng, uint64_t psd_avg, uint64_t threshold_db = 6.)
        : _psd_leng( psd_leng), _psd_avg( psd_avg),  _threshold_db( threshold_db), _psd_cnt( 0){
        _fft.setLeng( psd_leng);
    }

    // @brief add Data to internal buffer and starts detection
    void dataIn( const std::vector<std::complex<float>> &input) {
        _puff.push( input);
    }

    /// @brief return Peaks if exists
    std::vector<Carrier> getPeaks() const { return _carriers;}

private:
    /// @brief Verarbeitet Daten aus dem Puffer _puff:
    ///        fft transformieren, jeweils für die Leistungsanalyse
    ///        und für die Extraktion
    void process( const std::vector<std::complex<float>> &data) override {
        // append to logical structure
        _buffer.insert( _buffer.end(), data.begin(), data.end());

        uint64_t buffer_consumed = 0;
        while( _buffer.size() - buffer_consumed >= _fft.leng()) {
            _win.vonHannWindow( _buffer.data() + buffer_consumed, _buffer_fft.data(), _fft.leng());
            _fft.fft(_buffer_fft, _buffer_fft);

            std::transform( std::execution::par_unseq, _buffer_fft.begin(),
                            _buffer_fft.end(), _buffer_psd.begin(),
                            []( const std::complex<float> &samp)
                            { return std::real( std::norm( samp));});

            std::vector<Peak> peaks;
            findPeaks( _buffer_psd, peaks);
#ifdef DEBUG
            std::cerr << "Peaks found: " << peaks.size() << std::endl;
#endif
            // extract peaks as carriers
            std::vector<Carrier> carriers;
            carriers = extractCarriers( _buffer_fft, peaks);

            // check for previsous peaks in range and compare IDs
            for( auto &carrier : carriers)
                checkForCarriersIdent( carrier);

            buffer_consumed += _overl_step;
        }
        // discard all consumed samples
        _buffer.erase( _buffer.begin(), _buffer.begin() + buffer_consumed);
    }


    /// @brief Check if peak-pos and peak-bw already exists
    void checkForCarriersIdent( const Carrier signal) {
        for( auto &carrier : _carriers) {
            if(    signal.rel_band_width < carrier.rel_band_width * 1.1
                && signal.rel_band_width < carrier.rel_band_width * .9) {

                std::cerr << signal.rel_band_width << " :: " << carrier.rel_band_width << std::endl;

                if(    signal.rel_freq < carrier.rel_freq + carrier.rel_band_width * .1
                    && signal.rel_freq > carrier.rel_freq - carrier.rel_band_width * .1) {
                    std::cerr << signal.rel_freq << " :: " << carrier.rel_freq << std::endl;

                    // already existing carrier - only add samples
                    carrier.samples.insert( carrier.samples.end(),
                                   signal.samples.begin(), signal.samples.end());
                    std::cerr << "found corresponding carrier !" << std::endl;
                    return;
                }

                else if(    signal.rel_freq < carrier.rel_freq + carrier.rel_band_width * .5
                         && signal.rel_freq > carrier.rel_freq - carrier.rel_band_width * .5)
                    std::cerr << "!!! Something curious carrier on carrier detection" << std::endl;
            }

        }
    }

    /// @brief extract time signal from detected peaks, therefor estimate extraction fft_leng and low-pass filter
    /// @param input frequency vector
    /// @param peaks to corresponding frequency vector
    /// @return Carriers same leng as peaks
    std::vector<Carrier>
    extractCarriers( std::vector<std::complex<float>> &input,
                     const std::vector<Peak> &peaks, uint64_t rel_invers_overlap = 4) {
        std::vector<Carrier> carriers;
        carriers.resize( peaks.size());
        const uint64_t fft_leng = input.size();
        for( const Peak &pk : peaks) {
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

    std::vector<uint64_t> _channel_id;
    std::vector<std::complex<float>> _buffer, _buffer_fft;
    std::vector<float> _buffer_psd;
    std::vector<struct Carrier> _carriers;
    std::atomic_bool _is_processing;
    ConditionSafeQueue<std::complex<float>> _puff;

    FFT _fft;
    FFTWindow _win;
    LowPassFilter _lpf;
};

#endif // CARRIERPROCESSING_H
