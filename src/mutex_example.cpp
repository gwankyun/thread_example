#include <thread> // std::thread 
#include <chrono> // std::chrono
#include <mutex>  // std::mutex std::lock_guard

#include <spdlog/spdlog.h> // SPDLOG_INFO

void join(std::thread& _thread)
{
    if (_thread.joinable())
    {
        _thread.join();
    }
}

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
            SPDLOG_INFO("_i: {}", _i);
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
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    example_mutex();

    return 0;
}
