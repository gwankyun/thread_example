#pragma once
#include "common.hpp"

namespace lite
{
    namespace detail
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
        inline void close_thread(HANDLE &_thread)
        {
            if (_thread != NULL)
            {
                DBG("");
                CloseHandle(_thread);
                _thread = NULL;
            }
        }

        struct FuncInterface
        {
            FuncInterface() {}
            virtual ~FuncInterface(){};
            virtual void call() = 0;
        };

        template <typename Fn>
        struct Func : FuncInterface
        {
            Func(Fn _fn) : fn(_fn) {}
            ~Func() {}
            void call() OVERRIDE
            {
                fn();
            }
            Fn fn;
        };

        template <typename Fn, typename A1>
        struct Func1 : FuncInterface
        {
            Func1(Fn _fn, A1 _a1) : fn(_fn), a1(_a1) {}
            ~Func1() {}
            void call() OVERRIDE
            {
                fn(a1);
            }
            Fn fn;
            A1 a1;
        };

        template <typename Fn, typename A1, typename A2>
        struct Func2 : FuncInterface
        {
            Func2(Fn _fn, A1 _a1, A2 _a2) : fn(_fn), a1(_a1), a2(_a2) {}
            ~Func2() {}
            void call() OVERRIDE
            {
                fn(a1, a2);
            }
            Fn fn;
            A1 a1;
            A2 a2;
        };

        template <typename Fn, typename A1, typename A2, typename A3>
        struct Func3 : FuncInterface
        {
            Func3(Fn _fn, A1 _a1, A2 _a2, A3 _a3) : fn(_fn), a1(_a1), a2(_a2), a3(_a3) {}
            ~Func3() {}
            void call() OVERRIDE
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
            FuncInterface *func = (FuncInterface *)_param;
            if (func == NULL)
            {
                return 0;
            }
            func->call();
            return 0;
        }
    }

    struct thread
    {
        typedef HANDLE native_handle_type;
        thread(LPTHREAD_START_ROUTINE f, LPVOID args) : m_func(NULL)
        {
            m_handle = detail::create_thread(f, args);
        }

        template<typename Fn>
        thread(Fn _f) : m_func(NULL)
        {
            m_func = new detail::Func<Fn>(_f);
            m_handle = detail::create_thread(detail::func, m_func);
        }

        template<typename Fn, typename A1>
        thread(Fn _f, A1 _a1) : m_func(NULL)
        {
            m_func = new detail::Func1<Fn, A1>(_f, _a1);
            m_handle = detail::create_thread(detail::func, m_func);
        }

        template<typename Fn, typename A1, typename A2>
        thread(Fn _f, A1 _a1, A2 _a2) : m_func(NULL)
        {
            m_func = new detail::Func2<Fn, A1, A2>(_f, _a1, _a2);
            m_handle = detail::create_thread(detail::func, m_func);
        }

        template<typename Fn, typename A1, typename A2, typename A3>
        thread(Fn _f, A1 _a1, A2 _a2, A3 _a3) : m_func(NULL)
        {
            m_func = new detail::Func3<Fn, A1, A2, A3>(_f, _a1, _a2, _a3);
            m_handle = detail::create_thread(detail::func, m_func);
        }

        ~thread()
        {
            DBG("");
            detail::close_thread(m_handle);
            delete_ptr(m_func);
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
        detail::FuncInterface* m_func;
    };
}
