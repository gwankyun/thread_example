#include <thread> // std::thread
#include <chrono> // std::chrono
#include <atomic> // std::atomic

#include <spdlog/spdlog.h> // SPDLOG_INFO

void join(std::thread& _thread)
{
    if (_thread.joinable())
    {
        _thread.join();
    }
}

void print_atomic(std::atomic<int>& _i)
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        auto value = _i.fetch_add(1); // 原子加一並返回原先的值
        if (value >= 10)
        {
            return;
        }
        SPDLOG_INFO("value: {}", value);
    }
}

/// @brief 原子操作
void example_atomic()
{
    std::atomic<int> i = 0;

    std::thread t([&i]
        {
            print_atomic(i);
        });

    print_atomic(i);

    join(t);
}

int main()
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    example_atomic();

    return 0;
}
