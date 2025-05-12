#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <vector>
#include <queue>
#include <complex>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <optional>


///// @brief Einer der vielen Implementierungen eines Datenbuffers je Block
//template <typename T = char>
//class ConditionSafeQueue {
//    uint64_t _max_limit{ 1000};
//    std::queue<std::vector<T>> _queue; // eigentlicher Puffer
//    mutable std::mutex _mutexer; // Besetztzeichen
//    std::condition_variable _empty_condition; // "Sie haben Post"

//public:
//    ConditionSafeQueue() = default;
//    ConditionSafeQueue( const ConditionSafeQueue<T> &) = delete;
//    ConditionSafeQueue& operator =( const ConditionSafeQueue<T> &) = delete;
//    virtual ~ConditionSafeQueue(){}

//    bool empty() const {return _queue.empty();}
//    uint64_t size() const {
//        std::lock_guard<std::mutex> lock(_mutexer);
//        return _queue.size();
//    }
//    /// @brief Wenn sich Daten in der _queue befinden, werden diese in output
//    ///        geschrieben und von der _queue geloescht
//    /// @param output Datenrausgabe, wenn vorhanden
//    /// @return true: Daten vorhanden und in output, false: Daten NICHT vorhanden
//    bool try_pop( std::vector<T> &output) {
//        std::unique_lock<std::mutex> lock(_mutexer);
//        if( ! _empty_condition.wait_for( lock, std::chrono::milliseconds( 1),
//                                        [ this] {return ! _queue.empty();}))
//            return false;
//        output = std::move( _queue.front());
//        _queue.pop();
//        return true;
//    }

//    /// @brief: Kopiert Daten auf die _queue und loescht, sobald limit erreicht
//    void push( std::vector<T> input) {
//        std::lock_guard<std::mutex> lock( _mutexer);
//        if( _queue.size() >_max_limit)
//            _queue.pop();
//        _queue.push( std::move( input));
//        _empty_condition.notify_one();
//    }
//};



//template <typename T = std::complex<float>>
//class ConditionSafeBuffer {
//    uint64_t _max_limit{ 64 * 1024 * 1024};
//    std::vector<T> _buffer; // eigentlicher Puffer
//    mutable std::mutex _mutexer; // Besetztzeichen
//    std::condition_variable _empty_condition; // "Sie haben Post"

//public:
//    ConditionSafeBuffer() = default;
//    ConditionSafeBuffer( const ConditionSafeBuffer<T> &) = delete;
//    ConditionSafeBuffer& operator =( const ConditionSafeBuffer<T> &) = delete;
//    virtual ~ConditionSafeBuffer(){}

//    bool empty() const {return _buffer.empty();}

//    uint64_t size() const {
//        std::lock_guard<std::mutex> lock(_mutexer);
//        return _buffer.size();
//    }

//    bool try_pop( std::vector<T>::iterator output) {
//        std::unique_lock<std::mutex> lock(_mutexer);
//        if( ! _empty_condition.wait_for( lock, std::chrono::milliseconds( 1),
//                                        [ this] {return ! _buffer.empty();}))
//            return false;
//        output = std::move( output));
//        _buffer.pop();
//        return true;
//    }
//    bool try_pop( std::vector<T>::iterator output) {
//        std::unique_lock<std::mutex> lock(_mutexer);
//        if( ! _empty_condition.wait_for( lock, std::chrono::milliseconds( 1),
//                                        [ this] {return ! _buffer.empty();}))
//            return false;
//        output = std::move( output));
//        _buffer.pop();
//        return true;
//    }
//};

/// @brief Einer der vielen Implementierungen eines Datenbuffers je Block mit Schreib/ Leseschutz
///        und notifier
template <class T>
class ConditionSafeQueue {
    uint64_t _max_limit{ 64 * 1024 * 1024};
    std::queue<std::vector<T>> _queue; // eigentlicher Puffer
    mutable std::mutex _mutexer; // Besetztzeichen
    std::condition_variable _empty_condition, _full_condition; // "Sie haben Post"
    std::atomic_bool _reject_input = false;

public:
    ConditionSafeQueue() = default;
    ConditionSafeQueue( const ConditionSafeQueue<T> &) = delete;
    ConditionSafeQueue& operator =( const ConditionSafeQueue<T> &) = delete;
    virtual ~ConditionSafeQueue(){}

    bool empty() const {return _queue.empty();}
    uint64_t size() const {
        std::unique_lock<std::mutex> lock(_mutexer);
        return _queue.size();
    }

    //void stop( );
	/// @brief ABBRUCH
	void abort() { 
		_reject_input = true;
        clear();
    }
	void clear() { 
		std::unique_lock<std::mutex> lock(_mutexer);
		std::queue<std::vector<T>>().swap( _queue);
	}
	
    /// @brief Wenn sich Daten in der _queue befinden, werden diese an output angehangen bzw. geswapped
    /// @param output vector-Referenz
    /// @return true: Daten vorhanden und in output, false: Daten NICHT vorhanden
    std::optional<bool> try_pop( std::vector<T> &output, bool blocking = true) {
        std::unique_lock<std::mutex> lock(_mutexer);
		if( blocking) {
            while( _queue.empty()) {
                if( _reject_input) return false;
				_empty_condition.wait( lock);
			}
		}

        if( _queue.empty() ||_reject_input) return false;
        if( ! _empty_condition.wait_for( lock, std::chrono::milliseconds( 1),
                                        [ this] {return ! _queue.empty();}))
            return false;
		// Falls output==leer, dann SCHNELLER Speicherwechsl
        if(output.empty())
            std::swap(output, _queue.front());
		// sonst Daten anhaengen
        else
            output.insert(output.end(), _queue.front().begin(), _queue.front().end());
        _queue.pop();
		
		_full_condition.notify_one();
        return true;
    }

// mit iterator versuchen
    //    bool try_pop( std::vector<T>::iterator output) {
    //        std::unique_lock<std::mutex> lock(_mutexer);
    //        if( ! _empty_condition.wait_for( lock, std::chrono::milliseconds( 1),
    //                                        [ this] {return ! _buffer.empty();}))
    //            return false;
    //        output = std::move( output));
    //        _buffer.pop();
    //        return true;
    //    }

    /// @brief: Kopiert Daten auf die _queue und loescht, sobald limit erreicht
	/// @param blocking true: waits, until queue is capable, false: discard if queue full 
    bool push( std::vector<T> input, bool blocking = true) {
		if( _reject_input) return false;
        std::unique_lock<std::mutex> lock( _mutexer);
		if( blocking) {
			while(  _queue.size() >= _max_limit) {
				if( _reject_input) return false;
				_full_condition.wait( lock);
			}
		}
		else {
			if( _queue.size() >= _max_limit) return false;
		}

        _queue.push( std::move( input));
        _empty_condition.notify_one();
		return true;
    }
};

#endif // QUEUE_HPP
