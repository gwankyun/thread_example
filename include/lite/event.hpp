#pragma once
#include "common.hpp"

namespace lite
{
    inline HANDLE create_event(bool _manual, bool _initial)
    {
        HANDLE event = CreateEvent(
            NULL,
            _manual ? TRUE : FALSE,  // 是否需要手動調用ResetEvent，自動模式時衹會激活一次
            _initial ? TRUE : FALSE, // 初始是否有信號
            NULL);
        return event;
    }

    inline void close_event(HANDLE &_event)
    {
        if (_event != NULL)
        {
            CloseHandle(_event);
        }
    }

    inline DWORD wait_event(HANDLE &_event, unsigned long _milliseconds = INFINITE)
    {
        return wait(_event, _milliseconds);
    }

    class Event
    {
    public:
        typedef HANDLE native_handle_type;

        Event(bool _manual, bool _initial)
        {
            m_handle = create_event(_manual, _initial);
        }

        ~Event()
        {
            close_event(m_handle);
        }

        native_handle_type native_handle()
        {
            return m_handle;
        }

        void set()
        {
            SetEvent(m_handle);
        }

        void reset()
        {
            ResetEvent(m_handle);
        }

        DWORD wait(unsigned long _milliseconds = INFINITE)
        {
            return wait_event(m_handle, _milliseconds);
        }

    private:
        native_handle_type m_handle;
    };
}
