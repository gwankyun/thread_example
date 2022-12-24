#pragma once
#include <Windows.h>
#include "event.hpp"

namespace lite
{
#if __cplusplus < 201103L
    struct future_status
    {
        enum Type
        {
            ready,
            timeout,
            deferred
        };
    };
    typedef future_status::Type future_statusType;
#else
    enum class future_status
    {
        ready,
        timeout,
        deferred
    };
    using future_statusType = future_status;
#endif

    template <typename T>
    class future
    {
    public:
        future()
            : m_event(NULL), m_value(NULL) {}

        future(Event *_event, T *_value)
            : m_event(_event), m_value(_value) {}

        ~future() {}

        void wait() const
        {
            if (m_event != NULL)
            {
                m_event->wait();
            }
        }

        future_statusType wait_for(unsigned long _milliseconds)
        {
            DWORD result = m_event->wait(_milliseconds);
            switch (result)
            {
            case WAIT_OBJECT_0:
                return future_status::ready;
                break;
            case WAIT_TIMEOUT:
                return future_status::timeout;
                break;
            default: // WAIT_ABANDONED
                return future_status::timeout;
                break;
            }
        }

        T get()
        {
            return *m_value;
        }

    private:
        T *m_value;
        Event *m_event;
    };

    template <typename R>
    class promise
    {
    public:
        promise() : m_event(false, false)
        {
        }
        ~promise() {}

        void set_value(const R &_value)
        {
            m_value = _value;
            m_event.set();
        }

        future<R> get_future()
        {
            future<R> ft(&m_event, &m_value);
            return ft;
        }

    private:
        R m_value;
        Event m_event;
    };
}
