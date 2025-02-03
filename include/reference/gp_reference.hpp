#ifndef _GP_STD_REFERENCE_TYPES_
#define _GP_STD_REFERENCE_TYPES_


#include <array>
#include <stdexcept>
#include <cassert>
#include "gp_atomic.hpp"

#define GP_ASSERT(x) assert(x) 


namespace gp_std
{
    template <typename T>
    class ptr;

    template <typename T>
    class array_ptr;

    template <typename T>
    class weak_ref;

    template <typename T, template <typename...> class base_container = ptr>
    class sync_ref;

    template <typename T>
    using atomic_sync_ref = sync_ref<T, gp_std::atomic>;
   

 /// @brief class ptr
    /// @tparam T The type of the data
    /// @note A smart pointer class that can be used to wrap a pointer to an object.
    /// @note Simplest pointer type 
    template <typename T>
    class ptr
    {
    public:
        ptr()       : m_ptr(nullptr) {}
        ptr(T* ptr) : m_ptr(ptr)     {}
        ptr(T& obj) : m_ptr(&obj)    {}
        ptr(const ptr &other) : m_ptr(other.m_ptr) {}
        ptr(ptr &&other) noexcept : m_ptr(other.m_ptr)
        {
            other.m_ptr = nullptr;
        }

        ptr& operator=(const ptr &other)
        {
            if (this != &other)
            {
                m_ptr = other.m_ptr;
            }
            return *this;
        }

        ptr& operator=(ptr &&other) noexcept
        {
            if (this != &other)
            {
                m_ptr = other.m_ptr;
                other.m_ptr = nullptr;
            }
            return *this;
        }

        ptr& operator=(T* ptr)
        {
            m_ptr = ptr;
            return *this;
        }


        bool operator==(const ptr &other) const { return m_ptr == other.m_ptr; }
        bool operator==(T* ptr) const { return m_ptr == ptr;  }
        bool operator==(T& obj) const { return m_ptr == &obj; }

        bool operator!=(const ptr &other) const { return m_ptr != other.m_ptr; }
        bool operator!=(T* ptr) const { return m_ptr != ptr;  }
        bool operator!=(T& obj) const { return m_ptr != &obj; }

        bool operator!() const { return m_ptr == nullptr; }
        operator bool()  const { return m_ptr != nullptr; }

        operator T* () const   { return m_ptr; }

        T& operator*() const 
        {
            GP_ASSERT(m_ptr != nullptr && "Null pointer dereference Detected !!!");
            return *m_ptr;
        }

        T* operator->() const
        {
            GP_ASSERT(m_ptr != nullptr && "Null pointer dereference Detected !!!");
            return m_ptr;
        }

        T* get() const { return m_ptr; }

        void reset(T* ptr = nullptr) { m_ptr = ptr; }

        ~ptr() {}

    private:
        T* m_ptr;
    };


    /// @brief class array_ptr
    /// @tparam T The type of the data
    /// @note A smart pointer class that can be used to wrap a pointer to an array of objects.
    /// @note Similar to array_view in C++20 conveys the intent that the object is a view of an array.
    template <typename T>
    class array_ptr
    {
    public:
        // Constructors
        array_ptr() : m_ptr(nullptr), m_size(0) {}

        array_ptr(T* ptr, const std::size_t& size) : m_ptr(ptr), m_size(size) {}

        array_ptr(const array_ptr& other) : m_ptr(other.m_ptr), m_size(other.m_size) {}

        array_ptr(array_ptr &&other) noexcept : m_ptr(other.m_ptr), m_size(other.m_size)
        {
            other.m_ptr = nullptr;
            other.m_size = 0;
        }

        // Assignment operators
        array_ptr &operator=(const array_ptr &other)
        {
            if (this != &other)
            {
                m_ptr = other.m_ptr;
                m_size = other.m_size;
            }
            return *this;
        }

        array_ptr& operator=(array_ptr &&other) noexcept
        {
            if (this != &other)
            {
                m_ptr = other.m_ptr;
                m_size = other.m_size;
                other.m_ptr = nullptr;
                other.m_size = 0;
            }
            return *this;
        }

        array_ptr& operator=(const T*& ptr)
        {
            m_ptr = ptr;
            return *this;
        }

        // Access operators
        T& operator[](const std::size_t& index) const
        {
            return m_ptr[index];
        }

