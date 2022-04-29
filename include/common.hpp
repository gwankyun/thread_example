#pragma once
#include <thread> // std::thread
#include <future> // std::future
#include <chrono> // std::chrono

#if SPDLOG_EXISTS
#  include <spdlog/spdlog.h>
#else
#  include <iostream>
#  define SPDLOG_INFO(x) std::cout << (x) << "\n";
#endif

inline void join(std::thread& _thread)
{
    if (_thread.joinable())
    {
        _thread.join();
    }
}

template <typename T, typename Rep, typename Per>
inline T wait_for(std::future<T>& _ft, const std::chrono::duration<Rep, Per>& _rel_time)
{
    auto flag = true;
    while (flag)
    {
        auto status = _ft.wait_for(_rel_time);
        switch (status)
        {
        case std::future_status::ready:
            flag = false;
            return _ft.get();
        case std::future_status::timeout:
            SPDLOG_INFO("timeout");
            break;
        case std::future_status::deferred:
            SPDLOG_INFO("deferred");
            break;
        default:
            break;
        }
    }
    return T{};
}
