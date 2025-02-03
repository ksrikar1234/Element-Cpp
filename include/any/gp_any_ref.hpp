#ifndef _GP_ANY_REF_
#define _GP_ANY_REF_

#include <type_traits>
#include <typeinfo>
#include <cassert>

namespace gp_std
{
    /**
     * @class any_ptr
     * @brief A lightweight type-erased pointer wrapper.
     *
     * This class is a type-erased pointer wrapper that allows storing pointers of any type
     * while keeping track of the type information via a hash code. It provides a way to
     * store and transfer pointers to objects of different types, with type safety checks
     * based on the stored type hash code.
     *
     * The purpose of this class is to provide a generic pointer-like behavior while
     * maintaining type information for runtime checks. This can be useful for scenarios
     * where you need to store or pass around pointers of different types in a unified way.
     *
     * @note Example Demo:
     * ```cpp
     * int x = 10;
     * any_ptr ptr1(&x);
     * 
     * int& x = ptr1;
     * double y = 3.14;
     * any_ptr ptr2(&y);
     *
     * // The pointers can hold different types and are type-safe.
     * ```
     * @warning Use this only for passing referencing Objects 
     * @warning Doesnt Handle Heap Allocated Objects well
     * @warning Use gp_std::any instead
     */
    class any_ptr
    {
    public:
        any_ptr() : ref_(nullptr), type_hash_code(0) {}

        template <typename T>
        any_ptr(T *ref) : ref_(ref), type_hash_code(typeid(T).hash_code()) {}

        any_ptr(const any_ptr &other) : ref_(other.ref_), type_hash_code(other.type_hash_code) {}

        any_ptr &operator=(const any_ptr &other)
        {
            ref_ = other.ref_;
            type_hash_code = other.type_hash_code;
            return *this;
        }

        any_ptr& operator=(std::nullptr_t)
        {
            ref_ = nullptr;
            type_hash_code = 0;
            return *this;
        }

        template <typename T>
        any_ptr& operator=(T* ref)
        {
            ref_ = ref;
            type_hash_code = typeid(T).hash_code();
            return *this;
        }

        template <typename T>
        T* operator->() const noexcept
        {
            return recover<T>();
        }
        
        template<typename T>
        operator T&()
        {
            return value<T>();
        }

        template<typename T>
        operator const T&() const
        {
            return value<T>();
        }

        template <typename T>
        T& value()
        {   
            T* recovered_ptr = recover<T>();
            if(recovered_ptr)
                return *recovered_ptr;
            else
                assert(recovered_ptr != nullptr && "Dereferencing a unsucessfull recovery in any_ptr !! For Fixing, Use pointer recovery and check if its a null dynamic in ur code");
            
        }

        template <typename T>
        const T& value() const 
        {   
            T* recovered_ptr = recover<T>();
            if(recovered_ptr)
                return *recovered_ptr;
            else
                assert(recovered_ptr != nullptr && "Dereferencing a unsucessfull recovery in any_ptr !! For Fixing, Use pointer recovery and check if its a null dynamic in ur code");
            
        }

        bool operator==(const any_ptr &other) const
        {
            return type_hash_code == other.type_hash_code && ref_ == other.ref_;
        }

        bool operator!=(const any_ptr &other) const
        {
            return !(*this == other);
        }

        bool operator>(const any_ptr &other) const
        {
            return ref_ > other.ref_;
        }

        bool operator<(const any_ptr &other) const
        {
            return ref_ < other.ref_;
        }

        bool operator>=(const any_ptr &other) const
        {
            return ref_ >= other.ref_;
        }

        bool operator<=(const any_ptr &other) const
        {
            return ref_ <= other.ref_;
        }

        template <typename T>
        T* recover() const noexcept
        {
            if (type_hash_code == typeid(T).hash_code())
                return static_cast<T *>(ref_);
            return nullptr;
        }

        operator bool() const noexcept
        {
            return ref_ != nullptr || type_hash_code != 0;
        }

        void* get() const noexcept
        {
            return ref_;
        }

        template <typename T>
        void check() const
        {
            if (type_hash_code != typeid(T).hash_code())
                throw std::bad_cast();
        }

    private:
        mutable void* ref_;
        unsigned long int type_hash_code;
    };
}

#endif
