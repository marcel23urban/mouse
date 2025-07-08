#ifndef BASEPROCESSOR_HPP
#define BASEPROCESSOR_HPP

#include <thread>

#include "conditionalsafequeue.hpp"

class BaseProcessor {
    void run() {
        std::vector<std::complex<float>> data;

        while( _running) {
            if( _puff.try_pop( data)) {
                process( data);
            }
        }
    }

    virtual void process( const std::vector<std::complex<float>> &input) = 0;

public:
    BaseProcessor() {

    }

    bool start() {
        _running = true;
        _fred = std::thread( &BaseProcessor::process, this);
        return _fred.joinable();
    };
    bool stop() {
        _running = false;

    };

    void dataIn( std::vector<std::complex<float>> input) {
        if( input.empty()) return;
        _puff.push( input);
    }


private:

    std::atomic<bool> _running;
    ConditionSafeQueue<std::complex<float>> _puff;
    std::thread _fred;
};

#endif // BASEPROCESSOR_HPP
