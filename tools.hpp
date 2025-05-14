#ifndef TOOLS_HPP
#define TOOLS_HPP

#include <vector>
#include <algorithm>
#include <complex>
#include <cstdint>
#include <deque>
#include <iostream>
#include <execution>

namespace Tools {

template <typename T>
void norm( const std::vector<std::complex<float>> &input, std::vector<T> &output) {
    std::transform( std::execution::par, input.begin(), input.end(), output.begin(),
                   [] ( const auto &val) {return static_cast<T>(std::norm( val));});
}

/// @brief: Generiert eine Schieberegisterfolge
/// @param polynom Angabe des Polynoms z.B. 4,1,0
/// @param seed Vorbelegung des Schieberegisters z.b. 111000
/// @param leng Laenge der LSR
//std::vector<char> generateLSR( const std::vector<int> &polynom,
//                               const std::vector<char> &seed, int leng) {
//    int max_coeff = *std::max_element( polynom.begin(), polynom.end());

//    std::vector<char> lsr( leng, 0);
//    std::copy( seed.begin(), seed.end(), lsr.begin());
//    for( uint64_t w = 0; w < lsr.size() - max_coeff; ++w)
//        for( uint64_t x = 0; x < polynom.size(); ++x)
//            lsr[ max_coeff + w] |= lsr.at( w + polynom.at(x));
//    return lsr;
//}

/// @brief applies a moving average
///
template <class T>
class MovingAverage {
    std::deque<std::vector<T>> _buffer;
    std::vector<T> _cum_sum, _output;
    uint64_t _leng;

public:
    MovingAverage( uint64_t leng) : _leng( leng) {
    }

    uint64_t getLeng() const { return _leng;}
    void setLeng( uint64_t leng) { _leng = leng;}

    /// @brief Ausgabe Mittelwert
    void average( std::vector<T> &output) { output = getAverage(); }
    std::vector<T> getAverage() const { return _output;}

    // may implement Max / Min / Avg
    void setMode() {
        throw std::runtime_error("not implemented");
    }

