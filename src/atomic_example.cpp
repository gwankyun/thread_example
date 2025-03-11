#include <thread> // std::thread std::jthread
#include <chrono> // std::chrono
#include <mutex>  // std::mutex std::lock_guard std::unique_lock
#include <condition_variable> // std::condition_variable
#include <vector> // std::vector
#include <atomic>
using namespace std::literals;

#include <common.hpp> // join wait_for

void print_atomic(std::atomic<int>& _i)
{
    while (true)
    {
        auto value = _i.fetch_add(1); // 原子加一並返回原先的值
        if (value >= 10)
        {
            return;
        }
        DBG(value);
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
#if HAS_SPDLOG
    spdlog::set_pattern("[%Y-%m-%d %T.%e] [%^%l%$] [t:%6t] [p:%6P] [%-20!!:%4#] %v");
#endif

    example_atomic();

    return 0;
}
