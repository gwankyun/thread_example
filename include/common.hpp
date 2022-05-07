#pragma once
#include <thread> // std::thread
#include <future> // std::future
#include <chrono> // std::chrono
#include <condition_variable> // std::condition_variable

#if HAS_SPDLOG
#  include <spdlog/spdlog.h>
#else
#  include <iostream>
#  define SPDLOG_INFO(x) std::cout << (x) << "\n";
#endif

#define TO_STRING_IMPL(x) #x
#define TO_STRING(x) TO_STRING_IMPL(x)

#define DBG(x) SPDLOG_INFO("{}: {}", TO_STRING(x), x)

inline void join(std::thread& _thread)
{
    if (_thread.joinable())
    {
        _thread.join();
    }
}

template<typename Rep, typename Per, typename Pred, typename Finished>
inline void wait_for(std::mutex& _mtx, std::condition_variable& _cv, const std::chrono::duration<Rep, Per>& _rel_time, Pred _pred, Finished _finished)
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(_mtx);
        auto result = _cv.wait_for(lock, _rel_time, _pred);
        lock.unlock();
        if (_finished(result))
        {
            return;
        }
    }
}

template<typename T, typename Rep, typename Per, typename Finished>
inline void wait_for(std::future<T>& _ft, const std::chrono::duration<Rep, Per>& _rel_time, Finished _finished)
{
    while (true)
    {
        auto status = _ft.wait_for(_rel_time);
        auto result = _finished(status);
        if (result)
        {
            return;
        }
    }
}
