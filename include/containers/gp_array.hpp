#ifndef _GP_STD_ARRAY_HPP_
#define _GP_STD_ARRAY_HPP_
#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#pragma once

#include <cstddef>
#include <new>
#include <utility>
#include <type_traits>
#include <algorithm>
#include <stdexcept>

namespace gp_std
{
    template <typename T, std::size_t N>
    class array
    {
    private:
        typename std::aligned_storage<sizeof(T), alignof(T)>::type m_data[N];
    
        T* ptr(std::size_t index) noexcept { return reinterpret_cast<T*>(&m_data[index]); }
        const T* ptr(std::size_t index) const noexcept { return reinterpret_cast<const T*>(&m_data[index]); }
    
    public:
        using value_type = T;
        using size_type = std::size_t;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;
        using iterator = T*;
        using const_iterator = const T*;
    
        // Default Constructor (only if T is default constructible)
        array() requires std::is_default_constructible<T>::value
        {
            for (size_type i = 0; i < N; ++i)
                new (&m_data[i]) T();
        }
    
        // Variadic Constructor: Perfect Forward to all elements
        template <typename... Args>
        array(Args&&... args)
        {
            for (size_type i = 0; i < N; ++i)
                new (&m_data[i]) T(std::forward<Args>(args)...);
        }
    
        // Initializer list constructor
        array(std::initializer_list<T> init)
        {
            size_type i = 0;
            for (auto& elem : init)
                new (&m_data[i++]) T(elem);
    
            for (; i < N; ++i)
                new (&m_data[i]) T();
        }
    
        // Copy Constructor
        array(const array& other)
        {
            for (size_type i = 0; i < N; ++i)
                new (&m_data[i]) T(other[i]);
        }
    
        // Move Constructor
        array(array&& other) noexcept
        {
            for (size_type i = 0; i < N; ++i)
                new (&m_data[i]) T(std::move(other[i]));
        }
    
        ~array()
        {
            for (size_type i = 0; i < N; ++i)
                ptr(i)->~T();
        }
    
        array& operator=(const array& other)
        {
            if (this != &other)
            {
                for (size_type i = 0; i < N; ++i)
                    (*ptr(i)) = other[i];
            }
            return *this;
        }
    
        array& operator=(array&& other) noexcept
        {
            if (this != &other)
            {
                for (size_type i = 0; i < N; ++i)
                    (*ptr(i)) = std::move(other[i]);
            }
            return *this;
        }
    
        reference operator[](size_type index) { return *ptr(index); }
        const_reference operator[](size_type index) const { return *ptr(index); }
    
        reference at(size_type index)
        {
            if (index >= N) throw std::out_of_range("gp_std::array out of range");
            return (*this)[index];
        }
    
        const_reference at(size_type index) const
        {
            if (index >= N) throw std::out_of_range("gp_std::array out of range");
            return (*this)[index];
        }
    
        reference front() { return (*this)[0]; }
        const_reference front() const { return (*this)[0]; }
    
        reference back() { return (*this)[N - 1]; }
        const_reference back() const { return (*this)[N - 1]; }
    
        pointer data() { return ptr(0); }
        const_pointer data() const { return ptr(0); }
    
        iterator begin() { return ptr(0); }
        const_iterator begin() const { return ptr(0); }
    
        iterator end() { return ptr(0) + N; }
        const_iterator end() const { return ptr(0) + N; }
    
        constexpr size_type size() const noexcept { return N; }
        constexpr bool empty() const noexcept { return N == 0; }
    
        void fill(const T& value)
        {
            std::fill(begin(), end(), value);
        }
    
        void swap(array& other) noexcept
        {
            std::swap_ranges(begin(), end(), other.begin());
        }
    
        bool operator==(const array& other) const
        {
            return std::equal(begin(), end(), other.begin());
        }
    
        bool operator!=(const array& other) const { return !(*this == other); }
        bool operator<(const array& other) const { return std::lexicographical_compare(begin(), end(), other.begin(), other.end()); }
        bool operator>(const array& other) const { return other < *this; }
        bool operator<=(const array& other) const { return !(other < *this); }
        bool operator>=(const array& other) const { return !(*this < other); }
    };

} // namespace gp_std

#endif
