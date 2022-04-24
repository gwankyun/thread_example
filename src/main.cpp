#include <thread> // std::thread std::jthread
#include <chrono> // std::chrono
#include <mutex>  // std::mutex std::lock_guard std::unique_lock
#include <condition_variable> // std::condition_variable
#include <future> // std::promise std::future std::launch std::packaged_task std::async
#include <semaphore> // std::counting_semaphore std::binary_semaphore
#include <latch> // std::latch
#include <vector> // std::vector
#include <barrier> // std::barrier
using namespace std::literals;

#include <common.hpp> // join wait_for

void example_01()
{
    std::thread t([](int _line)
        {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            SPDLOG_INFO(_line);
        }, __LINE__);

    SPDLOG_INFO("");
    if (t.joinable()) // ���C�����̽Y��ǰ�Ӿ��̈����ꮅ
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
            std::lock_guard<std::mutex> lock(_mtx); // �Ԅӽ��i
            if (_i >= 10) // �Д�ҲҪ���i
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
            if (_i >= 10) // �Д�ҲҪ���i
            {
                return;
            }
            _cv.wait(lock, [&_current, _type]
                {
                    return _current == _type || _current == State::Start;
                }); // �ȴ�֪ͨ
            SPDLOG_INFO("{}: {}", name, _i);
            _i++;
            _current = _type == State::Child_1 ? State::Child_2 : State::Child_1;
            lock.unlock(); // ��ǰ���i #2
            _cv.notify_one(); // ֪ͨ��һ������
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

    join(t);
}

void example_03_01()
{
    std::mutex mtx; // �i
    std::condition_variable cv; // �l��׃��
    bool flag = false;

    std::thread t([&mtx, &cv, &flag]
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&flag] { return flag; }); // �ȴ��������޸�
            SPDLOG_INFO("Child recv");
            std::this_thread::sleep_for(1s);
            flag = true;
            lock.unlock();
            cv.notify_one();
            SPDLOG_INFO("Child send");
        });

    SPDLOG_INFO("Main send");
    std::unique_lock<std::mutex> lock(mtx); // #1
    flag = true;
    lock.unlock(); // ��ǰ���i #2
    cv.notify_one();
    std::this_thread::sleep_for(1s);
    lock.lock(); // �ٴ����i #3
    cv.wait(lock, [&flag] { return flag; }); // �ȴ��Ӿ����޸�
    SPDLOG_INFO("Main recv");

    join(t);
}

void example_03_02()
{
    std::mutex mtx; // �i
    std::condition_variable cv; // �l��׃��
    int i = 0;
    State current = State::None; // ��춘��R����

    // ����һ
    std::thread child_1([&mtx, &cv, &i, &current]
        {
            print_03(mtx, cv, i, current, State::Child_1);
        });

    // ���̶�
    std::thread child_2([&mtx, &cv, &i, &current]
        {
            print_03(mtx, cv, i, current, State::Child_2);
        });

    SPDLOG_INFO(""); // �@�l�Z�����child_1�cchild_2���� #3

    std::unique_lock<std::mutex> lock(mtx);
    current = State::Start;
    lock.unlock();
    cv.notify_all(); // ֪ͨ���о��� #4

    join(child_1);

    join(child_2);
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

    auto result = wait_for(ft, std::chrono::milliseconds(200));
    SPDLOG_INFO("result: {0}", result);

    join(t);
}

void example_05()
{
    std::packaged_task<int()> task([]
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            return 99;
        });
    std::future<int> ft = task.get_future();

    std::thread t([&task] { task(); });

    auto result = wait_for(ft, std::chrono::milliseconds(200));
    SPDLOG_INFO("result: {0}", result);

    join(t);
}

void example_06()
{
    auto ft = std::async(std::launch::async, []
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            return 99;
        });

    auto result = wait_for(ft, std::chrono::milliseconds(200));
    SPDLOG_INFO("result: {0}", result);
}

void example_07()
{
    // �����r�Ԅ��{��t.join()
    std::jthread t([](int _line)
        {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            SPDLOG_INFO(_line);
        }, __LINE__);

    SPDLOG_INFO("");
}

void example_08()
{
    std::binary_semaphore semaphore(0);

    std::jthread t([&semaphore]
        {
            semaphore.acquire(); // �����̰l��̖��ŕ��{��
            SPDLOG_INFO("Child recv");
            std::this_thread::sleep_for(1s);
            SPDLOG_INFO("Child send");
            semaphore.release();
        });

    SPDLOG_INFO("Main send");
    semaphore.release();
    std::this_thread::sleep_for(1s);
    semaphore.acquire(); // �ȴ��Ӿ��̰l��̖
    SPDLOG_INFO("Main recv");
}

void on_latch(std::latch& _lt)
{
    while (true)
    {
        if (_lt.try_wait())
        {
            SPDLOG_INFO("latch open");
            return;
        }
        else
        {
            SPDLOG_INFO("count_down");
            _lt.count_down();
        }
        std::this_thread::sleep_for(100ms);
    }
}

void example_09()
{
    std::latch lt(5);

    std::jthread t([&lt] { on_latch(lt); });

    on_latch(lt);
}

void example_10()
{
    bool flag = true;
    std::barrier b(5, []() noexcept // noexcept�ز����٣���Ȼ�o�����g
        {
            SPDLOG_INFO("CompletionFunction");
        });

    std::jthread t([&flag, &b]
        {
            SPDLOG_INFO("before wait");
            b.wait(b.arrive());
            SPDLOG_INFO("after wait");
            flag = false;
        });

    while (flag)
    {
        std::this_thread::sleep_for(100ms);
        SPDLOG_INFO("arrive");
        auto _ = b.arrive();
    }
}

int main()
{
#if SPDLOG_EXISTS
    spdlog::set_pattern("[%Y-%m-%d %T.%e] [%^%l%$] [t:%6t] [p:%6P] [%-20!!:%4#] %v");
#endif

    //example_01();

    //example_02();

    //example_03_01();
    //example_03_02();

    //example_04();

    //example_05();

    //example_06();

    //example_07();

    //example_08();

    //example_09();
    
    example_10();

    return 0;
}
