#include <thread> // std::thread std::jthread
#include <chrono> // std::chrono
#include <mutex>  // std::mutex std::lock_guard std::unique_lock
#include <condition_variable> // std::condition_variable
#include <vector> // std::vector
using namespace std::literals;

#include <common.hpp> // join wait_for

/// @brief 分離
void example_detach()
{
    std::mutex mtx; // 鎖
    std::condition_variable cv; // 條件變量
    auto flag = false; // 用於標識子線程結束

    std::thread t([&mtx, &cv, &flag]
        {
            std::this_thread::sleep_for(3s);
            {
                std::unique_lock<std::mutex> lock(mtx);
                flag = true;
            }
            // lock.unlock();
            SPDLOG_INFO("child");
            cv.notify_one();
        });

    DBG(t.joinable()); // true
    t.detach(); // 線程和線程柄分離
    DBG(t.joinable()); // false

    wait_for(mtx, cv, 200ms, [&flag] { return flag; },
        [](bool _result)
        {
            DBG(_result);
            std::this_thread::sleep_for(200ms);
            return _result;
        });
}

int main()
{
#if HAS_SPDLOG
    spdlog::set_pattern("[%Y-%m-%d %T.%e] [%^%l%$] [t:%6t] [p:%6P] [%-20!!:%4#] %v");
#endif

    example_detach();

    return 0;
}
