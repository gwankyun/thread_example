#include <barrier> // std::barrier
#include <chrono>  // std::chrono
#include <thread>  // std::jthread
using namespace std::literals;

#include <catch2/../catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp> // TEST_CASE REQUIRE
#include <spdlog/spdlog.h>              // SPDLOG_INFO

/// @brief 屏障
TEST_CASE("thread", "[barrier]")
{
    std::atomic<int> result = 0;
    std::atomic<bool> flag = true;
    std::barrier brr(4, []() noexcept // noexcept必不可少，不然無法編譯
                     { SPDLOG_INFO("CompletionFunction"); });

    for (auto i = 0; i != 3; i++)
    {
        std::jthread t(
            [i, &result, &brr, &flag]
            {
                while (flag)
                {
                    SPDLOG_INFO("i: {}", i);
                    std::this_thread::sleep_for(100ms);
                    result += 1;
                    brr.arrive_and_wait(); // 減一並等變零
                }
            });
        t.detach();
    }

    brr.arrive_and_wait();
    REQUIRE(result == 3);

    brr.arrive_and_wait();
    REQUIRE(result == 6);

    brr.arrive_and_wait();
    REQUIRE(result == 9);

    flag = false;
}

int main(int _argc, char* _argv[])
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    auto result = Catch::Session().run(_argc, _argv);
    return result;
}
