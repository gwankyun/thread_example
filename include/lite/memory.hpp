#pragma once
#include "common.hpp"

namespace lite
{
    namespace detail
    {
        struct deletor_interface
        {
            deletor_interface() {}
            virtual ~deletor_interface() {}
            virtual void operator()(void* _ptr) = 0;
        };

        struct control_block_interface
        {
            control_block_interface() {}
            virtual ~control_block_interface() {}
            virtual void increase() = 0;
            virtual void decrease() = 0;
            virtual long use_count() = 0;
            virtual void release() = 0;
        };

        template <typename T>
        struct control_block : public control_block_interface
        {
            control_block()
                : data(NULLPTR)
                , shared_count(0)
                , deletor(NULLPTR)
                {};

            control_block(T *_ptr)
                : data(_ptr),
                  shared_count(0),
                  deletor(NULLPTR)
            {
            };

            template <typename D>
            control_block(T *_ptr, D _d)
                : data(_ptr),
                  shared_count(0),
                  deletor(NULLPTR)
            {
                struct Deletor : deletor_interface
                {
                    Deletor(D _d) : d(_d) {}
                    ~Deletor() {}
                    void operator()(void *_ptr) OVERRIDE
                    {
                        d(_ptr);
                    }
                    D d;
                };
                deletor = new Deletor(_d);
            };

            ~control_block()
            {
                // if (data != NULLPTR && deletor != NULLPTR)
                // {
                //     (*deletor)(data);
                // }
                // delete_ptr(data);
            }

            void increase() OVERRIDE
            {
                shared_count++;
            }

            void decrease() OVERRIDE
            {
                if (shared_count > 0)
                {
                    shared_count--;
                }
            }

            void release() OVERRIDE
            {
                if (data != NULLPTR && deletor != NULLPTR)
                {
                    (*deletor)(data);
                }
                else
                {
                    delete_ptr(data);
                }
            }

            // template<typename T>
            // T* get()
            // {
            //     return (T)data;
            // }

            long use_count() OVERRIDE
            {
                return shared_count;
            }

            T* data;
            int shared_count;
            deletor_interface* deletor;
        };
    }

    template <typename T>
    class shared_ptr
    {
    public:
        typedef T element_type;

        shared_ptr()
            : m_value(NULLPTR),
              m_block(NULLPTR) NOEXCEPT{};

        template <typename Y>
        explicit shared_ptr(Y *_ptr)
            : m_value((T*)_ptr),
              m_block(new detail::control_block<Y>(_ptr))
        {
            m_block->increase();
            DBG(m_block->use_count());
        }

        shared_ptr(const shared_ptr &r) NOEXCEPT
            : m_value(r.m_value),
              m_block(r.m_block)
        {
            m_block->increase();
            DBG(m_block->use_count());
        }

        template <typename Y>
        shared_ptr(const shared_ptr<Y> &r) NOEXCEPT
            : m_value(r.m_value),
              m_block(r.m_block)
        {
            m_block->increase();
            DBG(m_block->use_count());
        }

        ~shared_ptr()
        {
            if (m_block != NULLPTR)
            {
                // DBG(m_block->use_count());
                // if (m_block->use_count() == 0)
                // {
                //     return;
                // }
                m_block->decrease();
                DBG(m_block->use_count());
                if (m_block->use_count() == 0)
                {
                    m_block->release();
                    delete_ptr(m_block);
                }
            }
        }

        shared_ptr &operator=(const shared_ptr &r) NOEXCEPT
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

        element_type *get() const NOEXCEPT
        {
            return m_value;
        }

        T &operator*() const NOEXCEPT
        {
            return *m_value;
        }

        T *operator->() const NOEXCEPT
        {
            return m_value;
        }

        long use_count() const NOEXCEPT
        {
            return m_block->use_count();
        }

    // private:
        T* m_value;
        detail::control_block_interface* m_block;
    };

    template <typename T, typename U>
    inline shared_ptr<T> dynamic_pointer_cast(const shared_ptr<U> &r) NOEXCEPT
    {
        shared_ptr<T> s;
        s.m_value = dynamic_cast<T*>(r.m_value);
        s.m_block = r.m_block;
        s.m_block->increase();
        return s;
    }
}
