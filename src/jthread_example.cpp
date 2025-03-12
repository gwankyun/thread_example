#include <chrono> // std::chrono
#include <thread> // std::jthread
using namespace std::literals;

#include <spdlog/spdlog.h> // SPDLOG_INFO

/// @brief 自動合併線程類
void example_jthread()
{
    // 析構時自動調用t.join()
    std::jthread t(
        []
        {
            std::this_thread::sleep_for(1s);
            SPDLOG_INFO("child");
        });

    SPDLOG_INFO("main");
}

int main()
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    example_jthread();

    return 0;
}
