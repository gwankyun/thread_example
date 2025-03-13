#include <chrono> // std::chrono
#include <mutex>  // std::mutex std::lock_guard
#include <string> // std::string
#include <thread> // std::jthread
using namespace std::literals;

#include <catch2/../catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp> // TEST_CASE REQUIRE
#include <spdlog/spdlog.h>              // SPDLOG_INFO

void print_mutex(std::mutex& _mtx, int& _i, int& _result)
{
    while (true)
    {
        {
            std::lock_guard<std::mutex> lock(_mtx); // 自動解鎖
            if (_i >= 10)                           // 判斷也要加鎖
            {
                return;
            }
            SPDLOG_INFO("_i: {}", _i);
            _result += _i;
            _i++;
        }
        std::this_thread::sleep_for(100ms);
    }
}

/// @brief 鎖
TEST_CASE("thread", "[mutex]")
{
    std::mutex mtx;
    auto i = 0;
    int result = 0;

    std::jthread t([&mtx, &i, &result] { print_mutex(mtx, i, result); });

    print_mutex(mtx, i, result);

    REQUIRE(result == 45);
}

int main(int _argc, char* _argv[])
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    auto result = Catch::Session().run(_argc, _argv);
    return result;
}
