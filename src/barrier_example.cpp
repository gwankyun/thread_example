#include <thread> // std::thread std::jthread
#include <chrono> // std::chrono
#include <mutex>  // std::mutex std::lock_guard std::unique_lock
#include <condition_variable> // std::condition_variable
#include <vector> // std::vector
#include <barrier> // std::barrier
using namespace std::literals;

#include <common.hpp> // join wait_for

/// @brief 屏障
void example_barrier()
{
    auto flag = true;
    std::barrier b(5, []() noexcept // noexcept必不可少，不然無法編譯
        {
            SPDLOG_INFO("CompletionFunction");
        });

    std::jthread t([&flag, &b]
        {
            SPDLOG_INFO("before wait");
            b.arrive_and_wait(); // 等同b.wait(b.arrive()); #2
            SPDLOG_INFO("after wait");
            flag = false; // 注釋這句會計數會週而複始 #3
        });

    while (flag)
    {
        std::this_thread::sleep_for(100ms);
        SPDLOG_INFO("arrive");
        auto _ = b.arrive(); // b的計數減一 #1
    }
}

int main()
{
#if HAS_SPDLOG
    spdlog::set_pattern("[%Y-%m-%d %T.%e] [%^%l%$] [t:%6t] [p:%6P] [%-20!!:%4#] %v");
#endif

    example_barrier();

    return 0;
}
