#include <chrono> // std::chrono
#include <future> // std::async

#include <spdlog/spdlog.h> // SPDLOG_INFO

/// @brief 異步調用
void example_async()
{
    auto future = std::async(std::launch::async, []
        {
            for (auto i = 0; i < 10; i++)
            {
                SPDLOG_INFO("child");
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            return 99;
        });

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
}

int main()
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    example_async();

    return 0;
}
