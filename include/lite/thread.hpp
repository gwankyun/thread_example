#pragma once
#include "common.hpp"

namespace lite
{
    /// @brief 創建線程
    /// @param _f 函數
    /// @param _args 參數
    /// @return 線程柄
    /// @retval NULL 表示失敗
    inline HANDLE create_thread(LPTHREAD_START_ROUTINE _f, LPVOID _args)
    {
        HANDLE thread = CreateThread(
            NULL,  // 描述線程內核對象安全屬性指針，一般為NULL。
            0,     // 線程棧大小，為0即與創建它的線程相同。
            _f,    // 線程函數指針，原型有要求。
            _args, // 線程函數的參數，可以為NULL。
            0,     // 線程運行狀態，0表示立即執行。
            NULL   // 線程ID，一般為NULL。
        );
        return thread;
    }

    /// @brief 關閉線程
    /// @param _thread 線程柄
    /// @return 無
    inline void close_thread(HANDLE& _thread)
    {
        if (_thread != NULL)
        {
            CloseHandle(_thread);
            _thread = NULL;
        }
    }

    struct FuncInterface
    {
        FuncInterface() {}
        virtual ~FuncInterface() {};
        virtual void call() = 0;
    };

    template<typename Fn, typename Args>
    struct Func1 : FuncInterface
    {
        Func1(Fn _fn, Args _args) : fn(_fn), args(_args) {}
        ~Func1() {}
        void call() override
        {
            fn(args);
        }
        Fn fn;
        Args args;
    };

    template<typename Fn, typename A1, typename A2>
    struct Func2 : FuncInterface
    {
        Func2(Fn _fn, A1 _a1, A2 _a2) : fn(_fn), a1(_a1), a2(_a2) {}
        ~Func2() {}
        void call() override
        {
            fn(a1, a2);
        }
        Fn fn;
        A1 a1;
        A2 a2;
    };

    template<typename Fn, typename A1, typename A2, typename A3>
    struct Func3 : FuncInterface
    {
        Func3(Fn _fn, A1 _a1, A2 _a2, A3 _a3) : fn(_fn), a1(_a1), a2(_a2), a3(_a3) {}
        ~Func3() {}
        void call() override
        {
            fn(a1, a2, a3);
        }
        Fn fn;
        A1 a1;
        A2 a2;
        A3 a3;
    };

    inline DWORD WINAPI func(LPVOID _param)
    {
        FuncInterface* func = (FuncInterface*)_param;
        if (func == NULL)
        {
            return 0;
        }
        func->call();
        return 0;
    }

    struct thread
    {
        typedef HANDLE native_handle_type;
        thread(LPTHREAD_START_ROUTINE f, LPVOID args) : m_func(NULL)
        {
            m_handle = create_thread(f, args);
        }

        template<typename Fn, typename Args>
        thread(Fn _f, Args _args) : m_func(NULL)
        {
            m_func = new Func1<Fn, Args>(_f, _args);
            m_handle = create_thread(func, m_func);
        }

        template<typename Fn, typename A1, typename A2>
        thread(Fn _f, A1 _a1, A2 _a2) : m_func(NULL)
        {
            m_func = new Func2<Fn, A1, A2>(_f, _a1, _a2);
            m_handle = create_thread(func, m_func);
        }

        template<typename Fn, typename A1, typename A2, typename A3>
        thread(Fn _f, A1 _a1, A2 _a2, A3 _a3) : m_func(NULL)
        {
            m_func = new Func3<Fn, A1, A2, A3>(_f, _a1, _a2, _a3);
            m_handle = create_thread(func, m_func);
        }

        ~thread()
        {
            close_thread(m_handle);
            if (m_func)
            {
                delete m_func;
                m_func = NULL;
            }
        }

        native_handle_type native_handle()
        {
            return m_handle;
        }

        bool joinable() const NOEXCEPT
        {
            return true;
        }

        void join()
        {
            wait(m_handle);
        }

    private:
        native_handle_type m_handle;
        FuncInterface* m_func;
    };
}
