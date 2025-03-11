#include <thread> // std::thread std::jthread
#include <chrono> // std::chrono
#include <mutex>  // std::mutex std::lock_guard std::unique_lock
#include <condition_variable> // std::condition_variable
#include <vector> // std::vector
#include <iostream>
using namespace std::literals;

#include <common.hpp> // join wait_for

/// @brief 自動合併線程類
void example_jthread()
{
    // 析構時自動調用t.join()
    std::jthread t([]
        {
            std::this_thread::sleep_for(3s);
            SPDLOG_INFO("child");
        });

    SPDLOG_INFO("main");
}

int main()
{
#if HAS_SPDLOG
    spdlog::set_pattern("[%Y-%m-%d %T.%e] [%^%l%$] [t:%6t] [p:%6P] [%-20!!:%4#] %v");
#endif

    example_jthread();

    return 0;
}
