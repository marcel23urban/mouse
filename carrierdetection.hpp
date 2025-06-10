#ifndef CARRIERDETECTION_HPP
#define CARRIERDETECTION_HPP

#include <vector>
#include <complex>
#include <execution>
#include <chrono>

#include "dsp.hpp"
#include "fft.hpp"


struct Peak {
    uint64_t pos_left;
    uint64_t pos_right;
    float magnitude;
};

struct Carrier {
    double rel_freq;
    double samp_rate;
    double band_width;
    std::time_t start_time;
};

/// @brief Try to find peaks by sorting descending by magnitude while keeping index and looking
///        left/right for threshold (peak - left/right < 12.)-> match!
/// @param input  floats ( i.e. PSD)
/// @param peaks output tuple of peaks <mag, left, right>
/// @param threshold difference peak and left/rigth in dB
/// @param stepping left right going for check threshold
void findPeaks( const std::vector<float> &input, std::vector<std::tuple<float, uint64_t, uint64_t>> &peaks, float threshold = 12., uint64_t stepping = 1) {
    std::vector<std::pair<uint64_t, float>> _indexed_samples( input.size());
    float avg = std::accumulate( input.begin(), input.end(), .0);
    avg /= input.size();
    std::transform( input.begin(), input.end(), _indexed_samples.begin(),
                   [ index = 0] (float val) mutable {return std::make_pair( static_cast<uint64_t>( index), val);});
    // absteiend sortieren
    std::sort( _indexed_samples.begin(), _indexed_samples.end(), []
              ( const auto &a, const auto &b) { return a.second < b.second;});

    // check descending ordered samples for possible peaks
    for( const auto &samp : _indexed_samples) {
        bool in_free_range = true;
        // check sample if it is in an existing peak-range
        for( const auto &peak : peaks) {
            if(    samp.first > std::get<1>( peak)
                || samp.first < std::get<2>( peak)) {
                in_free_range = false;
                break;
            }
        }
        // range already taken
        if( ! in_free_range) continue;

        uint64_t width = std::min( input.size() / 2, std::min( samp.first, input.size() - samp.first));

        const float mag = samp.second;
        const uint64_t pos = samp.first;
        for( uint64_t w = 0; w < width; ++w) {
            // check if threshold reached
            if(    ( mag - input.at( pos + w)) > threshold
                || ( mag - input.at( pos - w)) > threshold) {

                peaks.emplace_back( mag, pos - w, pos + w);
                break;
            }
        }
    }
}


/// Traegerdetektion anhand Leistung.
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

    void push( const std::vector<std::complex<float>> &input) {
        _psd.add( input, _psd_buffer);
        if( ++_psd_cnt > 8) {
            checkCarrier( _psd_buffer);


        }
    }

    /// @brief return Peaks if exists
    std::vector<Peak> Peak getPeaks() const { return _peaks;}

private:
    void checkCarrier( const std::vector<float> &input) {
        std::vector<std::tuple<float, uint64_t, uint64_t>> peaks;
        findPeaks( _psd_buffer, peaks);
        if( peaks.empty()) return;

        for( auto peak : peaks)
            std::cerr << std::get<>

        // check for previsous peaks in range and compare IDs

    }

    /// @brief extract time signal, therefor estimate extraction fft_leng and low-pass filter
    void extractCarriers( std::vector<std::complex<float>> &input, const std::vector<Carrier> &carriers) {
        // fft_leng of necessary extraction window
        for( const Carrier &carrier : carriers) {
            uint64_t extract_fft_leng = static_cast<double>( input.size()) * 1.25 * carrier.band_width;


            std::iterator<uint64_t> beg = carrier.rel_freq * input.size() - std::ceil( .5 * carrier)
        }



    }

    uint64_t _psd_leng, _psd_avg, _threshold_db;
    Psd _psd;
    std::vector<uint64_t> _channel_id;
    std::vector<std::complex<float>> _fft_buffer;
    std::vector<float> _psd_buffer;
    std::vector<struct Peak> _peaks;
    uint64_t _psd_cnt, _psd_leng;

    LowPassFilter _lpf;
};

#endif // CARRIERDETECTION_HPP
