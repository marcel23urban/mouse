#ifndef DSP_HPP
#define DSP_HPP

#include <execution>

#include "fft.hpp"
#include "tools.hpp"




/// @brief Ad-Hoc Funtion to correlate two vectors
void crossCorrelate( const std::vector<std::complex<float>> &input_a,
                    const std::vector<std::complex<float>> &input_b,
                    std::vector<std::complex<float>> &output) {
    FFT fft( input_a.size());
    std::vector<std::complex<float>> a_fft( input_a.size()), b_fft;
    fft.fft( input_a, a_fft);
    fft.fft( input_b, b_fft);
    std::transform( std::execution::par, a_fft.begin(), a_fft.begin(),
                   b_fft.begin(), output.begin(), []
                   (const auto &a, const auto &b) {return a * std::conj( b);});
    fft.ifft( output, output);
}

/// @brief Klasse fuer die schnelle Kreuzkorellation mit konstanter Sequenz
class XCorr{
    FFT _fft;
    std::vector<std::complex<float>> _sequence_fft, _input_fft, _output_conj;

public:
    XCorr( uint64_t leng = 0, const std::vector<std::complex<float>> &sequence = {}) {
        setSequence( sequence, leng);
    }

	/// @brief korreliert die Eingagnsdaten gegen die Korrelationssequenz
    void correlate( const std::vector<std::complex<float>> &input,
                   std::vector<std::complex<float>> &output) {
        if( _sequence_fft.empty()) throw std::runtime_error("FEHLER empty sequence");
        _fft.fft( input, _input_fft);
		
		// konjugierte multiplikation
        std::transform( std::execution::par, _input_fft.begin(), _input_fft.end(),
                        _output_conj.begin(), [] ( const auto &a, const auto &b)
                        { a * std::conj( b);});
		_fft.ifft( _output_conj, output);
    }

    bool setSequence( const std::vector<std::complex<float>> &sequence, uint64_t leng) {
        if( sequence.empty() || leng < 2) return false;
        _fft.setLeng( leng);
        std::vector<std::complex<float>> seq = sequence;
        seq.resize( _fft.leng(), std::complex<float>( .0, .0));
        _sequence_fft.resize( leng);
        _fft.fft( seq, _sequence_fft);
        return true;
    }
	
	uint64_t leng() const { return _fft.leng();}
	void setLeng( uint64_t leng) { _fft.setLeng( leng);}
};


/// @brief stellt eine Leistungsneutrale Kreuzkorellation dar ( 0.0 >= Resultat <= 1.0)
class PowerNeutralXcorr {
    XCorr _xcorr, _xcorr_abs, _xcorr_norm;
    std::vector<std::complex<float>> _input_xcorr, _input_abs, _input_abs_xcorr,
									  _input_norm, _input_norm_xcorr;
	uint64_t _seq_size;
public:

    PowerNeutralXcorr( uint64_t leng = 0, const std::vector<std::complex<float>> &sequence = {})
    {
 //       _fft.setLeng( leng);
 //       _xcorr.setSequence( sequence);
    }

    /// @brief korreliert die gesetzte Sequenz mit den Eingangsdaten und gibt
    ///		    das Ergebnis nach output
	/// @param input 
	/// @param output
    void correlate( const std::vector<std::complex<float>> &input,
                    std::vector<std::complex<float>> &output) {
        // Kreuzkorellation Zeitsignal
		_xcorr.correlate( input, _input_xcorr);
		// Kreuzkorellation mit dem Absolutwerten des Zeitsignals
		Tools::abs( input, _input_abs);
		_xcorr_abs.correlate( _input_abs, _input_abs_xcorr);
		// Kreuzkorrelation mit Absolutquadrat(norm) des Zeitsignals
		Tools::norm( input, _input_norm);
        _xcorr_norm.correlate( _input_norm, _input_norm_xcorr);
		// Multiplikation
        std::transform( std::execution::par, _input_xcorr.begin(), _input_xcorr.end(),
						 _input_abs_xcorr.begin(), output.begin(), std::multiplies());
		
		const float seq_size = static_cast<float>( _seq_size);
		// Division durch Kreuzkorrelierte des Betragsquadrat dabei auf .0 pruefen
		std::transform( std::execution::par, output.begin(), output.end(), 
                         _input_norm_xcorr.begin(), [ seq_size]
						 ( auto &val_1, const auto &val_2) {
         val_2 == .0 ? ( val_1 = 0) : (val_1 = val_1 / (val_2 + seq_size));});
    }

    bool setSequence( const std::vector<std::complex<float>> &sequence, uint64_t leng) {
        if( sequence.empty() || leng < 2) return false;
		_seq_size = sequence.size();
        std::vector<std::complex<float>> seq = sequence;
        seq.resize( leng, std::complex<float>( .0, .0));
        _xcorr.setSequence( seq, leng);
        std::vector<std::complex<float>> tmp( leng);
		std::transform( std::execution::par, tmp.begin(), tmp.end(), tmp.begin(),
					   []( auto &val){ return std::abs( val);});
        _xcorr_abs.setSequence( tmp, leng);
		std::transform( std::execution::par, tmp.begin(), tmp.end(), tmp.begin(),
					   []( auto &val){ return std::norm( val);});
        _xcorr_norm.setSequence( tmp, leng);
		
        return true;
    }
	
