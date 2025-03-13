#include <thread> // std::jthread
#include <chrono> // std::chrono
#include <latch> // std::latch
using namespace std::literals;

#include <catch2/../catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp> // TEST_CASE REQUIRE
#include <spdlog/spdlog.h>              // SPDLOG_INFO

/// @brief 閂
TEST_CASE("thread", "[latch]")
{
    std::atomic<int> result = 0;
    std::latch ltc{4};

    for (auto i = 0; i != 3; i++)
    {
        std::jthread t(
            [i, &result, &ltc]
            {
                SPDLOG_INFO("i: {}", i);
                std::this_thread::sleep_for(100ms);
                result += 1;
                ltc.arrive_and_wait(); // 減一並等變零
            });
        t.detach();
    }

    ltc.count_down();
    SPDLOG_INFO("main");
    ltc.wait();

    REQUIRE(result == 3);
}

int main(int _argc, char* _argv[])
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    auto result = Catch::Session().run(_argc, _argv);
    return result;
}
