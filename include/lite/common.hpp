#pragma once
#include <Windows.h>

#ifndef NOEXCEPT
#if __cplusplus >= 201103L
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif
#endif

#ifndef OVERRIDE
#if __cplusplus >= 201103L
#define OVERRIDE override
#else
#define OVERRIDE
#endif
#endif

#ifndef NULLPTR
#if __cplusplus >= 201103L
#define NULLPTR nullptr
#else
#define NULLPTR NULL
#endif
#endif

namespace lite
{
    inline DWORD wait(HANDLE &_handle, unsigned long _milliseconds = INFINITE)
    {
        return WaitForSingleObject(_handle, _milliseconds);
    }

    template <typename R>
    struct result_of
    {
    };

    template <typename R>
    struct result_of<R()>
    {
        typedef R type;
    };

    // template <typename R>
    // struct result_of<R(void)>
    // {
    //     typedef R type;
    // };

    template <typename R, typename A1>
    struct result_of<R(A1)>
    {
        typedef R type;
    };

    template <typename R, typename A1>
    struct result_of<R (*)(A1)>
    {
        typedef R type;
    };

    template <typename R, typename A1>
    struct result_of<R (&)(A1)>
    {
        typedef R type;
    };

    template<typename T>
    inline void swap(T& _m, T& _n)
    {
        T temp = _m;
        _m = _n;
        _n = temp;
    }

    template <typename T>
    class ptr
    {
    public:
        ptr() : m_ptr(NULLPTR) {}
        ptr(T* _ptr) : m_ptr(_ptr) {}
        ~ptr() {}

        T*& get()
        {
            return m_ptr;
        }

        ptr& operator=(T* _ptr)
        {
            m_ptr = _ptr;
            return *this;
        }

        explicit operator bool() const // 隱式轉換，可以直接if(Bool())
        {
            return m_ptr != NULLPTR;
        }

        T* operator->() const NOEXCEPT
        {
            return m_ptr;
        }

    private:
        T *m_ptr;
    };

    template <typename T>
    inline void delete_ptr(T *&_ptr)
    {
        if (_ptr != NULLPTR)
        {
            delete _ptr;
            _ptr = NULLPTR;
        }
    }

    template <typename T>
    inline void delete_ptr(ptr<T> &_ptr)
    {
        if (_ptr.get() != NULLPTR)
        {
            delete _ptr.get();
            _ptr = NULLPTR;
        }
    }

    template <typename T>
    struct control_block
    {
        control_block()
            : data(NULLPTR), shared_count(0){};

        control_block(T* _ptr)
            : data(_ptr), shared_count(0){};

        ~control_block()
        {
            delete_ptr(data);
        }

        void increase()
        {
            shared_count++;
        }

        void decrease()
        {
            if (shared_count > 0)
            {
                shared_count--;
            }
        }

        T *data;
        int shared_count;
    };

    template <typename T>
    class shared_ptr
    {
    public:
        shared_ptr() NOEXCEPT
        {
        };

        template <typename Y>
        explicit shared_ptr(Y *_ptr)
        : m_block(new control_block(_ptr))
        {
        }

        shared_ptr(const shared_ptr &r) NOEXCEPT
            : m_block(r.m_block)
        {
            m_block->increase();
        }

        template <typename Y>
        shared_ptr(const shared_ptr<Y> &r) NOEXCEPT
            : m_block(r.m_block)
        {
            m_block->increase();
        }

        ~shared_ptr()
        {
            if (m_block != NULLPTR)
            {
                m_block->decrease();
                if (m_block->shared_count == 0)
                {
                    delete_ptr(m_block);
                }
            }
        }

        shared_ptr& operator=(const shared_ptr& r) NOEXCEPT
        {
            if (&r != this)
            {
                m_block = r.m_block;
                m_block->increase();
            }
            return *this;
        }

        template <class Y>
        shared_ptr &operator=(const shared_ptr<Y> &r) NOEXCEPT
        {
            if (&r != this)
            {
                m_block = r.m_block;
                m_block->increase();
            }
            return *this;
        }

    private:
        control_block *m_block;
    };
}
