#include <thread> // std::thread std::jthread
#include <chrono> // std::chrono
#include <mutex>  // std::mutex std::lock_guard std::unique_lock
#include <condition_variable> // std::condition_variable
#include <vector> // std::vector
#include <iostream>
#include <future> // std::promise std::future std::launch std::packaged_task std::async
using namespace std::literals;

#include <common.hpp> // join wait_for

/// @brief 創建線程
void example_thread()
{
    std::thread t([]
        {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            SPDLOG_INFO("child");
        });

    SPDLOG_INFO("main");
    if (t.joinable()) // 保證主線程結束前子線程執行完畢
    {
        t.join();
    }
}

int main()
{
#if HAS_SPDLOG
    spdlog::set_pattern("[%Y-%m-%d %T.%e] [%^%l%$] [t:%6t] [p:%6P] [%-20!!:%4#] %v");
#endif

    example_thread();

    return 0;
}
