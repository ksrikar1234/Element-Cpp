#ifndef _GP_INTRUSIVE_HPP
#define _GP_INTRUSIVE_HPP

#include <utility>     // For std::forward
#include <new>         // For placement new
#include <cassert>     // For assert
#include <type_traits> // For std::is_base_of

namespace gp_std
{
    template <typename T>
    class intrusive_ptr; // Forward declaration

    // Base class providing intrusive reference counting
    class ref_counted
    {
    private:
        template <typename T>
        friend class intrusive_ptr;
        // Intrusive reference counting
        void ___add_ref() noexcept
        {
            ++ref_count_;
        }

        void ___release() noexcept
        {
            --ref_count_;
            if (ref_count_ == 0)
            {
                delete this;
            }
        }

        void ___init_ref_count() noexcept
        {
            ref_count_ = 1;
        }

        int intrusive_ref_count() const noexcept
        {
            return ref_count_;
        }

    protected:
        ref_counted() : ref_count_(-1) {}
        virtual ~ref_counted() {}

    private:
        int ref_count_;
    };

    // Templated intrusive pointer for managing ref_counted-derived objects
    template <typename T>
    class intrusive_ptr
    {
    public:
        static_assert(std::is_base_of<ref_counted, T>::value, "T must be derived from ref_counted");
        /// @brief Constructs an intrusive_ptr with a nullptr
        intrusive_ptr() noexcept : ptr_(nullptr) {}

        /// @brief Constructs an intrusive_ptr with a nullptr
        intrusive_ptr(T* heap_allocated_resource) noexcept : ptr_(heap_allocated_resource)
        {
            if (ptr_)
            {
                ptr_->___init_ref_count();
            }
        }

        intrusive_ptr(const intrusive_ptr& other) noexcept : ptr_(other.ptr_)
        {
            if (ptr_)
            {
                ptr_->___add_ref();
            }
        }

        intrusive_ptr(intrusive_ptr&& other) noexcept : ptr_(other.ptr_)
        {
            other.ptr_ = nullptr;
        }

        intrusive_ptr &operator=(const intrusive_ptr& other) noexcept
        {
            if (this != &other)
            {
                if (ptr_)
                {
                    ptr_->___release();
                }

                ptr_ = other.ptr_;

                if (ptr_)
                {
                    ptr_->___add_ref();
                }
            }
            return *this;
        }

        intrusive_ptr &operator=(intrusive_ptr&& other) noexcept
        {
            if (this != &other)
            {
                if (ptr_)
                {
                    ptr_->___release();
                }
                ptr_ = other.ptr_;
                other.ptr_ = nullptr;
            }
            return *this;
        }

        ~intrusive_ptr()
        {
            if (ptr_)
            {
                ptr_->___release();
            }
        }

        T* get() const noexcept
        {
            return ptr_;
        }

        T& operator*() const noexcept
        {
            return *ptr_;
        }

        T* operator->() const noexcept
        {
            return ptr_;
        }

        void reset(T *heap_allocated_resource = nullptr) noexcept
        {
            if (ptr_)
            {
                ptr_->___release();
            }

            ptr_ = heap_allocated_resource;

            if (ptr_)
            {
                ptr_->___init_ref_count();
            }
        }

        void swap(intrusive_ptr &other) noexcept
        {
            T* temp = ptr_;
            ptr_ = other.ptr_;
            other.ptr_ = temp;
        }

        explicit operator bool() const noexcept
        {
            return ptr_ != nullptr;
        }

    private:
        T* ptr_;
    };
}
#endif // _GP_INTRUSIVE_HPP
