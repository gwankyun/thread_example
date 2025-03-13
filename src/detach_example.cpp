#include <chrono> // std::chrono
#include <mutex>  // std::mutex std::unique_lock
#include <string> // std::string
#include <thread> // std::jthread
using namespace std::literals;

#include <catch2/../catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp> // TEST_CASE REQUIRE
#include <spdlog/spdlog.h> // SPDLOG_INFO

/// @brief 分離
std::string example_detach(bool _detach)
{
    using std::this_thread::sleep_for;
    std::mutex mtx;    // 鎖
    auto flag = false; // 用於標識子線程結束
    std::string result;

    {
        std::jthread t(
            [&mtx, &flag, &result]
            {
                sleep_for(100ms);
                SPDLOG_INFO("child begin");
                result += "3";
                sleep_for(1s);
                {
                    std::unique_lock<std::mutex> lock(mtx);
                    flag = true;
                }
                SPDLOG_INFO("child end");
                result += "4";
            });

        SPDLOG_INFO("t.joinable(): {}", t.joinable()); // true
        if (_detach)
        {
            t.detach(); // 線程和線程柄分離
        }
        SPDLOG_INFO("t.joinable(): {}", t.joinable()); // false
        SPDLOG_INFO("main begin");
        result += "1";
    }

    SPDLOG_INFO("main end");
    result += "2";

    while (true)
    {
        {
            std::unique_lock<std::mutex> lock(mtx); // #1
            if (flag)
            {
                break;
            }
            else
            {
                SPDLOG_INFO("wait");
            }
        }
        sleep_for(100ms);
    }
    return result;
}

TEST_CASE("thread", "[detach]")
{
    REQUIRE(example_detach(true) == "1234");
    REQUIRE(example_detach(false) == "1342");
}

int main(int _argc, char* _argv[])
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    auto result = Catch::Session().run(_argc, _argv);
    return result;
}
