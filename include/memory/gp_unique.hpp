#ifndef _GP_STD_RUST_BOX_HPP_
#define _GP_STD_RUST_BOX_HPP_

#include <memory>
#include <cassert>

namespace gp_std
{
    // Copies the object when a non const method to acess the object is called
    #define GP_FAIL(msg) assert(false && msg)

    template <typename T>
    class unique;

    template <typename T>
    class resource_creation_functor;

    class TypeErasedFunction
    {
        public:
        using DeleterFunc = void (*)(void *);

        explicit TypeErasedFunction(DeleterFunc func = nullptr) : deleter(func) 
        {}
        
        void operator()(void *ptr) const
        {
            if (deleter)
            {
                deleter(ptr);
            }
            else
            {
                GP_FAIL("gp_std::unique :: TypeErasedDeleter function not set");
            }
        }

    private:
        DeleterFunc deleter;
    };

    class TypeErasedDeleter : public TypeErasedFunction
    {
       public:
       explicit TypeErasedDeleter(DeleterFunc func = nullptr) : TypeErasedFunction(func)
       {}
    };

    class TypeErasedDestructor : public TypeErasedFunction
    {
       public:
       explicit TypeErasedDestructor(DeleterFunc func = nullptr) : TypeErasedFunction(func)
       {}
    };

    template <typename T>
    class unique_ptr_handle
    {
    public:
        unique_ptr_handle& operator=(unique_ptr_handle<T> &&other)
        {
            if (this != &other)
            {
                m_ptr = std::move(other.m_ptr);
            }
            return *this;
        } 

        ~unique_ptr_handle()
        {}

        T* get() const
        {
            return m_ptr.get();
        }

        T* operator->() const
        {
            return m_ptr.get();
        }
        
        T& operator*()
        {
            return *m_ptr;
        }
     
        const T& operator*() const
        {
            return *m_ptr;
        }

        bool operator==(const unique_ptr_handle<T> &other) const
        {
            return m_ptr == other.m_ptr;
        }

        bool operator==(std::nullptr_t) const
        {
            return m_ptr == nullptr;
        }

        bool operator!=(const unique_ptr_handle<T> &other) const
        {
            return m_ptr != other.m_ptr;
        }

        bool operator!=(std::nullptr_t) const
        {
            return m_ptr != nullptr;
        }

        operator bool() const
        {
            return m_ptr != nullptr;
        }

    private:
        friend class unique<T>;
        friend class resource_creation_functor<T>;
        unique_ptr_handle(T* ptr, TypeErasedDeleter deleter)
            : m_ptr(ptr, deleter)
        {
            if(ptr == nullptr)
            {
                GP_FAIL("gp_std::unique_ptr_handle :: Cannot assign nullptr to unique_ptr_handle");
            }
        }

        unique_ptr_handle()
            : m_ptr(nullptr, TypeErasedDeleter())
        {
        }

        unique_ptr_handle(unique_ptr_handle<T> &&other)
            : m_ptr(std::move(other.m_ptr))
        {
        }

        unique_ptr_handle(const unique_ptr_handle<T> &other) = delete;
        unique_ptr_handle &operator=(const unique_ptr_handle<T> &other) = delete;

        std::unique_ptr<T, TypeErasedDeleter> m_ptr;
    };

    // A unique_resource_handle typedef for type-erased unique_ptr
    template <typename T>
    using unique_resource_handle = unique_ptr_handle<T>;

    template <typename T>
    inline TypeErasedDeleter makeTypeErasedDeleter()
    {
        return TypeErasedDeleter([](void* ptr)
                                 { delete static_cast<T*>(ptr); });
    }

    template <typename T>
    inline TypeErasedDestructor makeTypeErasedDestructor()
    {
        return TypeErasedDestructor([](void* ptr)
                                     { static_cast<T*>(ptr)->~T(); });
    }

    template <typename T>
    class resource_creation_functor
    {
        public:
        template<typename... Args>
        static unique_resource_handle<T> create(Args&&... args)
        {
            return unique_resource_handle<T>
            (
                new T(std::forward<Args>(args)...),
                makeTypeErasedDeleter<T>()
            );
        }
    };


    template <typename T, typename... Args>
    inline unique_resource_handle<T> create_unique_resource(Args &&...args)
    {    
        return resource_creation_functor<T>::create(std::forward<Args>(args)...);
    }

