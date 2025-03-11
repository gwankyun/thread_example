
#include <thread> // std::thread std::jthread
#include <chrono> // std::chrono
#include <mutex>  // std::mutex std::lock_guard std::unique_lock
#include <condition_variable> // std::condition_variable
#include <vector> // std::vector
#include <iostream>
#include <future> // std::promise std::future std::launch std::packaged_task std::async
using namespace std::literals;

#include <common.hpp> // join wait_for

void print_mutex(std::mutex& _mtx, int& _i)
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        {
            std::lock_guard<std::mutex> lock(_mtx); // 自動解鎖
            if (_i >= 10) // 判斷也要加鎖
            {
                return;
            }
            DBG(_i);
            _i++;
        }
    }
}

/// @brief 鎖
void example_mutex()
{
    std::mutex mtx;
    auto i = 0;

    std::thread t([&mtx, &i]
        {
            print_mutex(mtx, i);
        });

    print_mutex(mtx, i);

    join(t);
}

int main()
{
#if HAS_SPDLOG
    spdlog::set_pattern("[%Y-%m-%d %T.%e] [%^%l%$] [t:%6t] [p:%6P] [%-20!!:%4#] %v");
#endif

    example_mutex();

    return 0;
}