    /// @brief checkinput - buffer size, moving average
    void push( const std::vector<T> &input) {
        std::vector<T> tmp = input;
        if( _cum_sum.size() != input.size()) {
            _cum_sum = std::vector<T>( input.size(), static_cast<T>( 0.0));
            _buffer.resize( 0);
            std::cerr << " size mismatch" << std::endl;
        }
        _buffer.push_back( tmp);
        std::transform( std::execution::par, _cum_sum.begin(), _cum_sum.end(), _buffer.back().begin(), _cum_sum.begin(), std::plus<float>());
        if(_buffer.size() > _leng) {
            std::transform( std::execution::par, _cum_sum.begin(), _cum_sum.end(),  _buffer.front().begin(), _cum_sum.begin(), std::minus<float>());
            _buffer.pop_front();
        }
        _output.resize( _cum_sum.size());

        const float siz = static_cast<float>( _buffer.size());
        std::transform( std::execution::par, _cum_sum.begin(), _cum_sum.end(), _output.begin(), [ siz] (const float &val)
                       {return val / siz;});
    }
};


/// @brief Addiert to_add auf base zellenweise.
template <typename T>
static inline void
addToVector(std::vector<T> &base, const std::vector<T> &to_add) {
    if( base.empty())
        base.resize( to_add.size(), T(.0));
    if( base.size() != to_add.size())
        throw std::invalid_argument("addToVector(): base.size() != to_add.size()");

    std::transform(base.begin(), base.end(), to_add.begin(),
                   base.begin(), std::plus<T>());
}
template <typename T>
static inline void
addToVector(      std::vector<std::vector<T>> &base,
            const std::vector<std::vector<T>> &to_add) {
    if( base.empty())
        base.resize( to_add.size());
    if( base.size() != to_add.size())
        throw std::invalid_argument("addToVector(): base.size() != to_add.size()");

    for(uint64_t w = 0; w < base.size(); ++w)
        addToVector(base[w], to_add[w]);
}


uint64_t inline
nextPow2(uint64_t leng) {
    uint64_t exp = 0;
    while( static_cast<uint64_t>( 1 << ++exp) < leng);
    return exp;
}

/// @brief Berechnet die Varianz als Quadrat des Betrages (std::pow(std::abs, 2))
template <typename T = std::complex<float>>
double inline
variance( const std::vector<T> &input) {
    double sum = 0.0;
    for(auto val : input)
        sum += static_cast<double>(std::real(val * std::conj(val)));
    return sum / input.size();
}

template <typename T = std::complex<float>>
T inline
interpolateLinear( const std::vector<T> &input, double pos) {
    if(pos <= 0.0) return input[0];
    if(pos > input.size() - 1) return input.back();

    double ipos, fpos = std::modf(pos, &ipos);

    uint64_t i = static_cast<uint64_t>(ipos);
    return    input[i] * static_cast<T>(1.0 - fpos)
            + input[i + 1] * static_cast<T>(fpos);
}


/// @brief Berechnet den Absolutwert
/// @param input Abtastwerte
/// @return Absolutwerte der Eingangsdaten
template <typename T = double>
std::vector<T> inline
norm( const std::vector<std::complex<float>> &input) {
    if( input.empty())
        return {};
    std::vector<T> output( input.size());
    std::transform( std::execution::par_unseq, input.begin(), input.end(), output.begin(),
                   [](std::complex<float> c)
                   {return static_cast<T>( std::norm(c));});
    return output;
}

template <typename T_in = double, typename T_out = T_in>
void
abs( const std::vector<T_in> &input, std::vector<T_out> &output) {
    output.resize( input.size());
    std::transform( std::execution::par_unseq, input.begin(), input.end(), output.begin(), 
                    [] ( const T_in &val) { return std::abs<T_out>( val);});
}

/// @brief Berechnet den Absolutwert
/// @param input Abtastwerte
/// @return Absolutwerte der Eingangsdaten
template <typename T = double>
std::vector<T> inline
abs( const std::vector<std::complex<float>> &input) {
    std::vector<T> output(input.size());
    std::transform( std::execution:par_unseq, input.begin(), input.end(), output.begin(),
                   [] (const std::complex<float> &c) 
                   { return static_cast<T>(std::abs(c));});
    return output;
}

template <typename T = double>
std::vector<std::vector<T>> inline
    abs( const std::vector<std::vector<std::complex<float>>> &input) {
    std::vector<std::vector<T>> output(input.size());
    for( auto &vec : input)
        output.push_back( abs( vec));
    return output;
}


/// @brief Exponiert jeden Wert pow-fach
/// @param input Eingangsvektor
/// @param pow Exponent
/// @param pow Exponent um den jeder Wert potentiert wird
template <typename T = std::complex<float>>
static std::vector<T> inline
nPow(const std::vector<T> &input, int pow = 2) {
    std::vector<T> result(input.size());
    std::transform( std::execution::par_unseq, 
                    input.begin(), input.end(), result.begin(),
                    [pow]( const T &value) {return std::pow( value, pow);});

    return result;
}

/// @brief Berechnet fuer jeden Wert des Vektors: 10.0 * std::log10(Value).
///        Prueft auf isinfinite
/// @param input Eingangsvektor
/// @return Ausgangsvektor double
template <typename T = double>
static void
log10(  std::vector<float> &input) {
    for( uint64_t w = 0; w < input.size(); ++w) {
        if( std::isinf( input.at( w)))
            input[ w] = -120.;
        else
            input[ w] = 10 * std::log10( input.at( w));
    }
}
/// @brief Berechnet fuer jeden Wert des Vektors: 10.0 * std::log10(Value).
/// @param input Eingangsvektor
/// @return Ausgangsvektor double
template <typename T = double>
static std::vector<T>
log10d(const std::vector<double> &input) {
    std::vector<T> output(input.size());
    std::transform( std::execution::par_unseq, input.begin(), input.end(), output.begin(),
                   []( const double &val) { return static_cast<T>( 10.0 * std::log10(val));});
    return output;
}

///// @brief Berechnet fuer jeden Wert des Vektors: 10.0 * std::log10(Value).
///// @param input Eingangsvektor 2D
///// @return Ausgangsvektor 2D double
static std::vector<std::vector<double>>
log10(const std::vector<std::vector<double>> &input) {
    std::vector<std::vector<double>> output(input.size());
    for(uint64_t w = 0; w < input.size(); ++w)
        output[w] = log10d(input[w]);

    return output;
}

template <typename T = std::complex<float>>
std::vector<double> inline
getCumulativeSum( const std::vector<T> &input) {
    std::vector<double> output(input.size() + 1, 0.0);
    for(uint64_t w = 1; w < input.size(); ++w)
        output[w] = output[w - 1] + static_cast<double>(std::abs(input[w]));

    return output;
}

/// @brief Berechnet den Mittelwert der Amplituden
template <typename T = std::complex<float>>
double inline
getMeanMagnitude( const T *input, const uint64_t leng)
{
    double mean = 0.0;
    for(uint64_t w = 0; w < leng; ++w)
    {
        mean += static_cast<double>(std::abs(input[w]));
    }

    return mean / static_cast<double>(leng);
}
template <typename T = std::complex<float>>
double inline
getMeanMagnitude( const std::vector<T> &input)
{
    return getMeanMagnitude(input.data(), input.size());
}


template <typename T = uint64_t>
uint64_t inline
crossSum( T number) {
    uint64_t output = 0;
    while(number > 0) {
        output += static_cast<uint64_t>( number) % 10;
        number /= 10.;
    }
    return output;
}

/// @brief Berechnet die am naechstenliegende Zahl mit der geringsten Quersumme
///        im angegebenen Bereich z.B. number: 4799998 range: 2 -> 480000
/// @param number
/// @param range
template <typename T = double>
T inline
findCrossSumInRange( T number, uint64_t range) {
    if( crossSum( number + std::pow(10, range)) <= crossSum( number) )
        number += std::pow(10, range) - 1;

    int64_t  diff = 0;
    uint64_t tmp = static_cast<uint64_t>(number);
    for(uint64_t w = 0; w < range; ++w) {
        diff += (tmp % 10) * std::pow(10, w);
        tmp /= 10;
    }

    T result = static_cast<T>(static_cast<uint64_t>(number) - diff);

    if(std::abs(number - result) >= std::pow(10, range)) {
        std::cerr << std::abs(number - result) << " >= " << std::pow(10, range) << std::endl;
        throw std::runtime_error("FUCKED: ");
    }

    return result;
}




/// @brief Interpoliert die Position des Maximums innerhalb dreier Werte
///        liegen Abtastpunkt vor und nach dem Peak nicht auf der selben Hoehe,
///        dann kann die Peakposition nicht mittig sein.
/// @param
/// @param
/// @return interpolated Peak-Position
template <typename T = double>
double inline
interpolatePeakPosition(const std::vector<T> &input, uint64_t peak_pos) {
    double pos = 0.5 * static_cast<double>(
           (input[peak_pos - 1] - input[peak_pos + 1])
           / (input[peak_pos - 1] - 2 * input[peak_pos] + input[peak_pos + 1]));

    if(std::isinf(pos) || std::isnan(pos)) return peak_pos;

    return static_cast<double>(peak_pos) + std::max(std::min(pos, 1.0), -1.0);
}


/// @brief
template <typename T = std::complex<float>>
T inline
interpolCubic(const std::vector<T> &input, double pos) {
    if(input.size() < 3 || pos < 1.0 || pos >= input.size() - 2)
        return interpolateLinear<T> (input, pos);


    double ipos;
    const double fpos = std::modf(pos, &ipos);

    const uint64_t i = static_cast<uint64_t>( ipos);

    T a_0 = -static_cast<T>(0.5) * (input[i - 1] - input[i + 2])
            + static_cast<T>(1.5) * (input[i] - input[i + 1]),
      a_1 = input[i - 1] - static_cast<T>(2.5) * (input[i] - input[i + 1])
            - static_cast<T>(0.5) * input[i + 2],
      a_2 = -static_cast<T>(0.5) * (input[i - 1] - input[i + 1]),
      a_3 = input[i];

    return   a_0 * static_cast<T>(fpos * fpos * fpos)
           + a_1 * static_cast<T>(fpos * fpos)
           + a_2 * static_cast<T>(fpos)
           + a_3;
}


template <typename T = double >
double inline
getLocalRatio(const std::vector<T> &input,
              int64_t start_pos,
              int64_t local_area = 45)
{
    double peak_avg = 0.0;
    int64_t left = std::max(start_pos - local_area, static_cast<int64_t>(0));
    for(int64_t x = left; x < (start_pos - 5); ++x)
        peak_avg += input[x];
    int64_t right = std::min(start_pos + local_area, static_cast<int64_t>(input.size()));
    for(int64_t x = start_pos + 5; x < right; ++x)
        peak_avg += input[x];
    return input[start_pos]
            /  (peak_avg / static_cast<double>(right - left - 10));
}


/// @brief
/// @return Tools::absolute Position des naechsten Maximums im Eingangsvektor
template <typename T = double>
uint64_t
findNextLocalMaxRightwise(const std::vector<T> &input, uint64_t start_pos,
                          uint64_t threshold = 10.0) {
    int64_t interval = static_cast<int64_t>(std::ceil(input.size()
                                                              / 10000));

    for(int64_t w = start_pos + interval; w < input.size() - interval; ++w)
        if(getLocalRatio(input, w, interval) > threshold)
            return w;

    return start_pos;
}


/// @brief
/// @return Tools::absolute Position des naechsten Maximums im Eingangsvektor
template <typename T = double>
T
findNextLocalMaxLeftwise(const std::vector<T> &input, uint64_t start_pos,
                    double threshold = 10.0)
{
    uint64_t interval = static_cast<uint64_t>(std::ceil(input.size()
                                                              / 10000));


    for(int64_t w = start_pos - interval; w > input.size() - interval; --w)
    {
        if(getLocalRatio(input, w, interval) > threshold)
        {
            return w;
        }
    }

    return start_pos;
}


class Peaks {
    std::vector<double> _data;
    std::vector<std::pair<double, uint64_t>> _peaks;
    std::pair<double, uint64_t> _min_peak, _max_peak;
    uint64_t _gurad_interval, _range;
    double _threshold;

