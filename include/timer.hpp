/*
 * timer.hpp
 *
 *  Created on: Jul 29, 2017
 *      Author: nullifiedcat
 */

#ifndef CH_TIMER_HPP
#define CH_TIMER_HPP
#include <chrono>

class Timer
{
public:
    typedef std::chrono::system_clock clock;
    std::chrono::time_point<clock> last{};
    inline Timer(){};

    inline bool check(unsigned ms) const
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - last).count() >= ms;
    }
    inline bool test_and_set(unsigned ms)
    {
        if (check(ms))
        {
            update();
            return true;
        }
        return false;
    }
    inline void update()
    {
        last = clock::now();
    }
};
#endif
