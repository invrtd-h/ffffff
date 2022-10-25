#ifndef UNDERSCORE_CPP_PRACTICE_HPP
#define UNDERSCORE_CPP_PRACTICE_HPP

#include <mutex>

namespace fff {
    /**
     * This code is copied from cppreference.com
     */
    class ThreadsafeCounter
    {
        mutable std::mutex m; // The "M&M rule": mutable and mutex go together
        int data = 0;
    public:
        int get() const
        {
            std::lock_guard<std::mutex> lk(m);
            return data;
        }

        void inc()
        {
            std::lock_guard<std::mutex> lk(m);
            ++data;
        }
    };
}

#endif//UNDERSCORE_CPP_PRACTICE_HPP
