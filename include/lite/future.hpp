#pragma once
#include "thread.hpp"
#include "event.hpp"
#include <vector>

namespace lite
{
#if __cplusplus >= 201103L
    enum class future_status
    {
        ready,
        timeout,
        deferred
    };
    using future_statusType = future_status;
#else
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
#endif

    // template <typename R>
    // class promise;

    namespace type
    {
        typedef future_statusType future_status;
    }

    struct packaged_task_interface
    {
        packaged_task_interface(){};
        virtual ~packaged_task_interface(){};
        // virtual void operator()() = 0;
    };

    template <typename R>
    struct future_block
    {
        future_block() : event(false, false), m_count(0)
        {
            DBG("+1");
        }
        ~future_block()
        {
            DBG("-1");
        }
        R value;
        Event event;

        void increase()
        {
            m_count++;
        }

        void decrease()
        {
            if (m_count > 0)
            {
                m_count--;
            }
        }

        int count()
        {
            return m_count;
        }

    private:
        future_block(const future_block &);
        int m_count;
    };

    template <typename R>
    inline bool delete_block(future_block<R>*& _block)
    {
        if (_block->count() == 0)
        {
            delete_ptr(_block);
            return true;
        }
        else
        {
            return false;
        }
    }

    class promise_interface
    {
    public:
        promise_interface() {}
        virtual ~promise_interface() {}
    };

    template <typename T>
    class future
    {
    public:
        future()
        {
        }

        future(future_block<T> *_event)
            : m_block(_event)
        {
            m_block->increase();
        }

        future(const future &_other)
            : m_block(_other.m_block)
        {
            m_block->increase();
        }

        ~future()
        {
            DBG(m_block->count());
            m_block->decrease();
            if (delete_block(m_block.get()))
            {
                if (m_thread.get() != NULLPTR)
                {
                    if (m_thread.get()->joinable())
                    {
                        m_thread.get()->join();
                    }
                    delete_ptr(m_thread);
                }
                delete_ptr(m_promise);
            }
        }

        void wait() const
        {
            if (m_block)
            {
                m_block->event.wait();
            }
        }

        type::future_status wait_for(unsigned long _milliseconds)
        {
            DBG("");
            DWORD result = m_block->event.wait(_milliseconds);
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
            return m_block->value;
        }

        ptr<thread> m_thread;
        ptr<packaged_task_interface> m_promise;

    private:
        ptr< future_block<T> > m_block;
    };

    template <typename R>
    class promise
    {
    public:
        promise()
            : m_block(new future_block<R>()), m_owned(true)
        {
        }
        ~promise()
        {
            delete_block(m_block);
        }

        void set_value(const R &_value)
        {
            DBG("");
            m_block->value = _value;
            m_owned = false;
            m_block->event.set();
            DBG("set_value end");
        }

        future<R> get_future()
        {
            DBG("");
            future<R> ft(m_block);
            m_owned = false;
            return ft; // 此處會調用一次刪除操作。
        }

    private:
        future_block<R> *m_block;
        bool m_owned;
    };

    template <typename R>
    class packaged_task : public packaged_task_interface
    {
    public:
        packaged_task();
        ~packaged_task();
        void operator()() OVERRIDE {}
    };

    template <typename F>
    class packaged_task<F()> : public packaged_task_interface
    {
    public:
        typedef F (*Fn)();

        packaged_task() : m_valid(false) NOEXCEPT {}

        packaged_task(Fn _f)
            : m_fn(_f), m_valid(true)
        {
        }
        ~packaged_task()
        {
            DBG("~packaged_task");
        }

        void operator()() OVERRIDE
        {
            m_promise.set_value(m_fn());
        }

        bool valid() const NOEXCEPT
        {
            return m_valid;
        }

        future<F> get_future()
        {
            return m_promise.get_future();
        }

        void swap(packaged_task &other) NOEXCEPT
        {
            ::swap(m_fn, other.m_fn);
            ::swap(m_promise, other.m_promise);
            ::swap(m_valid, other.m_valid);
        }

