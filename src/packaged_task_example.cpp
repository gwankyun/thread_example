#include <thread> // std::jthread
#include <chrono> // std::chrono
#include <future> // std::packaged_task
using namespace std::literals;

#include <catch2/../catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp> // TEST_CASE REQUIRE
#include <spdlog/spdlog.h>              // SPDLOG_INFO

/// @brief 任務
int example_packaged_task()
{
    using std::this_thread::sleep_for;
    std::packaged_task<int()> task(
        []
        {
            for (auto i = 0; i < 10; i++)
            {
                SPDLOG_INFO("child");
                sleep_for(100ms);
            }
            return 99;
        });
    auto future = task.get_future();

    std::jthread t([&task] { task(); });

    auto result = [&future]
    {
        using std::future_status;
        while (true)
        {
            auto status = future.wait_for(100ms);
            switch (status)
            {
            case future_status::ready:
                return future.get();
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
    return result;
}

TEST_CASE("future", "[packaged_task]")
{
    REQUIRE(example_packaged_task() == 99);
}

int main(int _argc, char* _argv[])
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    auto result = Catch::Session().run(_argc, _argv);
    return result;
}
