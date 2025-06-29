#ifndef BASEPROCESSOR_HPP
#define BASEPROCESSOR_HPP

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

    void start() { _running = true;};
    void stop() { _running = false;};

private:

    std::atomic<bool> _running;
    ConditionSafeQueue<std::complex<float>> _puff;
};

#endif // BASEPROCESSOR_HPP
