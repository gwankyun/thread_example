#pragma once
#include <Windows.h>

namespace lite
{
    struct mutex
    {
        typedef HANDLE native_handle_type;
        mutex()
        {
            m_handle = CreateMutex(NULL, FALSE, NULL);
        }

        ~mutex()
        {
            CloseHandle(m_handle);
        }

        native_handle_type native_handle()
        {
            return m_handle;
        }

        void lock()
        {
            WaitForSingleObject(m_handle, INFINITE);
        }

        DWORD lock(DWORD dwMilliseconds)
        {
            return WaitForSingleObject(m_handle, dwMilliseconds);
        }

        void unlock()
        {
            ReleaseMutex(m_handle);
        }

    private:
        native_handle_type m_handle;
    };

    class timed_mutex
    {
    public:
        timed_mutex() {}
        ~timed_mutex() {}

    private:
    };

    template<typename Mutex>
    class lock_guard
    {
    public:
        typedef Mutex mutex_type;

        lock_guard(mutex_type& m) : m_mutex(m)
        {
            m_mutex.lock();
        }

        lock_guard(mutex_type &m, DWORD dwMilliseconds) : m_mutex(m)
        {
            m_result = m_mutex.lock(dwMilliseconds);
        }

        ~lock_guard()
        {
            m_mutex.unlock();
        }

        DWORD result()
        {
            return m_result;
        }

    private:
        mutex_type& m_mutex;
        DWORD m_result;
    };
}
