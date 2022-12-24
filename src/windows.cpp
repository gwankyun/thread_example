#include <iostream>
#include <string>
#include <log.hpp>
#include <Windows.h>
#include <thread.hpp>
#include <mutex.hpp>
#include <future.hpp>

void example_CreateThread()
{
    struct Func
    {
        static DWORD WINAPI func(LPVOID param)
        {
            Sleep(3000);
            *(int*)param = 1;
            return 0;
        }
    };

    int i = 0;

    lite::thread t(Func::func, &i);


    if (t.native_handle() == NULL)
    {
        return;
    }

    DBG(i);
    if (t.joinable())
    {
        t.join();
    }
    DBG(i);
}

lite::mutex g_mutex;

void example_CreateMutex()
{
    struct Func
    {
        static DWORD WINAPI func(LPVOID param)
        {
            lite::lock_guard<lite::mutex> guard(g_mutex);
            Sleep(3000);
            *(int*)param = 1;
            return 0;
        }
    };

    int i = 0;

    lite::thread t(Func::func, &i);

    Sleep(1000);

    if (t.native_handle() == NULL)
    {
        return;
    }

    while (true)
    {
        lite::lock_guard<lite::mutex> guard(g_mutex, 500);
        auto flag = guard.result();
        if (flag == WAIT_OBJECT_0)
        {
            DBG(i);
            break;
        }
        else if (flag == WAIT_TIMEOUT)
        {
            DBG("WAIT_TIMEOUT");
        }
        else
        {
            DBG("WAIT_FAILED");
        }
        Sleep(100);
    }

    if (t.joinable())
    {
        t.join();
    }
}

void example_promise()
{
    lite::promise<int> pms;
    DBG("");
    lite::future<int> ft = pms.get_future();

    struct Func
    {
        static DWORD WINAPI func(LPVOID param)
        {
            Sleep(3000);
            ((lite::promise<int>*)param)->set_value(1);
            return 0;
        }
    };

    lite::thread t(Func::func, &pms);
    Sleep(1000);

    // ft.wait();

    while (true)
    {
        auto status = ft.wait_for(500);
        if (status == lite::future_status::ready)
        {
            break;
        }
        DBG("waiting");
        Sleep(100);
    }
    

    DBG(ft.get());
    if (t.joinable())
    {
        t.join();
    }
    
}

inline HANDLE create_event(bool _manual, bool _initial)
{
    HANDLE event = CreateEvent(
        NULL,
        _manual ? true : false, // 是否需要手動調用ResetEvent，自動模式時衹會激活一次
        _initial ? true : false, // 初始是否有信號
        NULL);
    return event;
}

inline void close_event(HANDLE& _event)
{
    if (_event != NULL)
    {
        CloseHandle(_event);
    }
}

void example_event()
{
    bool manual = true;
    HANDLE event = create_event(manual, false);
    if (!event)
    {
        return;
    }

    struct Param
    {
        HANDLE event;
        int id;
        bool manual;
    };

    struct Func
    {
        static DWORD WINAPI func(LPVOID _param)
        {
            Param* param = (Param*)_param;
            if (param == NULL)
            {
                return 0;
            }

            WaitForSingleObject(param->event, INFINITE);
            DBG(param->id);

            // 自動模式，事件衹會觸發一次。
            if (!param->manual)
            {
                SetEvent(param->event);
            }

            return 0;
        }
    };

    Param param[2];
    param[0].event = event;
    param[0].id = 1;
    param[0].manual = manual;
    param[1].event = event;
    param[1].id = 2;
    param[1].manual = manual;

    lite::thread t1(Func::func, &param[0]);
    lite::thread t2(Func::func, &param[1]);

    DBG("start");
    Sleep(3000);
    SetEvent(event);

    if (t1.joinable())
    {
        t1.join();
    }

    if (t2.joinable())
    {
        t2.join();
    }

    close_event(event);
}

int main()
{
#if HAS_SPDLOG
    spdlog::set_pattern("[%Y-%m-%d %T.%e] [%^%l%$] [t:%6t] [p:%6P] [%-20!!:%4#] %v");
#endif

    // example_CreateThread();
    // example_CreateMutex();
    example_promise();
    // example_event()

    return 0;
}
