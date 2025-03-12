#include <thread> // std::jthread
#include <chrono> // std::chrono
#include <barrier> // std::barrier
using namespace std::literals;

#include <spdlog/spdlog.h> // SPDLOG_INFO

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

int main()
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    example_barrier();

    return 0;
}
