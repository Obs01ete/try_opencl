//
// Created by Dmitrii Khizbullin on 2019-06-30.
//

#ifndef TRY_OPENCL_TIMER_H
#define TRY_OPENCL_TIMER_H

#include <chrono>
#include <string>

class Timer
{
private:
    std::chrono::time_point<std::chrono::steady_clock> m_start;

public:
    Timer();

    void print(const std::string& prefix);
};

class ReentryTimer
{
private:
    std::unique_ptr<std::chrono::time_point<std::chrono::steady_clock> > m_start;

public:
    ReentryTimer();
    void print(const std::string& prefix);
};

#endif //TRY_OPENCL_TIMER_H
