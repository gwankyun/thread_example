
#include <thread> // std::thread std::jthread
#include <chrono> // std::chrono
#include <mutex>  // std::mutex std::lock_guard std::unique_lock
#include <condition_variable> // std::condition_variable
#include <vector> // std::vector
#include <iostream>
#include <future> // std::promise std::future std::launch std::packaged_task std::async
using namespace std::literals;

#include <common.hpp> // join wait_for

void print_child()
{
    for (auto i = 0; i < 10; i++)
    {
        SPDLOG_INFO("child");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

template<typename T>
bool get_value(std::future<T>& _future, T& _value)
{
    auto flag = false;
    wait_for(_future, std::chrono::milliseconds(100),
        [&_value, &flag, &_future](std::future_status _status)
        {
            switch (_status)
            {
            case std::future_status::ready:
                _value = _future.get();
                flag = true;
                return true;
            case std::future_status::timeout:
                SPDLOG_INFO("timeout");
                return false;
            case std::future_status::deferred:
                SPDLOG_INFO("deferred");
                return false;
            default:
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    return flag;
}

/// @brief 期望與承諾
void example_promise()
{
    std::promise<int> promise;
    auto future = promise.get_future();

    std::thread t([&promise]
        {
            print_child();
            promise.set_value(99);
        });

    int result = 0;
    if (get_value(future, result))
    {
        DBG(result);
    }

    join(t);
}

int main()
{
#if HAS_SPDLOG
    spdlog::set_pattern("[%Y-%m-%d %T.%e] [%^%l%$] [t:%6t] [p:%6P] [%-20!!:%4#] %v");
#endif

    example_promise();

    return 0;
}
