#include <chrono> // std::chrono
#include <string> // std::string
#include <thread> // std::thread

#include <catch2/../catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp> // TEST_CASE REQUIRE
#include <spdlog/spdlog.h>              // SPDLOG_INFO

/// @brief 創建線程
std::string example_thread()
{
    std::string result;

    std::thread t(
        [&result]
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            SPDLOG_INFO("child");
            result += "2";
        });

    SPDLOG_INFO("main");
    result += "1";
    if (t.joinable()) // 保證主線程結束前子線程執行完畢
    {
        t.join();
    }
    return result;
}

TEST_CASE("thread", "[thread]")
{
    REQUIRE(example_thread() == "12");
}

int main(int _argc, char* _argv[])
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    auto result = Catch::Session().run(_argc, _argv);
    return result;
}
