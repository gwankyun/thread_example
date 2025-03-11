

#include <thread> // std::thread std::jthread
#include <chrono> // std::chrono
#include <mutex>  // std::mutex std::lock_guard std::unique_lock
#include <condition_variable> // std::condition_variable
#include <vector> // std::vector
#include <iostream>
#include <future> // std::promise std::future std::launch std::packaged_task std::async
using namespace std::literals;

#include <common.hpp> // join wait_for

enum struct State
{
    None = 0,
    Start,
    Child_1,
    Child_2
};

void print_notify(std::mutex& _mtx, std::condition_variable& _cv, int& _i, State& _current, State _type)
{
    std::string name = _type == State::Child_1 ? "child_1" : "child_2";
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        {
            std::unique_lock<std::mutex> lock(_mtx); // #1
            if (_i >= 10) // 判斷也要加鎖
            {
                return;
            }
            _cv.wait(lock, [&_current, _type]
                {
                    return _current == _type || _current == State::Start;
                }); // 等待通知
            SPDLOG_INFO("{}: {}", name, _i);
            _i++;
            _current = _type == State::Child_1 ? State::Child_2 : State::Child_1;
            lock.unlock(); // 提前解鎖 #2
            _cv.notify_one(); // 通知另一個線程
        }
    }
}

/// @brief 條件變量
void example_condition_variable_notify_one()
{
    std::mutex mtx; // 鎖
    std::condition_variable cv; // 條件變量
    auto flag = false;

    std::thread t([&mtx, &cv, &flag]
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&flag] { return flag; }); // 等待主線程#4通知
            SPDLOG_INFO("Child recv");
            std::this_thread::sleep_for(std::chrono::seconds(1));
            flag = true;
            lock.unlock();
            cv.notify_one(); // 通知主線程 #5
            SPDLOG_INFO("Child send");
        });

    SPDLOG_INFO("Main send");
    std::unique_lock<std::mutex> lock(mtx); // #1
    flag = true;
    lock.unlock(); // 提前解鎖 #2
    cv.notify_one(); // 通知子線程 #4
    std::this_thread::sleep_for(std::chrono::seconds(1));
    lock.lock(); // 再次上鎖 #3
    cv.wait(lock, [&flag] { return flag; }); // 等待子線程#5通知
    SPDLOG_INFO("Main recv");

    join(t);
}

int main()
{
#if HAS_SPDLOG
    spdlog::set_pattern("[%Y-%m-%d %T.%e] [%^%l%$] [t:%6t] [p:%6P] [%-20!!:%4#] %v");
#endif

    example_condition_variable_notify_one();

    return 0;
}
