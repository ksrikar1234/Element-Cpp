#ifndef _GP_SHARED_HPP_
#define _GP_SHARED_HPP_

#include <memory>

namespace gp_std
{
    template <typename T>
    class shared;

    template <typename T>
    class shared
    {
    public:
        shared() : m_ptr(nullptr) {}

        template <typename... Args>
        shared(Args &&...args)
        {
            emplace(std::forward<Args>(args)...);
        }

        shared(const shared& other)
        {
            m_ptr = other.m_ptr;
        }

        shared(shared&& other)
        {
            m_ptr = std::move(other.m_ptr);
        }

        shared& operator=(shared&& other)
        {
            m_ptr = std::move(other.m_ptr);
            return *this;
        }

        shared& operator=(const shared& other)
        {
            m_ptr = other.m_ptr;
            return *this;
        }

        shared& operator=(const T& value)
        {
            emplace(value);
            return *this;
        }

        shared& operator=(T&& value)
        {
            emplace(std::move(value));
            return *this;
        }

        template <typename... Args>
        void emplace(Args&&... args)
        {
            if(m_ptr)
            {
                new(m_ptr.get()) T(std::forward<Args>(args)...);
            }
            m_ptr = std::make_shared<T>(std::forward<Args>(args)...);
        }

        template <typename... Args>
        shared& store(Args &&...args)
        {
            emplace(std::forward<Args>(args)...);
        }

        template <typename... Args>
        shared& detach_and_store(Args &&...args)
        {
            m_ptr = std::make_shared<T>(std::forward<Args>(args)...);
        }

        // Use Count
        size_t use_count() const
        {
            return m_ptr.use_count();
        }

        T* operator->()
        {
            assert(m_ptr != nullptr && "gp_std::shared : Operator -> called on nullptr");
            return m_ptr.get();
        }

        const T* operator->() const
        {
            assert(m_ptr != nullptr && "gp_std::shared : Operator -> called on nullptr");
            return m_ptr.get();
        }

        T& operator*()
        {
            assert(m_ptr != nullptr && "gp_std::shared : Operator * called on nullptr");
            return *m_ptr;
        }

        const T& operator*() const
        {
            assert(m_ptr != nullptr && "gp_std::shared : Operator * called on nullptr");
            return *m_ptr;
        }

        operator T&()
        {
            assert(m_ptr != nullptr && "gp_std::shared : Operator T& called on nullptr");
            return *m_ptr;
        }

        operator const T&() const
        {
            assert(m_ptr != nullptr && "gp_std::shared : Operator const T& called on nullptr");
            return *m_ptr;
        }
        
        operator bool()
        {
            return m_ptr != nullptr;
        }

        bool operator==(const shared &other)
        {
            return m_ptr == other.m_ptr;
        }

        bool operator!=(const shared &other)
        {
            return !(*this == other);
        }

        bool operator==(const void *other)
        {
            return static_cast<void *>(m_ptr.get()) == other;
        }

    private:
        std::shared_ptr<T> m_ptr;
    };
}

#endif