    uint64_t leng() const { return _xcorr.leng();}
};


/// @brief Performes a single psd OR a sums a psd to its avg
class Psd {

    FFT _fft;
    std::vector<std::complex<float>> _input_fft, _output_psd;
public:
    Psd( uint64_t leng = 0) {
        _fft.setLeng( leng);
    }

	/// @brief psd single shot
	/// @param input IQ Samples
	/// @param output result is SET to output as real float
    void get( const std::vector<std::complex<float>> &input, std::vector<float> &output,
			    bool log10 = true) {
		if( input.size() != _fft.leng()) throw std::invalid_argument("FEHLER input.size() != _fft.leng()");
		output.resize( input.size());
        _fft.fft( input, _input_fft);
		if( log10) {
            std::transform( std::execution::par_unseq,
						   _input_fft.begin(), _input_fft.end(),
						   _output_psd.begin(), [] ( const auto &val)
						   { return 10 * std::log10( std::norm( val));});
		}
		else {
            std::transform( std::execution::par_unseq,
						   _input_fft.begin(), _input_fft.end(),
						   _output_psd.begin(), [] ( const auto &val)
						   { return std::norm( val);});
		}
    }
	
	/// @brief psd calc and add psd to its predecessors
    /// @param input IQ Samples, if in_out.size != input.size: in_out resize .0
    /// @param in_out result is ADD to output as real float, if
    void add( const std::vector<std::complex<float>> &input, std::vector<float> &in_out) {
		if( input.size() != _fft.leng()) throw std::invalid_argument("FEHLER input.size() != _fft.leng()");
		if( in_out.size() != input.size()) {
			in_out.clear();
			in_out.resize( input.size(), .0);
		}
		_fft.fft( input, _input_fft);
		std::transform( std::execution::par,
					     _input_fft.begin(), _input_fft.end(),
                         in_out.begin(), [] ( const auto &val_in, auto &val_out)
                         { return val_out += std::norm( val_in);});
	}
	
	uint64_t leng() const { return _fft.leng();}
	void setLeng( uint64_t leng) { _fft.setLeng( leng);}
};


/// @brief LowPass Filter
class LowPassFilter {
    std::vector<std::complex<float>> _window;

    /// @brief Berechnet ein Filterfenster wobei der erste Teil der
    ///        Durchlassbereich, der zweite Teile der Uebergangsberich und der
    ///        dritte Teil der Daempfungsbereich ist.
    /// @param leng Laenge der zu filternden Signalbloecke
    /// @param passband Durchlassbereich vor dem Uebergangsbereich
    /// @param transition Uebergangsbereich vor dem Stopband (Steilheit des Filters)
    /// @param stop_attenuation Daempfung des Stopbandes
    /// @return Filterbins fuer den Frequenzbereich
    template <typename T = std::complex<float>>
    std::vector<T>
    generateLowPassWindow( uint64_t leng, double passband, double transition, double stop_attenuation = -60.0)
    {
        transition = std::min( std::max( 1e-6, std::abs(transition)), 0.99);
        if(leng <= 1) throw std::invalid_argument("leng <= 1");
        if(stop_attenuation >= 1.0) throw std::invalid_argument("stop_attenuation >= 1.0");
        double pass_factor = 1.0;
        double stop_factor = std::pow(10.0, stop_attenuation / 10.0);

        // Das Fenster wird fuer ein Seitenband berechnet und spaeter gespiegelt
        std::vector<double> window(leng / 2, pass_factor);

        uint64_t i = std::max<uint64_t>(0, static_cast<int>(std::ceil(passband * window.size())));
        for(; i < window.size(); ++i) {
            double x = static_cast<double>(i) / (window.size() - 1) - passband;
            if(x > transition) { // Stopband
                window[i] = stop_factor;
            }
            else { // Filterflanke
                window[i] = (0.5 + 0.5 * std::cos(M_PI * x / transition))
                * (pass_factor - stop_factor) + stop_factor;
            }
        }

        std::vector<T> window_T;
        window_T.reserve(leng);

        std::transform( window.begin(), window.end(), std::back_inserter( window_T),
                        [](double value){return static_cast<T>( value);});
        // compensate odd leng
        if( leng % 2) window_T.push_back(window.back());

        // push back, but vise versa
        std::transform(window.rbegin(), window.rend(), std::back_inserter(window_T),
                       [](double value){return static_cast<T>(value);});

        return window_T;
    }
public:
    LowPassFilter( uint64_t leng, double stop_attenuation = -60., double rel_passband = 0.1, double rel_transition = .0) {
        _window = generateLowPassWindow( leng, rel_passband, rel_transition, stop_attenuation);
    }

    /// @brief
    template <typename T>
    void apply( std::vector<T> &input) {
        if( input.size() != _window.size())
            throw std::runtime_error("LowPassFilter: input.size() != _window.size()");
        std::transform( std::execution::par_unseq, input.begin(), input.end(), _window.begin(),
                       input.begin(), std::multiplies<std::complex<float>>());
    }
};

#endif // DSP_HPP
