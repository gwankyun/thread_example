#include <thread> // std::jthread
#include <chrono> // std::chrono
#include <latch> // std::latch
using namespace std::literals;

#include <spdlog/spdlog.h> // SPDLOG_INFO

/// @brief 閂
void example_latch()
{
    std::latch ltc_main{2};
    std::latch ltc_1{1};
    std::latch ltc_2{1};

    std::jthread t1([&ltc_main, &ltc_1, &ltc_2]
        {
            SPDLOG_INFO("ltc_1 begin");
            ltc_1.count_down();
            while (!ltc_2.try_wait())
            {
                SPDLOG_INFO("wait");
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            SPDLOG_INFO("ltc_1 end");
            ltc_main.count_down();
        });

    std::jthread t2([&ltc_main, &ltc_1, &ltc_2]
        {
            while (!ltc_1.try_wait())
            {
                SPDLOG_INFO("wait");
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            SPDLOG_INFO("ltc_2 begin");
            ltc_2.count_down();
            SPDLOG_INFO("ltc_2 end");
            ltc_main.count_down();
        });

    SPDLOG_INFO("ltc_main begin");
    while (!ltc_main.try_wait())
    {
        SPDLOG_INFO("wait");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    SPDLOG_INFO("ltc_main end");
}

int main()
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    example_latch();
    
    return 0;
}
