#ifndef PEAKDETECTION_HPP
#define PEAKDETECTION_HPP

#include <vector>
#include <cstdint>
#include <algorithm>
#include <numeric>


struct Peak {
    uint64_t pos_left;
    uint64_t pos_right;
    float magnitude;
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

#endif // PEAKDETECTION_HPP
