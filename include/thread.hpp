#pragma once
#include <Windows.h>

#ifndef NOEXCEPT
#  if __cplusplus >= 201103L
#    define NOEXCEPT noexcept
#  else
#    define NOEXCEPT
#  endif
#endif

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
    inline void close_thread(HANDLE &_thread)
    {
        if (_thread != NULL)
        {
            CloseHandle(_thread);
            _thread = NULL;
        }
    }

    struct thread
    {
        typedef HANDLE native_handle_type;
        thread(LPTHREAD_START_ROUTINE f, LPVOID args)
        {
            m_handle = CreateThread(
                NULL, // 描述線程內核對象安全屬性指針，一般為NULL。
                0,    // 線程棧大小，為0即與創建它的線程相同。
                f,    // 線程函數指針，原型有要求。
                args, // 線程函數的參數，可以為NULL。
                0,    // 線程運行狀態，0表示立即執行。
                NULL  // 線程ID，一般為NULL。
            );
        }

        ~thread()
        {
            if (m_handle != NULL)
            {
                CloseHandle(m_handle);
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
            WaitForSingleObject(native_handle(), INFINITE);
        }

    private:
        native_handle_type m_handle;
    };
}
