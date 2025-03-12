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

/// @brief 條件變量
void example_condition_variable_notify_one()
{
    using std::this_thread::sleep_for;
    std::mutex mtx; // 鎖
    std::condition_variable cv; // 條件變量
    auto flag = false;

    std::thread t([&mtx, &cv, &flag]
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&flag] { return flag; }); // 等待主線程#4通知
            SPDLOG_INFO("Child recv");
            sleep_for(std::chrono::seconds(1));
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
    sleep_for(std::chrono::seconds(1));
    lock.lock(); // 再次上鎖 #3
    cv.wait(lock, [&flag] { return flag; }); // 等待子線程#5通知
    SPDLOG_INFO("Main recv");

    join(t);
}

int main()
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    example_condition_variable_notify_one();

    return 0;
}
