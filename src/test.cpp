#include <chrono> // std::chrono

#include <barrier>   // std::barrier
#include <future>    // std::promise std::future std::launch std::packaged_task std::async
#include <latch>     // std::latch
#include <mutex>     // std::mutex std::lock_guard
#include <semaphore> // std::counting_semaphore std::binary_semaphore
#include <thread>    // std::thread

#include <string>
#include <vector>

#include <catch2/../catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <spdlog/spdlog.h>

using namespace std::literals::chrono_literals;

void example_mutex(std::string& _output_m, std::string& _output_s)
{
    std::mutex mtx;

    std::vector<std::string> vec_main{"a", "b", "c"};
    std::vector<std::string> vec_sub{"1", "2", "3"};

    std::thread t(
        [&]
        {
            for (auto& i : vec_sub)
            {
                std::this_thread::sleep_for(100ms);
                std::unique_lock<std::mutex> lock(mtx);
                SPDLOG_INFO(i);
                _output_s += i;
            }
        });

    for (auto& i : vec_main)
    {
        std::this_thread::sleep_for(100ms);
        std::unique_lock<std::mutex> lock(mtx);
        SPDLOG_INFO(i);
        _output_m += i;
    }

    if (t.joinable())
    {
        t.join();
    }
}

void example_condition_variable_notify_one(std::string& _output)
{
    std::mutex mtx;             // 鎖
    std::condition_variable cv; // 條件變量

    std::vector<std::string> vec_main{"a", "b", "c"};
    std::vector<std::string> vec_sub{"1", "2", "3"};

    enum struct tag
    {
        main,
        sub
    } current(tag::main);

    std::thread t(
        [&]
        {
            for (auto& i : vec_sub)
            {
                {
                    std::unique_lock<std::mutex> lock(mtx);
                    cv.wait(lock, [&] { return current == tag::sub; });
                    current = tag::main;
                    _output += i;
                }
                cv.notify_one();
            }
        });

    for (auto& i : vec_main)
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&] { return current == tag::main; });
            current = tag::sub;
            _output += i;
        }
        cv.notify_one();
    }

    if (t.joinable())
    {
        t.join();
    }
}

int example_promise(int _value)
{
    std::promise<int> promise;
    auto future = promise.get_future();

    std::thread t([&, _value] { promise.set_value(_value); });

    if (t.joinable())
    {
        t.join();
    }

    return future.get();
}

int example_packaged_task(int _value)
{
    std::packaged_task<int()> task(
        [&]
        {
            // print_child();
            return _value;
        });
    auto future = task.get_future();

    std::thread t([&task] { task(); });

    if (t.joinable())
    {
        t.join();
    }

    return future.get();
}

int example_async(int _value)
{
    auto future = std::async(std::launch::async, [&_value] { return _value; });

    return future.get();
}

/// @brief 原子操作
int example_atomic()
{
    std::atomic<int> i = 0;

    /// @brief 原子更新
    auto update_atomic = [&]
    {
        while (true)
        {
            if (i.load() < 10) // 原子讀取值
            {
                i++; // 原子加一
            }
            else
            {
                return;
            }
        }
    };

    std::thread t([&] { update_atomic(); });

    update_atomic();

    if (t.joinable())
    {
        t.join();
    }

    return i;
}

/// @brief 信號量
std::string example_semaphore()
{
    std::binary_semaphore semaphore(0);
    std::string str;

    std::jthread t(
        [&]
        {
            semaphore.acquire(); // 主線程發信號後才會調用
            str += "2";
            SPDLOG_INFO("Child recv");
            std::this_thread::sleep_for(1s);
            SPDLOG_INFO("Child send");
            semaphore.release();
        });

    SPDLOG_INFO("Main send");
    str += "1";
    semaphore.release();
    std::this_thread::sleep_for(1s);
    semaphore.acquire(); // 等待子線程發信號
    str += "3";
    SPDLOG_INFO("Main recv");
    return str;
}

TEST_CASE("semaphore", "[binary_semaphore]")
{
    REQUIRE(example_semaphore() == "123");
}

int example_latch(int _count)
{
    std::latch lt(_count);
    std::atomic<int> value = 0;

    // 確保線程都執行完
    {
        std::vector<std::jthread> t(_count);

        for (auto i = 0u; i < _count; i++)
        {
            t[i] = std::jthread(
                [&, i]
                {
                    SPDLOG_INFO(i);
                    lt.count_down();
                    lt.wait();
                    value++;
                    SPDLOG_INFO("value: {}", value.load());
                });
        }
    }

    lt.wait();
    return value.load();
}

TEST_CASE("latch", "[latch]")
{
    auto count = 5;
    REQUIRE(example_latch(count) == count);
}

int example_barrier(int _count)
{
    std::atomic<int> value = 0;

    auto completion = [&]() noexcept {
        SPDLOG_INFO("completion");
    };

    std::barrier br(_count, completion);

    // 確保線程都執行完
    {
        std::vector<std::jthread> t(_count);

        for (auto i = 0u; i < _count; i++)
        {
            t[i] = std::jthread(
                [&, i]
                {
                    SPDLOG_INFO(i);
                    br.arrive_and_wait();
                    value++;
                    SPDLOG_INFO("value: {}", value.load());
                });
        }
    }

    return value.load();
}

TEST_CASE("barrier", "[barrier]")
{
    auto count = 5;
    REQUIRE(example_barrier(count) == count);
}

TEST_CASE("jthread", "[jthread]")
{
    auto value = 0;

    /// @brief 自動合併線程類
    auto example_jthread = [&]
    {
        // 析構時自動調用t.join()
        std::jthread t(
            [&]
            {
                std::this_thread::sleep_for(3s);
                SPDLOG_INFO("child");
                value = 10;
            });

        SPDLOG_INFO("main");
    };

    example_jthread();
    REQUIRE(value == 10);
}

TEST_CASE("atomic", "[atomic]")
{
    REQUIRE(example_atomic() == 10);
}

TEST_CASE("future promise", "[future]")
{
    REQUIRE(example_promise(99) == 99);
}

TEST_CASE("future async", "[async]")
{
    REQUIRE(example_async(99) == 99);
}

TEST_CASE("future packaged_task", "[packaged_task]")
{
    REQUIRE(example_packaged_task(99) == 99);
}

TEST_CASE("thread mutex", "[condition_variable]")
{
    std::string m;

    example_condition_variable_notify_one(m);
    REQUIRE(m == "a1b2c3");
}

TEST_CASE("thread mutex", "[mutex]")
{
    auto count = 3u;
    std::string m;
    std::string s;

    example_mutex(m, s);
    REQUIRE(m == "abc");
    REQUIRE(s == "123");
}

int main(int _argc, char* _argv[])
{
    std::string log_format{"[%C-%m-%d %T.%e] [%^%L%$] [t:%6t] [%-20!!:%4#] %v"};
    spdlog::set_pattern(log_format);

    auto result = Catch::Session().run(_argc, _argv);
    return result;
}
