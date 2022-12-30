#include <iostream>
#include <string>
#include <log.hpp>
#include <Windows.h>
#include <lite/thread.hpp>
#include <lite/mutex.hpp>
#include <lite/future.hpp>

void example_CreateThread()
{
    struct Func
    {
        static int func(int* param)
        {
            Sleep(3000);
            *param = 1;
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
        static int func(int* param)
        {
            lite::lock_guard<lite::mutex> guard(g_mutex);
            Sleep(3000);
            *param = 1;
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
        static int func(lite::promise<int>* param)
        {
            Sleep(3000);
            param->set_value(1);
            return 0;
        }
    };

    lite::thread t(Func::func, &pms);
    Sleep(1000);

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

void example_event()
{
    bool manual = true;
    lite::Event event(manual, false);

    struct Func
    {
        static int func(lite::Event* _event, bool manual, int _id)
        {
            _event->wait();
            DBG(_id);

            // 自動模式，事件衹會觸發一次。
            if (!manual)
            {
                _event->set();
            }

            return 0;
        }
    };

    lite::thread t1(Func::func, &event, manual, 1);
    lite::thread t2(Func::func, &event, manual, 2);

    DBG("start");
    Sleep(3000);
    event.set();

    if (t1.joinable())
    {
        t1.join();
    }

    if (t2.joinable())
    {
        t2.join();
    }
}

void example_packaged_task()
{
    struct Func
    {
        static int func()
        {
            DBG("");
            return 99;
        }
    };

    lite::packaged_task<int()> task(Func::func);
    lite::future<int> ft = task.get_future();

    lite::thread t([&task] { task(); });
    // lite::thread t(task);
    Sleep(3000);
    DBG("");
    DBG(ft.get());
    t.join();
}

// int func()
// {
//     Sleep(2000);
//     DBG("");
//     return 99;
// }

void example_async()
{
    struct Func
    {
        static int func()
        {
            Sleep(1000);
            DBG("");
            return 0;
        }

        static int func1(int a)
        {
            Sleep(1000);
            DBG(a);
            return a;
        }

        static int func2(int a, int b)
        {
            Sleep(3000);
            DBG(a + b);
            return a + b;
        }
    };

    DBG(0);
    lite::future<int> ft = lite::async(Func::func);
    ft.wait();
    DBG(ft.get());

    DBG(1);
    lite::future<int> ft1 = lite::async(Func::func1, 1);
    ft1.wait();
    DBG(ft1.get());

    DBG(2);
    lite::future<int> ft2 = lite::async(Func::func2, 1, 2);
    while (true)
    {
        lite::type::future_status status = ft2.wait_for(500);
        if (status == lite::future_status::ready)
        {
            DBG(ft2.get());
            break;
        }
        else
        {
            DBG("timeout");
        }
        Sleep(100);
    }
}

int f(int n)
{
    return n + 1;
}

template<typename F, typename A1>
typename lite::result_of<F>::type call(F f, A1 a1)
{
    return f(a1);
}

typedef int (*Fn)(int);

int main()
{
#if HAS_SPDLOG
    spdlog::set_pattern("[%Y-%m-%d %T.%e] [%^%l%$] [t:%6t] [p:%6P] [%-35!!:%4#] %v");
#endif

     //example_CreateThread();
     //example_CreateMutex();
    // example_promise();
    //example_event();
    // DBG()
    // funcion_traits<>
    // DBG(call(f, 1));

    // example_packaged_task();

    Fn fn = f;
    DBG(fn(9));

    example_async();


    return 0;
}
