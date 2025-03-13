
#include <thread> // std::jthread
#include <chrono> // std::chrono
#include <semaphore> // std::binary_semaphore
#include <string>
using namespace std::literals;

#include <catch2/../catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp> // TEST_CASE REQUIRE
#include <spdlog/spdlog.h>              // SPDLOG_INFO

/// @brief 信號量
std::string example_semaphore()
{
    using std::this_thread::sleep_for;
    std::string result;
    std::binary_semaphore semaphore(0);

    std::jthread t([&semaphore, &result]
        {
            semaphore.acquire(); // 主線程發信號後才會調用
            SPDLOG_INFO("Child recv");
            result += "2";
            sleep_for(100ms);
            SPDLOG_INFO("Child send");
            semaphore.release();
        });

    SPDLOG_INFO("Main send");
    result += "1";
    semaphore.release();
    sleep_for(100ms);
    semaphore.acquire(); // 等待子線程發信號
    SPDLOG_INFO("Main recv");
    result += "3";
    return result;
}

TEST_CASE("thread", "[semaphoresync]")
{
    REQUIRE(example_semaphore() == "123");
}

int main(int _argc, char* _argv[])
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    auto result = Catch::Session().run(_argc, _argv);
    return result;
}
