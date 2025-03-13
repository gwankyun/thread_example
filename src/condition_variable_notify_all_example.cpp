#include <chrono>             // std::chrono
#include <condition_variable> // std::condition_variable
#include <mutex>              // std::mutex std::unique_lock
#include <string>
#include <thread> // std::jthread
using namespace std::literals;

#include <catch2/../catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp> // TEST_CASE REQUIRE
#include <spdlog/spdlog.h>              // SPDLOG_INFO

enum struct State
{
    None = 0,
    Start,
    Child_1,
    Child_2
};

void print_notify(std::mutex& _mtx, std::condition_variable& _cv, int& _i, State& _current, State _type,
                  std::string& _result)
{
    std::string name = _type == State::Child_1 ? "child_1" : "child_2";
    while (true)
    {
        std::this_thread::sleep_for(200ms);
        {
            std::unique_lock<std::mutex> lock(_mtx); // #1
            if (_i >= 10)                            // 判斷也要加鎖
            {
                return;
            }
            _cv.wait(lock, [&_current, _type] { return _current == _type || _current == State::Start; }); // 等待通知
            SPDLOG_INFO("{}: {}", name, _i);
            _result += std::to_string(_i);
            _i++;
            _current = _type == State::Child_1 ? State::Child_2 : State::Child_1;
            lock.unlock();    // 提前解鎖 #2
            _cv.notify_one(); // 通知另一個線程
        }
    }
}

/// @brief 條件變量
std::string example_condition_variable_notify_all()
{
    std::string result;
    std::mutex mtx;             // 鎖
    std::condition_variable cv; // 條件變量
    auto i = 0;
    State current = State::None; // 用於標識線程

    // 線程一
    std::jthread child_1([&mtx, &cv, &i, &current, &result]
                         { print_notify(mtx, cv, i, current, State::Child_1, result); });

    // 線程二
    std::jthread child_2([&mtx, &cv, &i, &current, &result]
                         { print_notify(mtx, cv, i, current, State::Child_2, result); });

    SPDLOG_INFO(""); // 這條語句先於child_1與child_2線程 #3

    std::unique_lock<std::mutex> lock(mtx);
    current = State::Start;
    lock.unlock();
    cv.notify_all(); // 通知所有線程 #4
    return result;
}

TEST_CASE("condition_variable", "[notify_all]")
{
    REQUIRE(example_condition_variable_notify_all() == "0123456789");
}

int main(int _argc, char* _argv[])
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    auto result = Catch::Session().run(_argc, _argv);
    return result;
}
