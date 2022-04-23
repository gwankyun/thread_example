#include <thread> // std::thread std::jthread
#include <chrono> // std::chrono
#include <mutex>  // std::mutex std::lock_guard std::unique_lock
#include <condition_variable> // std::condition_variable
#include <future> // std::promise std::future std::launch std::packaged_task std::async
#include <semaphore> // std::counting_semaphore std::binary_semaphore
using namespace std::literals;

#if SPDLOG_EXISTS
#  include <spdlog/spdlog.h>
#else
#  include <iostream>
#  define SPDLOG_INFO(x) std::cout << (x) << "\n";
#endif

void example_01()
{
    std::thread t([](int _line)
        {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            SPDLOG_INFO(_line);
        }, __LINE__);

    SPDLOG_INFO("");
    if (t.joinable()) // 保證主線程結束前子線程執行完畢
    {
        t.join();
    }
}

void print_02(std::mutex& _mtx, int& _i)
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        {
            std::lock_guard<std::mutex> lock(_mtx); // 自動解鎖
            if (_i >= 10) // 判斷也要加鎖
            {
                return;
            }
            SPDLOG_INFO(_i);
            _i++;
        }
    }
}

enum struct State
{
    None = 0,
    Start,
    Child_1,
    Child_2
};

void print_03(std::mutex& _mtx, std::condition_variable& _cv, int& _i, State& _current, State _type)
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

void example_02()
{
    std::mutex mtx;
    int i = 0;

    std::thread t([&mtx, &i]
        {
            print_02(mtx, i);
        });

    print_02(mtx, i);

    if (t.joinable()) // 保證主線程結束前子線程執行完畢
    {
        t.join();
    }
}

void example_03()
{
    std::mutex mtx; // 鎖
    std::condition_variable cv; // 條件變量
    int i = 0;
    State current = State::None; // 用於標識線程

    // 線程一
    std::thread child_1([&mtx, &cv, &i, &current]
        {
            print_03(mtx, cv, i, current, State::Child_1);
        });

    // 線程二
    std::thread child_2([&mtx, &cv, &i, &current]
        {
            print_03(mtx, cv, i, current, State::Child_2);
        });

    SPDLOG_INFO(""); // 這條語句先於child_1與child_2線程 #3

    {
        std::lock_guard<std::mutex> lock(mtx);
        current = State::Start;
    }
    cv.notify_all(); // 通知所有線程 #4

    if (child_1.joinable())
    {
        child_1.join();
    }

    if (child_2.joinable())
    {
        child_2.join();
    }
}

template <typename T, typename Rep, typename Per>
void wait_for(std::future<T>& _ft, const std::chrono::duration<Rep, Per>& _rel_time)
{
    auto flag = true;
    while (flag)
    {
        auto status = _ft.wait_for(_rel_time);
        switch (status)
        {
        case std::future_status::ready:
            SPDLOG_INFO("ready: {0}", _ft.get());
            flag = false;
            break;
        case std::future_status::timeout:
            SPDLOG_INFO("timeout");
            break;
        case std::future_status::deferred:
            SPDLOG_INFO("deferred");
            break;
        default:
            break;
        }
    }
}

void example_04()
{
    std::promise<int> pms;
    std::future<int> ft = pms.get_future();

    std::thread t([&pms]
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            pms.set_value(99);
        });

    wait_for(ft, std::chrono::milliseconds(200));

    if (t.joinable())
    {
        t.join();
    }
}

void example_05()
{
    std::packaged_task<int()> task([]
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            return 99;
        });
    std::future<int> ft = task.get_future();

    std::thread t([&task]
        {
            task();
        });

    wait_for(ft, std::chrono::milliseconds(200));

    if (t.joinable())
    {
        t.join();
    }
}

void example_06()
{
    auto ft = std::async(std::launch::async, []
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            return 99;
        });

    wait_for(ft, std::chrono::milliseconds(200));
}

void example_07()
{
    // 析構時自動調用t.join()
    std::jthread t([](int _line)
        {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            SPDLOG_INFO(_line);
        }, __LINE__);

    SPDLOG_INFO("");
}

template<typename F>
struct scope_exit
{
    scope_exit(F _f) : f(_f) {}
    ~scope_exit() { f(); }
    F f;
};

void example_08()
{
    std::binary_semaphore semaphore(0);
    int i = 0;

    std::jthread t([&semaphore, &i]
        {
            semaphore.acquire(); // 主線程發信號後才會調用
            SPDLOG_INFO("Got");
            std::this_thread::sleep_for(1s);
            SPDLOG_INFO("Send");
            semaphore.release();
        });

    SPDLOG_INFO("Send");
    semaphore.release();
    std::this_thread::sleep_for(1s);
    semaphore.acquire(); // 等待子線程發信號
    SPDLOG_INFO("Got");
}

int main()
{
#if SPDLOG_EXISTS
    spdlog::set_pattern("[%Y-%m-%d %T.%e] [%^%l%$] [t:%6t] [p:%6P] [%-20!!:%4#] %v");
#endif

    //example_01();

    //example_02();

    //example_03();

    //example_04();

    //example_05();

    //example_06();

    //example_07();

    example_08();

    return 0;
}
