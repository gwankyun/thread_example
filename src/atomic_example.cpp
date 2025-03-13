#include <thread> // std::thread
#include <chrono> // std::chrono
#include <atomic> // std::atomic
using namespace std::literals;

#include <catch2/../catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp> // TEST_CASE REQUIRE
#include <spdlog/spdlog.h>              // SPDLOG_INFO

void print_atomic(std::atomic<int>& _i, std::atomic<int>& _result)
{
    while (true)
    {
        {
            auto value = _i.fetch_add(1); // 原子加一並返回原先的值
            if (value >= 10)
            {
                return;
            }
            _result.fetch_add(value);
            SPDLOG_INFO("value: {}", value);
        }
        std::this_thread::sleep_for(100ms);
    }
}

/// @brief 原子操作
TEST_CASE("thread", "[atomic]")
{
    std::atomic<int> i = 0;
    std::atomic<int> result = 0;

    std::jthread t([&i, &result] { print_atomic(i, result); });

    print_atomic(i, result);

    while (i.load() < 10) // 原子讀取值
    {
        SPDLOG_INFO("wait");
    }

    REQUIRE(result == 45);
}

int main(int _argc, char* _argv[])
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    auto result = Catch::Session().run(_argc, _argv);
    return result;
}
