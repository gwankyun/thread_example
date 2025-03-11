#include <thread> // std::thread std::jthread
#include <chrono> // std::chrono
#include <mutex>  // std::mutex std::lock_guard std::unique_lock
#include <condition_variable> // std::condition_variable
#include <vector> // std::vector
#include <latch>
#include <iostream>
using namespace std::literals;

#include <common.hpp> // join wait_for

void on_latch(std::latch& _lt)
{
    while (true)
    {
        std::this_thread::sleep_for(500ms);
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
    }
}

/// @brief 閂
void example_latch()
{
    // std::latch lt(5);

    // std::jthread t([&lt] { on_latch(lt); });

    // on_latch(lt);

    struct Job
    {
        const std::string name;
        std::string product{"未工作"};
        std::thread action{};
    };

    Job jobs[]{{"Annika"}, {"Buru"}, {"Chuck"}};
 
    std::latch work_done{std::size(jobs)};
    std::latch start_clean_up{1};
 
    auto work = [&](Job& my_job)
    {
        my_job.product = my_job.name + " 已工作";
        work_done.count_down();
        start_clean_up.wait();
        my_job.product = my_job.name + " 已清理";
    };
 
    std::cout << "工作启动... ";
    for (auto& job : jobs)
        job.action = std::thread{work, std::ref(job)};
 
    work_done.wait();
    std::cout << "完成:\n";
    for (auto const& job : jobs)
        std::cout << "  " << job.product << '\n';
 
    std::cout << "清理工作线程... ";
    start_clean_up.count_down();
    for (auto& job : jobs)
        job.action.join();
 
    std::cout << "完成:\n";
    for (auto const& job : jobs)
        std::cout << "  " << job.product << '\n';
}

int main()
{
#if HAS_SPDLOG
    spdlog::set_pattern("[%Y-%m-%d %T.%e] [%^%l%$] [t:%6t] [p:%6P] [%-20!!:%4#] %v");
#endif

    example_latch();
    
    return 0;
}