    /// @brief A Rust Box like resource handler class with type-erased deleter
    /// @note  Primary use case is to handle resources that are incomplete but need stack variables like behaviour
    /// @note  It get rids of the manual allocation and deallocation of resources
    /// @note  We cannot set it to nullptr (nullptr is impossible to assign to a unique resource)
    /// @note  This Guarentees that the resource is always valid & no bad dereferences occur
    /// @note  We can use this for incomplete types
    /// @note  We can clone the resource
    /// @note  We can check if resource exists (it will always be true because of using Rust Box like creation & assignment restrictions)
    /// @note  We can construct from other unique it will clone the resource
    /// @note  We can emplace a resource
    /// @note  We can get rid of the nullptr state by using emplace
    /// @note  We cannot move construct or assign since that leaves the source in a nullptr state
    /// @note  We can swap two unique resources
    /// @note  We can access the resource using ->, *, value() and operator T&()
    template <typename T>
    class unique // Not a ptr but a Dedicated resource handler class
    {
    public:
        ~unique() {}
         
        unique() = default;

        template <typename... Args>
        explicit unique(Args &&...args);

        unique(unique_resource_handle<T> other)
            : resource_(std::move(other))
        {}  
       
        // Construct resource inplace
        template <typename... Args>
        void emplace(Args &&...args);

        unique& operator=(unique_resource_handle<T> other);

        unique& operator=(const T& other);

        unique& operator=(T&& other);

        T* operator->() const;

        operator T& ();

        const T& operator*() const;

        T& operator*();

        T& value();

        const T& value() const;

        // Check if resource exists
        bool has_value() const
        {
            return resource_ != nullptr;
        }
        
        operator bool() const
        {
            return has_value();
        }

        void swap(unique& other)
        {
            if (this == &other) return;

            if (!has_value())
            {
                GP_FAIL("Cannot swap !! Self is Not initialized");
            }

            if (!other.has_value())
            {
                GP_FAIL("Cannot swap with another uninitialized unique");
            }

            resource_.swap(other.resource_);
        }

        unique_resource_handle<T> clone()  const
        {
            return (create_unique_resource<T>(*resource_));
        }

    private:
        // Access the resource
        T* get() const 
        {
            return resource_.get();
        }

        // Disallow copying
        unique(const unique& other) = delete; 
        unique& operator=(const unique& other) = delete;

        // Move constructor and move assignment
        unique(unique &&) noexcept;
        unique &operator=(unique &&) noexcept = default;

    private:
        unique_resource_handle<T> resource_; // The internal unique_resource_handle that handles resource
    };

    template <typename T>
    template <typename... Args>
    inline unique<T>::unique(Args &&...args)
    {
        emplace(std::forward<Args>(args)...);
    }

    // Create resource via make_unique
    template <typename T>
    template <typename... Args>
    inline void unique<T>::emplace(Args &&...args)
    {
        if (resource_)
        {
            // In-place destroy and construct
            static_cast<T *>(resource_.get())->~T();
            new (resource_.get()) T(std::forward<Args>(args)...);
            return;
        }
        else
            resource_ = create_unique_resource<T>(std::forward<Args>(args)...);
    }
    template <typename T>
    inline unique<T>& unique<T>::operator=(unique_resource_handle<T> other)
    {
        if (other)
        {
            resource_ = std::move(other);
        }
        else
        {
            GP_FAIL("Cannot assign to a nullptr");
        }
        return *this;
    }

    template <typename T>
    inline unique<T>& unique<T>::operator=(const T &other)
    {
        if (has_value())
        {
            *resource_ = other;
        }
        else
        {
            GP_FAIL("Cannot assign to a nullptr");
        }
        return *this;
    }

    template <typename T>
    inline unique<T>& unique<T>::operator=(T &&other)
    {
        if (has_value())
        {
            *resource_ = std::move(other);
        }
        else
        {
            GP_FAIL("Cannot assign to a nullptr");
        }
        return *this;
    }

    template <typename T>
    inline T *unique<T>::operator->() const
    {
        if (!has_value())
        {
            GP_FAIL("unique::operator-> called on nullptr");
        }
        return get();
    }

    template <typename T>
    inline unique<T>::operator T&()
    {
        if (!has_value())
        {
            GP_FAIL("unique::operator T& called on nullptr");
        }
        return *get();
    }

    template <typename T>
    inline const T& unique<T>::operator*() const
    {
        if (!has_value())
        {
            GP_FAIL("unique::operator* called on nullptr");
        }
        return *get();
    }

    template <typename T>
    inline T& unique<T>::operator*()
    {
        if (!has_value())
        {
            GP_FAIL("unique::operator* called on nullptr");
        }
        return *get();
    }

    template <typename T>
    inline T& unique<T>::value()
    {
        if (!has_value())
        {
            GP_FAIL("unique::value called on nullptr");
        }
        return *get();
    }

    template <typename T>
    inline const T& unique<T>::value() const
    {
        if (!has_value())
        {
            GP_FAIL("unique::value called on nullptr");
        }
        return *get();
    }


} // namespace gp_std

#endif // _GP_STD_RUST_BOX_HPP_