    private:
        packaged_task(const packaged_task &);
        Fn m_fn;
        promise<F> m_promise;
        bool m_valid;
    };

    template <typename F, typename A1>
    class packaged_task<F(A1)> : public packaged_task_interface
    {
    public:
        typedef F (*Fn)(A1);

        packaged_task() : m_valid(false) NOEXCEPT {}

        packaged_task(Fn _f)
            : m_fn(_f), m_valid(true)
        {
        }
        ~packaged_task()
        {
            DBG("~packaged_task");
        }

        void operator()(A1 _a1) OVERRIDE
        {
            m_promise.set_value(m_fn(_a1));
        }

        bool valid() const NOEXCEPT
        {
            return m_valid;
        }

        future<F> get_future()
        {
            return m_promise.get_future();
        }

        void swap(packaged_task &other) NOEXCEPT
        {
            ::swap(m_fn, other.m_fn);
            ::swap(m_promise, other.m_promise);
            ::swap(m_valid, other.m_valid);
        }

    private:
        packaged_task(const packaged_task &);
        Fn m_fn;
        promise<F> m_promise;
        bool m_valid;
    };

    template <typename F, typename A1, typename A2>
    class packaged_task<F(A1, A2)> : public packaged_task_interface
    {
    public:
        typedef F (*Fn)(A1, A2);

        packaged_task() : m_valid(false) NOEXCEPT {}

        packaged_task(Fn _f)
            : m_fn(_f), m_valid(true)
        {
        }
        ~packaged_task()
        {
            DBG("~packaged_task");
        }

        void operator()(A1 _a1, A2 _a2) OVERRIDE
        {
            m_promise.set_value(m_fn(_a1, _a2));
        }

        bool valid() const NOEXCEPT
        {
            return m_valid;
        }

        future<F> get_future()
        {
            return m_promise.get_future();
        }

        void swap(packaged_task &other) NOEXCEPT
        {
            ::swap(m_fn, other.m_fn);
            ::swap(m_promise, other.m_promise);
            ::swap(m_valid, other.m_valid);
        }

    private:
        packaged_task(const packaged_task &);
        Fn m_fn;
        promise<F> m_promise;
        bool m_valid;
    };

    template <typename R>
    inline future<R> async(R (*_f)())
    {
        typedef packaged_task<R()> task_type;
        task_type *task = new task_type(_f);
        future<R> ft = task->get_future();
        ft.m_promise = task;

        struct Func
        {
            Func(task_type *_task) : task(_task) {}
            ~Func() {}
            void operator()()
            {
                (*task)();
            }
            task_type *task;
        } func(task);

        thread *t = new thread(func);
        ft.m_thread = t;
        return ft;
    }

    template <typename R, typename A1>
    inline future<R> async(R (*_f)(A1), A1 _a1)
    {
        typedef packaged_task<R(A1)> task_type;
        task_type *task = new task_type(_f);
        future<R> ft = task->get_future();
        ft.m_promise = task;

        struct Func
        {
            Func(task_type *_task) : task(_task) {}
            ~Func() {}
            void operator()(A1 _a1)
            {
                (*task)(_a1);
            }
            task_type *task;
        } func(task);

        thread *t = new thread(func, _a1);
        ft.m_thread = t;
        return ft;
    }

    template <typename R, typename A1, typename A2>
    inline future<R> async(R (*_f)(A1, A2), A1 _a1, A2 _a2)
    {
        typedef packaged_task<R(A1, A2)> task_type;
        task_type *task = new task_type(_f);
        future<R> ft = task->get_future();
        ft.m_promise = task;

        struct Func
        {
            Func(task_type *_task) : task(_task) {}
            ~Func() {}
            void operator()(A1 _a1, A2 _a2)
            {
                (*task)(_a1, _a2);
            }
            task_type *task;
        } func(task);

        thread *t = new thread(func, _a1, _a2);
        ft.m_thread = t;
        return ft;
    }
}
