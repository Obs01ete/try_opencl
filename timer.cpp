//
// Created by Dmitrii Khizbullin on 2019-06-30.
//

#include "timer.h"

#include <iostream>

Timer::Timer()
{
    m_start = std::chrono::steady_clock::now();
}

void Timer::print(const std::string& prefix)
{
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end - m_start;
    std::cout << prefix << " " << diff.count()*1e+3f << " ms\n";
}


ReentryTimer::ReentryTimer() {}

void ReentryTimer::print(const std::string& prefix)
{
    auto end = std::chrono::steady_clock::now();
    if (m_start)
    {
        std::chrono::duration<double> diff = end - *m_start;
        std::cout << prefix << " " << diff.count()*1e+3f << " ms\n";
    }

    m_start = std::make_unique<std::chrono::time_point<std::chrono::steady_clock>>(end);
}
