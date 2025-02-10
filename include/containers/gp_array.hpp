#ifndef _GP_STD_ARRAY_HPP_
#define _GP_STD_ARRAY_HPP_
#include <initializer_list>
#include <algorithm>
#include <stdexcept>

namespace gp_std
{
    template <typename T, std::size_t N>
    class array
    {
    private:
        T m_data[N]; // Fixed-size internal storage

    public:
        // Type definitions
        typedef T value_type;
        typedef std::size_t size_type;
        typedef T& reference;
        typedef const T &const_reference;
        typedef T* pointer;
        typedef const T* const_pointer;
        typedef T* iterator;
        typedef const T* const_iterator;

        // Default constructor (value-initializes elements)
        array() : m_data{} {}

        // Initializer list constructor (C++11)
        array(std::initializer_list<T> init)
        {
            std::size_t i = 0;
            for (auto it = init.begin(); it != init.end() && i < N; ++it, ++i)
            {
                m_data[i] = *it;
            }
            for (; i < N; ++i)
            { // Zero-initialize remaining elements
                m_data[i] = T();
            }
        }

        array(const array<T, N>& other)
        {
            for (size_t i = 0; i < N; ++i)
            { (*this)[i] = other[i]; }
        }

        array(array<T, N>&& other)
        {
            for (size_t i = 0; i < N; ++i)
            { (*this)[i] = std::move(other[i]);}
        }

        // Element access
        reference operator[](size_type index) { return m_data[index]; }
        const_reference operator[](size_type index) const { return m_data[index]; }

        reference at(size_type index)
        {
            if (index >= N)
                throw std::out_of_range("Index out of range");
            return m_data[index];
        }

        const_reference at(size_type index) const
        {
            if (index >= N)
                throw std::out_of_range("Index out of range");
            return m_data[index];
        }

        array<T, N>& operator=(const array<T, N>& other)
        {
            if(this != &other)
            {
                for(size_t i = 0; i < N; ++i)
                {
                    (*this)[i] = other[i];
                }
            }
            return *this;
        }

        array<T, N>& operator=(array<T, N>&& other)
        {
            if(this != &other)
            {
                for(size_t i = 0; i < N; ++i)
                {
                    (*this)[i] = std::move(other[i]);
                }
            }
            return *this;
        }

        reference front() { return m_data[0]; }
        const_reference front() const { return m_data[0]; }

        reference back() { return m_data[N - 1]; }
        const_reference back() const { return m_data[N - 1]; }

        pointer data() { return m_data; }
        const_pointer data() const { return m_data; }

        // Iterators
        iterator begin() { return m_data; }
        const_iterator begin() const { return m_data; }
        iterator end() { return m_data + N; }
        const_iterator end() const { return m_data + N; }

        // Capacity
        constexpr size_type size() const { return N; }
        constexpr bool empty() const { return N == 0; }

        // Modifiers
        void fill(const T &value)
        {
            std::fill(begin(), end(), value);
        }

        void swap(array &other) noexcept
        {
            std::swap_ranges(begin(), end(), other.begin());
        }

        // Comparison operators (C++11 compatible)
        bool operator==(const array &other) const
        {
            return std::equal(begin(), end(), other.begin());
        }

        bool operator!=(const array &other) const
        {
            return !(*this == other);
        }

        bool operator<(const array &other) const
        {
            return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
        }

        bool operator>(const array& other) const
        {
            return other < *this;
        }

        bool operator<=(const array& other) const
        {
            return !(other < *this);
        }

        bool operator>=(const array& other) const
        {
            return !(*this < other);
        }
    };

} // namespace gp_std
#endif
