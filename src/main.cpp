#include <thread> // std::thread std::jthread
#include <chrono> // std::chrono
#include <mutex>  // std::mutex std::lock_guard std::unique_lock
#include <condition_variable> // std::condition_variable
#include <future> // std::promise std::future std::launch std::packaged_task std::async
#include <semaphore> // std::counting_semaphore std::binary_semaphore
#include <latch> // std::latch
#include <vector> // std::vector
#include <barrier> // std::barrier
#include <atomic>
using namespace std::literals;

#include <common.hpp> // join wait_for

/// @brief 創建線程
void example_thread()
{
    std::thread t([]
        {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            SPDLOG_INFO("child");
        });

    SPDLOG_INFO("main");
    if (t.joinable()) // 保證主線程結束前子線程執行完畢
    {
        t.join();
    }
}

void print_mutex(std::mutex& _mtx, int& _i)
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
            DBG(_i);
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

/// @brief 鎖
void example_mutex()
{
    std::mutex mtx;
    auto i = 0;

    std::thread t([&mtx, &i]
        {
            print_mutex(mtx, i);
        });

    print_mutex(mtx, i);

    join(t);
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

void print_child()
{
    for (auto i = 0; i < 10; i++)
    {
        SPDLOG_INFO("child");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

template<typename T>
bool get_value(std::future<T>& _future, T& _value)
{
    auto flag = false;
    wait_for(_future, std::chrono::milliseconds(100),
        [&_value, &flag, &_future](std::future_status _status)
        {
            switch (_status)
            {
            case std::future_status::ready:
                _value = _future.get();
                flag = true;
                return true;
            case std::future_status::timeout:
                SPDLOG_INFO("timeout");
                return false;
            case std::future_status::deferred:
                SPDLOG_INFO("deferred");
                return false;
            default:
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    return flag;
}

/// @brief 期望與承諾
void example_promise()
{
    std::promise<int> promise;
    auto future = promise.get_future();

    std::thread t([&promise]
        {
            print_child();
            promise.set_value(99);
        });

    int result = 0;
    if (get_value(future, result))
    {
        DBG(result);
    }

    join(t);
}

/// @brief 任務
void example_packaged_task()
{
    std::packaged_task<int()> task([]
        {
            print_child();
            return 99;
        });
    auto future = task.get_future();

    std::thread t([&task] { task(); });

    int result = 0;
    if (get_value(future, result))
    {
        DBG(result);
    }

    join(t);
}

/// @brief 異步調用
void example_async()
{
    auto future = std::async(std::launch::async, []
        {
            print_child();
            return 99;
        });

    int result = 0;
    if (get_value(future, result))
    {
        DBG(result);
    }
}

/// @brief 自動合併線程類
void example_jthread()
{
    // 析構時自動調用t.join()
    std::jthread t([]
        {
            std::this_thread::sleep_for(3s);
            SPDLOG_INFO("child");
        });

    SPDLOG_INFO("main");
}

/// @brief 信號量
void example_semaphore()
{
    std::binary_semaphore semaphore(0);

    std::jthread t([&semaphore]
        {
            semaphore.acquire(); // 主線程發信號後才會調用
            SPDLOG_INFO("Child recv");
            std::this_thread::sleep_for(1s);
            SPDLOG_INFO("Child send");
            semaphore.release();
        });

    SPDLOG_INFO("Main send");
    semaphore.release();
    std::this_thread::sleep_for(1s);
    semaphore.acquire(); // 等待子線程發信號
    SPDLOG_INFO("Main recv");
}

void on_latch(std::latch& _lt)
{
    while (true)
    {
        if (_lt.try_wait()) // 非阻塞等待計數為零 #2
        {
            SPDLOG_INFO("latch open");
            return;
        }
        else
        {
            SPDLOG_INFO("count_down");
            _lt.count_down(); // 計數減一 #1
        }
        std::this_thread::sleep_for(100ms);
    }
}

/// @brief 閂
void example_latch()
{
    std::latch lt(5);

    std::jthread t([&lt] { on_latch(lt); });

    on_latch(lt);
}

/// @brief 屏障
void example_barrier()
{
    auto flag = true;
    std::barrier b(5, []() noexcept // noexcept必不可少，不然無法編譯
        {
            SPDLOG_INFO("CompletionFunction");
        });

    std::jthread t([&flag, &b]
        {
            SPDLOG_INFO("before wait");
            b.arrive_and_wait(); // 等同b.wait(b.arrive()); #2
            SPDLOG_INFO("after wait");
            flag = false; // 注釋這句會計數會週而複始 #3
        });

    while (flag)
    {
        std::this_thread::sleep_for(100ms);
        SPDLOG_INFO("arrive");
        auto _ = b.arrive(); // b的計數減一 #1
    }
}

/// @brief 分離
void example_detach()
{
    std::mutex mtx; // 鎖
    std::condition_variable cv; // 條件變量
    auto flag = false; // 用於標識子線程結束

    std::thread t([&mtx, &cv, &flag]
        {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            std::unique_lock<std::mutex> lock(mtx);
            flag = true;
            lock.unlock();
            SPDLOG_INFO("child");
            cv.notify_one();
        });

    DBG(t.joinable()); // true
    t.detach(); // 線程和線程柄分離
    DBG(t.joinable()); // false

    wait_for(mtx, cv, std::chrono::milliseconds(200), [&flag] { return flag; },
        [](bool _result)
        {
            DBG(_result);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            return _result;
        });
}

void print_atomic(std::atomic<int>& _i)
{
    while (true)
    {
        auto value = _i.fetch_add(1); // 原子加一並返回原先的值
        if (value >= 10)
        {
            return;
        }
        DBG(value);
    }
}

/// @brief 原子操作
void example_atomic()
{
    std::atomic<int> i = 0;

    std::thread t([&i]
        {
            print_atomic(i);
        });

    print_atomic(i);

    join(t);
}

int main()
{
#if HAS_SPDLOG
    spdlog::set_pattern("[%Y-%m-%d %T.%e] [%^%l%$] [t:%6t] [p:%6P] [%-20!!:%4#] %v");
#endif

    //example_thread();

    //example_mutex();

    //example_condition_variable_notify_one();
    //example_condition_variable_notify_all();

    //example_promise();

    //example_packaged_task();

    //example_async();

    //example_jthread();

    //example_semaphore();

    //example_latch();
    //
    //example_barrier();

    example_detach();

    //example_atomic();

    return 0;
}
