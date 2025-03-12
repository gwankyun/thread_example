#include <thread> // std::thread std::jthread
#include <chrono> // std::chrono
#include <mutex>  // std::mutex std::unique_lock
using namespace std::literals;

#include <spdlog/spdlog.h> // SPDLOG_INFO

/// @brief 分離
void example_detach()
{
    std::mutex mtx; // 鎖
    std::condition_variable cv; // 條件變量
    auto flag = false; // 用於標識子線程結束

    std::thread t([&mtx, &cv, &flag]
        {
            std::this_thread::sleep_for(1s);
            {
                std::unique_lock<std::mutex> lock(mtx);
                flag = true;
            }
            SPDLOG_INFO("child begin");
        });

    SPDLOG_INFO("t.joinable(): {}", t.joinable()); // true
    t.detach(); // 線程和線程柄分離
    SPDLOG_INFO("t.joinable(): {}", t.joinable()); // false

    while (true)
    {
        {
            std::unique_lock<std::mutex> lock(mtx); // #1
            if (flag)
            {
                SPDLOG_INFO("child end");
                break;
            }
            else
            {
                SPDLOG_INFO("wait");
            }
        }
        std::this_thread::sleep_for(100ms);
    }
}

int main()
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    example_detach();

    return 0;
}
