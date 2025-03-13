#include <chrono>             // std::chrono
#include <condition_variable> // std::condition_variable
#include <mutex>              // std::mutex std::unique_lock
#include <thread>             // std::jthread
#include <string>             // std::string
using namespace std::literals;

#include <catch2/../catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp> // TEST_CASE REQUIRE
#include <spdlog/spdlog.h>              // SPDLOG_INFO

/// @brief 條件變量
std::string example_condition_variable_notify_one()
{
    using std::this_thread::sleep_for;
    std::mutex mtx;             // 鎖
    std::condition_variable cv; // 條件變量
    auto flag = false;
    std::string result;

    std::jthread t(
        [&mtx, &cv, &flag, &result]
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&flag] { return flag; }); // 等待主線程#4通知
            SPDLOG_INFO("Child recv");
            result += "2";
            sleep_for(1s);
            flag = true;
            lock.unlock();
            cv.notify_one(); // 通知主線程 #5
            SPDLOG_INFO("Child send");
        });

    SPDLOG_INFO("Main send");
    result += "1";
    std::unique_lock<std::mutex> lock(mtx); // #1
    flag = true;
    lock.unlock();   // 提前解鎖 #2
    cv.notify_one(); // 通知子線程 #4
    sleep_for(1s);
    lock.lock();                             // 再次上鎖 #3
    cv.wait(lock, [&flag] { return flag; }); // 等待子線程#5通知
    SPDLOG_INFO("Main recv");
    result += "3";

    return result;
}

TEST_CASE("condition_variable", "[notify_one]")
{
    REQUIRE(example_condition_variable_notify_one() == "123");
}

int main(int _argc, char* _argv[])
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    auto result = Catch::Session().run(_argc, _argv);
    return result;
}
