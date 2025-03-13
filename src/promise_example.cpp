#include <thread> // std::jthread
#include <chrono> // std::chrono
#include <future> // std::promise std::future
using namespace std::literals;

#include <catch2/../catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp> // TEST_CASE REQUIRE
#include <spdlog/spdlog.h>              // SPDLOG_INFO

/// @brief 期望與承諾
int example_promise()
{
    using std::this_thread::sleep_for;
    std::promise<int> promise;
    auto future = promise.get_future();

    std::jthread t([&promise]
        {
            for (auto i = 0; i < 10; i++)
            {
                SPDLOG_INFO("child");
                sleep_for(100ms);
            }
            promise.set_value(99);
        });

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

TEST_CASE("future", "[promise]")
{
    REQUIRE(example_promise() == 99);
}

int main(int _argc, char* _argv[])
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    auto result = Catch::Session().run(_argc, _argv);
    return result;
}