    void process() {
        peaksWithAmp();
        maxPeak();
        minPeak();
    }

    void
    peaksWithAmp() {
        _peaks.clear();
        for(uint64_t w = _gurad_interval; w < (_data.size() / 2); ++w) {
            double average = 0.0;
            if(w < _range)
                for(uint64_t x = 0; x < 2 * _range; ++x)
                    average += _data.at( x);
            else if(w + _range >= _data.size())
                for(uint64_t x = _data.size() - 2 * _range - 1; x < _data.size(); ++x)
                    average += _data.at( x);
            else
                for(uint64_t x = w - _range; x < w + _range; ++x)
                    average += _data.at( x);
            average /= 2 * _range;

            // Pruefen, ob lokale Spitze UND ueber Schwelle
            if(   (_data.at( w) > _data.at( w - 1))
               && (_data.at( w) > _data.at( w + 1))
               && (_data.at( w) > ( average * _threshold)))
                _peaks.push_back( std::pair<double, uint64_t>(_data.at( w), w));
        }
    }

    void
    maxPeak() {
        std::pair<double, uint64_t> best_peak = _peaks.front();
        for( const auto &peak : _peaks)
            if( best_peak.second < peak.second)
                best_peak = peak;
        _max_peak = best_peak;
    }

    void
    minPeak() {
        std::pair<double, uint64_t> best_peak = _peaks.front();
        for( const auto &peak : _peaks)
            if( best_peak.second > peak.second)
                best_peak = peak;
        _min_peak = best_peak;
    }

public:
    Peaks( const std::vector<double> &input = {})
        : _gurad_interval( 25), _range( 10), _threshold( 5.) {
        if( input.empty())
            return;
        setData( input);
        _data.reserve( input.size() / _range);
    }

