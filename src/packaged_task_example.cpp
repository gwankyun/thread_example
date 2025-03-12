#include <thread> // std::thread std::jthread
#include <chrono> // std::chrono
#include <future> // std::packaged_task
#include <spdlog/spdlog.h> // SPDLOG_INFO

/// @brief 任務
void example_packaged_task()
{
    using std::this_thread::sleep_for;
    std::packaged_task<int()> task(
        []
        {
            for (auto i = 0; i < 10; i++)
            {
                SPDLOG_INFO("child");
                sleep_for(std::chrono::milliseconds(100));
            }
            return 99;
        });
    auto future = task.get_future();

    std::thread t([&task] { task(); });

    auto result = [&future]
    {
        using std::future_status;
        while (true)
        {
            auto status = future.wait_for(std::chrono::milliseconds(100));
            switch (status)
            {
            case future_status::ready:
                return future.get();
                break;
            case future_status::timeout:
                SPDLOG_INFO("timeout");
                break;
            case future_status::deferred:
                SPDLOG_INFO("deferred");
                break;
            default:
                break;
            }
        }
    }();

    SPDLOG_INFO("result: {}", result);

    if (t.joinable())
    {
        t.join();
    }
}

int main()
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    example_packaged_task();

    return 0;
}
