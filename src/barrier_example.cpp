#include <thread> // std::jthread
#include <chrono> // std::chrono
#include <barrier> // std::barrier
using namespace std::literals;

// #include <spdlog/spdlog.h> // SPDLOG_INFO
#include <catch2/../catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp> // TEST_CASE REQUIRE
#include <spdlog/spdlog.h>              // SPDLOG_INFO

/// @brief 屏障
// void example_barrier()
// {
//     auto flag = true;
//     std::barrier b(5, []() noexcept // noexcept必不可少，不然無法編譯
//         {
//             SPDLOG_INFO("CompletionFunction");
//         });

//     std::jthread t([&flag, &b]
//         {
//             SPDLOG_INFO("before wait");
//             b.arrive_and_wait(); // 等同b.wait(b.arrive()); #2
//             SPDLOG_INFO("after wait");
//             flag = false; // 注釋這句會計數會週而複始 #3
//         });

//     while (flag)
//     {
//         std::this_thread::sleep_for(100ms);
//         SPDLOG_INFO("arrive");
//         auto _ = b.arrive(); // b的計數減一 #1
//     }
// }

// int main()
// {
//     spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

//     example_barrier();

//     return 0;
// }

TEST_CASE("thread", "[latch]")
{
    std::atomic<int> result = 0;
    std::atomic<bool> flag = true;
    // std::latch ltc{4};
    std::barrier brr(4, []() noexcept // noexcept必不可少，不然無法編譯
        {
            SPDLOG_INFO("CompletionFunction");
        });

    auto token = brr.arrive();

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
                    // std::this_thread::sleep_for(100ms);
                }
                // SPDLOG_INFO("i: {}", i);
                // result += 1;
                // ltc.arrive_and_wait(); // 減一並等變零

                // result += 1;
                // ltc.arrive_and_wait(); // 減一並等變零
            });
        t.detach();
    }

    // ltc.count_down();
    SPDLOG_INFO("main");
    brr.wait(std::move(token)); 

    REQUIRE(result == 3);

    // ltc.arrive_and_drop();

    // REQUIRE(result == 5);
    std::this_thread::sleep_for(100ms);
    REQUIRE(result == 6);

    std::this_thread::sleep_for(100ms);
    REQUIRE(result == 9);

    

    flag = false;
}

int main(int _argc, char* _argv[])
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    auto result = Catch::Session().run(_argc, _argv);
    return result;
}