    void setData( const std::vector<double> &input) {
        if( input.empty())
            return;
        _data = input;
        process();
    }

    std::vector<uint64_t> getPeaksSorted( bool is_ascending = true) {
        std::vector<std::pair<double, uint64_t>> tmp( _peaks);

        if( is_ascending)
            std::sort( tmp.begin(), tmp.end(), []
                       (const std::pair< double, uint64_t> &a,
                       const std::pair< double, uint64_t> &b) {
                       return a.first > b.first;});
        else
            std::sort( tmp.begin(), tmp.end(), []
                       (const std::pair< double, uint64_t> &a,
                       const std::pair< double, uint64_t> &b) {
                return a.first < b.first;});

        std::vector<uint64_t> output;
        output.reserve( tmp.size());
        for( const auto &peak : tmp)
            output.push_back( peak.second);
        return output;
    }
    std::vector<std::pair<double, uint64_t>> getPeaksWithAmp()  { return _peaks;}
    std::pair<double, uint64_t> getMaxPeak() { return _max_peak;}
    std::pair<double, uint64_t> getMinPeak() { return _min_peak;}
};


/// @brief Ermittelt Peaks ueber Schwelle
/// @param input
/// @param threshold Spitze-Umgebungs-Verhaeltnis
/// @param guard_interval Rand-Schutzbereich
/// @param range Bereich zur Umgebungsmittelung
std::vector<uint64_t> inline
findPeaks( const std::vector<double> &input, double threshold = 5.0,
           uint64_t guard_interval = 10, uint64_t range = 25) {
    std::vector<uint64_t> peaks_pos;
    peaks_pos.reserve( input.size() / range / 20);
    for(uint64_t w = guard_interval; w < (input.size() / 2); ++w) {
        double average = 0.0;
        if(w < range)
            for(uint64_t x = 0; x < 2 * range; ++x)
                average += input.at( x);
        else if(w + range >= input.size())
            for(uint64_t x = input.size() - 2 * range - 1; x < input.size(); ++x)
                average += input.at( x);
        else
            for(uint64_t x = w - range; x < w + range; ++x)
                average += input.at( x);
        average /= 2 * range;

        // Pruefen, ob lokale Spitze UND ueber Schwelle
        if(   (input.at( w) > input.at( w - 1))
           && (input.at( w) > input.at( w + 1))
           && (input.at( w) > (average * threshold)))
            peaks_pos.push_back( w);
    }
    return peaks_pos;
}


/// @brief Ermittelt Peaks ueber Schwelle
/// @param input
/// @param threshold Spitze-Umgebungs-Verhaeltnis
/// @param guard_interval Rand-Schutzbereich
/// @param range Bereich zur Umgebungsmittelung
/// @return
std::vector<std::pair<double, uint64_t>> inline
findPeaksWithAmp( const std::vector<double> &input, double threshold = 5.0,
           uint64_t guard_interval = 10, uint64_t range = 25) {
    std::vector<std::pair<double, uint64_t>> peaks;
    peaks.reserve( input.size() / range / 20);
    for(uint64_t w = guard_interval; w < (input.size() / 2); ++w) {
        double average = 0.0;
        if(w < range)
            for(uint64_t x = 0; x < 2 * range; ++x)
                average += input.at( x);
        else if(w + range >= input.size())
            for(uint64_t x = input.size() - 2 * range - 1; x < input.size(); ++x)
                average += input.at( x);
        else
            for(uint64_t x = w - range; x < w + range; ++x)
                average += input.at( x);
        average /= 2 * range;

        // Pruefen, ob lokale Spitze UND ueber Schwelle
        if(   (input.at( w) > input.at( w - 1))
           && (input.at( w) > input.at( w + 1))
           && (input.at( w) > (average * threshold)))
            peaks.push_back( std::pair<double, uint64_t>(input.at( w), w));
    }
    return peaks;
}

/// @brief Vektor mit sich selber konjugiert-komplex multiplizieren (z.B.: AKF).
std::vector<std::complex<float>> inline
conj( const std::vector<std::complex<float>> &input) {
    if( input.empty()) return {};
    std::vector<std::complex<float>> output(input.size());
    for( uint64_t w = 0; w < input.size(); ++w)
        output[ w] = std::complex<float>( std::norm( input.at( w)), .0);
    return output;
}
/// @brief Zwei Vektoren konjugiert-komplex multiplizieren. (z.B.: KKF).
inline std::vector<std::complex<float>>
conj(const std::vector<std::complex<float>> &input,
     const std::vector<std::complex<float>> &input2) {
    if(input.size() != input2.size())
        throw std::invalid_argument("FEHLER conj(): input.size() != input2.size()");

    std::vector<std::complex<float>> output(input.size());
    std::transform(input.begin(), input.end(),
                   input2.begin(),
                   output.begin(),
                   [](std::complex<float> value, std::complex<float> value2)
                   {return value * std::conj(value2);});
    return output;
}
/// @brief Zwei Vektoren konjugiert-komplex multiplizieren. (z.B.: KKF).
inline std::vector<std::complex<float>>
conj(const std::vector<std::complex<float>> &input,
     const std::complex<float> value) {
    if( input.empty()) return {};
    std::vector<std::complex<float>> output(input.size());
    std::complex<float> value_conj = std::conj(value);
    std::transform(input.begin(), input.end(),
                   output.begin(),
                   [value_conj](std::complex<float> sample)
                   {return sample * value_conj;});

    return output;
}

template <typename T = float>
inline void
conj( std::vector<std::complex<T>> &input) {
    std::transform( input.begin(), input.end(), input.begin(), [] ( std::complex<T> &val)
                   {return std::conj( val);});
}
template <typename T = float>
inline std::vector<std::complex<T>>
getConj( const std::vector<std::complex<T>> &input) {
    std::vector<std::complex<T>> tmp;
    std::transform( input.begin(), input.end(), tmp.begin(), [] ( std::complex<T> val)
                   {return std::conj<T>( val);});
    return tmp;
}

template <typename T = float>
inline void
shift( std::vector<T> &vec, int shift) {
    // links oder rechts rotieren
    if(shift < 0)
        std::rotate(vec.rbegin(), vec.rbegin() + std::abs(shift),
                    vec.rend());
    else
        std::rotate(vec.begin(), vec.begin() + shift, vec.end());
}
template <typename T = float>
inline void center( std::vector<T> &vec) {
    shift( vec, vec.size() / 2);
}


/// @brief Zaehlt die Vorkommen der Werte in input
/// @param input Vector
/// @param output Rückgabewert
/// @param find_and_count_divider filtert und zaehlt groessere Vielfache
/// @param output Paar-Vector mit erkannten Vorkommen
/// @param Minimale Zahl
template <typename T = uint64_t>
void
countOccurences( const std::vector<T> &input,
                 std::vector<std::pair<T, uint64_t>> &output,
                 bool find_and_count_divider = true,
                 uint64_t min_chipleng = 20) {
    if( input.empty()) return;
    output.reserve( output.size() + input.size());
    std::vector<T> tmp( input);
    std::sort(tmp.begin(), tmp.end(), std::less<int>());

    /* Alle Laengenvorkommen jeweils einmal merken. */
    for( auto w : tmp) {
        bool is_new = true;
        for( auto &x : output) {
            if(w == x.first) {
                is_new = false;
                x.second++;
            }
        }
        if( is_new && w > 20)
            output.push_back(std::pair<T, uint64_t>( w, 1));
    }

    if( ! find_and_count_divider)
        return;

    // nur kleinsten gemeinsame Teiler uebernehmen
    for(uint64_t w = 0; w < output.size(); ++w) {
        for(uint64_t x = 1; x < output.size(); ++x) {
            /* Nur kleinste gemeinsame Teiler ermitteln. */
            if(0 == (output.at(x).first % output.at(w).first) && x != w) {
                output[ w].second += output.at( x).second;
                output.erase( output.begin() + x);
            }
        }
    }
}



}

#endif
