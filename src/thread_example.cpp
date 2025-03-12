#include <thread> // std::thread 
#include <chrono> // std::chrono

#include <spdlog/spdlog.h> // SPDLOG_INFO

/// @brief 創建線程
void example_thread()
{
    std::thread t([]
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            SPDLOG_INFO("child");
        });

    SPDLOG_INFO("main");
    if (t.joinable()) // 保證主線程結束前子線程執行完畢
    {
        t.join();
    }
}

int main()
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    example_thread();

    return 0;
}
