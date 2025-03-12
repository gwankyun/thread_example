
#include <thread> // std::jthread
#include <chrono> // std::chrono
#include <semaphore> // std::binary_semaphore
using namespace std::literals;

#include <spdlog/spdlog.h> // SPDLOG_INFO

/// @brief 信號量
void example_semaphore()
{
    using std::this_thread::sleep_for;
    std::binary_semaphore semaphore(0);

    std::jthread t([&semaphore]
        {
            semaphore.acquire(); // 主線程發信號後才會調用
            SPDLOG_INFO("Child recv");
            sleep_for(100ms);
            SPDLOG_INFO("Child send");
            semaphore.release();
        });

    SPDLOG_INFO("Main send");
    semaphore.release();
    sleep_for(100ms);
    semaphore.acquire(); // 等待子線程發信號
    SPDLOG_INFO("Main recv");
}

int main()
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%l%$] [t:%6t] [%-20!!:%4#] %v");

    example_semaphore();

    return 0;
}