        // Access Safely
        T& at(const std::size_t& index) const
        {
            GP_ASSERT(m_ptr != nullptr && "Error: Attempt to dereference a null pointer in array_ptr.");
            GP_ASSERT(index < m_size && "Error: Index out of bounds in array_ptr.");
            return m_ptr[index];
        }

        // Comparison operators
        bool operator==(const array_ptr &other) const { return m_ptr == other.m_ptr && m_size == other.m_size; }
        bool operator==(T *ptr) const { return m_ptr == ptr; }
        bool operator!=(const array_ptr &other) const { return m_ptr != other.m_ptr || m_size != other.m_size; }
        bool operator!=(T *ptr) const { return m_ptr != ptr; }

        // Conversion operators
        operator bool() const  { return m_ptr != nullptr; }

        // Pointer access
        T* get() const { return m_ptr; }
        std::size_t size() const { return m_size; }

        void reset(T* ptr = nullptr, std::size_t size = 0)
        {
            m_ptr = ptr;
            m_size = size;
        }

        // Iterator support
        T* begin() const { return m_ptr; }
        T* end()   const { return m_ptr + m_size; }

        // Destructor
        ~array_ptr() {}

    private:
        T* m_ptr;
        std::size_t m_size;
    };


    /// @brief class sync_ref
    /// @tparam T The type of the data
    /// @note A reference wrapper that is always syncronises with other sync_ref objects.
    ///  ***  For example, if a sync_ref object is assigned to a new object, all other sync_ref objects will also point to the new object.
    ///  ***  This is useful when multiple objects need to point to the same object and the object is changed by one of the references.
    ///  ***  float obj1; 
    ///  ***  sync_ref<float> ref1(obj1);
    ///  ***  sync_ref<float> ref2 = ref1;
    ///  ***  ref1 = 10.0f;
    ///  ***  std::cout << *ref2 << std::endl; // Output: 10.0 as ref1 and ref2 are syncronised.
    ///  ***  ref2 = 20.0f;
    ///  ***  std::cout << *ref1 << std::endl; // Output: 20.0 as ref1 and ref2 are syncronised.
    template <typename T, template <typename...> class base_ptr_type>
    class sync_ref
    {
        using ptr_type = base_ptr_type<T>;

        static gp_std::spinlock& get_spinlock()
        {
            static gp_std::spinlock spinlock;
            return spinlock;
        }

        template <typename U>
        static ptr_type* get_sync_ptr()
        {
            static std::array<ptr_type , 128 * 128> sync_ptrs;
            get_spinlock().lock();

            static uint32_t curr_index = 0;
            
            if (curr_index >= sync_ptrs.size())
            {
                get_spinlock().unlock();
                throw std::runtime_error("Out of memory for sync_ptrs. Increase the size of the array.\n");
            }
            
            ptr_type* ptr = &sync_ptrs[curr_index++];
            get_spinlock().unlock();

            return ptr;
        }

    public:
        // Constructor
        sync_ref(T& obj)
        {
           ptr_ = get_sync_ptr<T>();
          *ptr_ = &obj;
        }

        // Default constructor
        sync_ref()
        {
            ptr_ = get_sync_ptr<T>();
           *ptr_ = nullptr;
        }

        // Copy constructor
        sync_ref(const sync_ref &other)
        {
            ptr_ = other.ptr_;
        }

        // Move constructor
        sync_ref(sync_ref &&other) noexcept
        {
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }

        // Copy assignment
        sync_ref &operator=(const sync_ref &other)
        {
            if (this != &other)
            {
                ptr_ = other.ptr_;
            }
            return *this;
        }

        // Move assignment
        sync_ref &operator=(sync_ref &&other) noexcept
        {
            if (this != &other)
            {
                ptr_ = other.ptr_;
                other.ptr_ = nullptr;
            }
            return *this;
        }

        // Nullptr assignment
        sync_ref &operator=(std::nullptr_t)
        {
            *ptr_ = nullptr;
            return *this;
        }

        // Assignment to another object
        sync_ref &operator=(T& obj)
        {
            *ptr_ = &obj;
            return *this;
        }

        bool operator==(const sync_ref &other) const
        {
            return ptr_ == other.ptr_;
        }

        bool operator!=(const sync_ref &other) const
        {
            return ptr_ != other.ptr_;
        }

        bool operator==(std::nullptr_t) const
        {
            return *ptr_ == nullptr;
        }

