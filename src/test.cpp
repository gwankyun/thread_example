#include <chrono> // std::chrono
#include <mutex>  // std::mutex std::lock_guard
#include <future> // std::promise std::future std::launch std::packaged_task std::async
#include <string>
#include <thread> // std::thread
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

    std::thread t(
        [&, _value]
        {
            promise.set_value(_value);
        });

    if (t.joinable())
    {
        t.join();
    }

    return future.get();
}

int example_packaged_task(int _value)
{
    std::packaged_task<int()> task([&]
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
    auto future = std::async(std::launch::async,
                             [&_value]
                             {
                                 return _value;
                             });

    return future.get();
}

void update_atomic(std::atomic<int>& _i)
{
    while (true)
    {
        if (_i.load() < 10) // 原子讀取值
        {
            _i++; // 原子加一
        }
        else
        {
            return;
        }
    }
}

/// @brief 原子操作
int example_atomic()
{
    std::atomic<int> i = 0;

    std::thread t([&i]
        {
            update_atomic(i);
        });

    update_atomic(i);

    if (t.joinable())
    {
        t.join();
    }

    return i;
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

int main(int argc, char* argv[])
{
    std::string log_format{"[%C-%m-%d %T.%e] [%^%L%$] [t:%6t] [%-20!!:%4#] %v"};
    spdlog::set_pattern(log_format);

    auto result = Catch::Session().run(argc, argv);
    return result;
}
