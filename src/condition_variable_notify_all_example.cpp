#include <thread> // std::thread
#include <chrono> // std::chrono
#include <mutex>  // std::mutex std::unique_lock
#include <condition_variable> // std::condition_variable

#include <spdlog/spdlog.h> // SPDLOG_INFO

void join(std::thread& _thread)
{
    if (_thread.joinable())
    {
        _thread.join();
    }
}

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
void example_condition_variable_notify_all()
{
    std::mutex mtx; // 鎖
    std::condition_variable cv; // 條件變量
    auto i = 0;
    State current = State::None; // 用於標識線程

    // 線程一
    std::thread child_1([&mtx, &cv, &i, &current]
        {
            print_notify(mtx, cv, i, current, State::Child_1);
        });

    // 線程二
    std::thread child_2([&mtx, &cv, &i, &current]
        {
            print_notify(mtx, cv, i, current, State::Child_2);
        });

    SPDLOG_INFO(""); // 這條語句先於child_1與child_2線程 #3

    std::unique_lock<std::mutex> lock(mtx);
    current = State::Start;
    lock.unlock();
    cv.notify_all(); // 通知所有線程 #4

    join(child_1);

    join(child_2);
}

int main()
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    example_condition_variable_notify_all();

    return 0;
}
