
#include <thread> // std::thread std::jthread
#include <chrono> // std::chrono
#include <mutex>  // std::mutex std::lock_guard std::unique_lock
#include <condition_variable> // std::condition_variable
#include <vector> // std::vector
#include <iostream>
#include <semaphore>
using namespace std::literals;

#include <common.hpp> // join wait_for

/// @brief 信號量
void example_semaphore()
{
    std::binary_semaphore semaphore(0);

    std::jthread t([&semaphore]
        {
            semaphore.acquire(); // 主線程發信號後才會調用
            SPDLOG_INFO("Child recv");
            std::this_thread::sleep_for(1s);
            SPDLOG_INFO("Child send");
            semaphore.release();
        });

    SPDLOG_INFO("Main send");
    semaphore.release();
    std::this_thread::sleep_for(1s);
    semaphore.acquire(); // 等待子線程發信號
    SPDLOG_INFO("Main recv");
}

int main()
{
#if HAS_SPDLOG
    spdlog::set_pattern("[%Y-%m-%d %T.%e] [%^%l%$] [t:%6t] [p:%6P] [%-20!!:%4#] %v");
#endif

    example_semaphore();

    return 0;
}
