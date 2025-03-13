#include <chrono> // std::chrono
#include <thread> // std::jthread
#include <string> // std::string
using namespace std::literals;

#include <catch2/../catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp> // TEST_CASE REQUIRE
#include <spdlog/spdlog.h>              // SPDLOG_INFO

/// @brief 自動合併線程類
std::string example_jthread()
{
    std::string result;

    // 析構時自動調用t.join()
    std::jthread t(
        [&result]
        {
            std::this_thread::sleep_for(1s);
            SPDLOG_INFO("child");
            result += "2";
        });

    SPDLOG_INFO("main");
    result += "1";
    return result;
}

TEST_CASE("thread", "[jthread]")
{
    REQUIRE(example_jthread() == "12");
}

int main(int _argc, char* _argv[])
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    auto result = Catch::Session().run(_argc, _argv);
    return result;
}