        bool operator!=(std::nullptr_t) const
        {
            return *ptr_ != nullptr;
        }

        // Dereference operator
        const T& operator*() const
        {
            return **ptr_;
        }

        // Arrow operator
        const ptr_type* operator->() const
        {
            return *ptr_;
        }

        // Get the underlying pointer
        const ptr_type* get() const
        {
            return *ptr_;
        }

        // Non Const Versions
        // Dereference operator
        T& operator*()
        {
            return **ptr_;
        }

        // Arrow operator
        ptr_type operator->() 
        {
            return *ptr_;
        }

        // Get the underlying pointer
        ptr_type get()
        {
            return *ptr_;
        }    

        void retarget(T& new_obj)
        {
            *ptr_ = &new_obj;
        }

        void retarget(T* new_obj)
        {
            *ptr_ = new_obj;
        }

        // Set the underlying pointer to a new value
        void reset(T &new_obj)
        {
            *ptr_ = &new_obj;
        }

        // Check if the sync_ref is valid
        bool valid() const
        {
            return *ptr_ != nullptr;
        }

    private:
        ptr_type* ptr_ = nullptr;
    };

    /// @brief class ref
    /// @tparam T The type of the data
    /// @note A reference wrapper that can be used to wrap a pointer or a reference to an object.
    ///  *** It provides a safe way to access the object by checking if the pointer is null before dereferencing it.
    ///  @warning It is used to imply that there is no guarantee that the pointer will always be pointing to a valid object.
    template <typename T>
    class weak_ref
    {
    public:
        using type = T;
        // Constructors
        weak_ref() : m_ptr(nullptr) {}
        weak_ref(T &obj) : m_ptr(&obj) { check(); }
        weak_ref(const weak_ref &other) : m_ptr(other.m_ptr) { check(); }
        weak_ref(weak_ref &&other) : m_ptr(other.m_ptr)
        {
            other.m_ptr = nullptr;
            check();
        }

        // Assignment operators
        weak_ref& operator=(const weak_ref &other)
        {
            if(this != &other) m_ptr = other.m_ptr;
            check();
            return *this;
        }

        weak_ref &operator=(T &obj)
        {
            m_ptr = &obj;
            check();
            return *this;
        }

        weak_ref &operator=(weak_ref &&other)
        {
            if (this != &other)
            {
                m_ptr = other.m_ptr;
                other.m_ptr = nullptr;
            }
            check();
            return *this;
        }

        // Function Call Operator
        T &operator()() const
        {
            check();
            return *m_ptr;
        }

        // Index Operator
        T &operator[](int index) const
        {
            check();
            return (m_ptr)[index];
        }

        // Comparison Operators
        bool operator==(const weak_ref &other) const { return m_ptr == other.m_ptr; }
        bool operator==(const T &obj) const { return m_ptr == &obj; }
        bool operator!=(const weak_ref &other) const { return m_ptr != other.m_ptr; }
        bool operator!=(const T &obj) const { return m_ptr != &obj; }

        // Logical Operators
        bool operator!() const { return m_ptr == nullptr; }
        operator bool() const  { return m_ptr != nullptr; }

        // Type Conversion Operators
        operator T*&() const
        {
            check();
            return m_ptr;
        }
        operator T&() const
        {
            check();
            return *m_ptr;
        }

        // Dereference Operators
        T& operator*() const
        {
            check();
            return *m_ptr;
        }

        T*& operator->() const
        {
            check();
            return m_ptr;
        }

        // Member Functions
        T* get() const
        {
            check();
            return m_ptr;
        }

        void swap(weak_ref &other)
        {
            T *temp = m_ptr;
            m_ptr = other.m_ptr;
            other.m_ptr = temp;
        }

        void change(T &obj)
        {
            m_ptr = &obj;
            check();
        }
        void change(weak_ref &other)
        {
            m_ptr = other.m_ptr;
            check();
        }

        bool is_null() const { return m_ptr == nullptr; }
        void check() const
        {
            if (m_ptr == nullptr)
                throw std::runtime_error(" Null pointer Reference !!!");
        }

        void reset(T* raw_ptr = nullptr) { m_ptr = raw_ptr; }
        void clear() { m_ptr = nullptr; }

        // Destructor
        ~weak_ref()
        {
            m_ptr = nullptr;
        }

    private:
        T* m_ptr;
    };

}

#endif // GP_STD_REFERENCE
