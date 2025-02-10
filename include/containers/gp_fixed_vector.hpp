#ifndef GP_FIXED_VECTOR_H
#define GP_FIXED_VECTOR_H

#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <iterator>
#include <utility>

namespace gp_std {

template <typename T, std::size_t N>
class fixed_vector {
private:
    typename std::aligned_storage<sizeof(T), alignof(T)>::type m_data[N];
    std::size_t m_size = 0;

public:
    using value_type = T;
    using size_type = std::size_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = T*;
    using const_iterator = const T*;

    fixed_vector();
    ~fixed_vector();

    fixed_vector(const fixed_vector&) = delete;
    fixed_vector& operator=(const fixed_vector&) = delete;

    bool push_back(const T& value);
    bool push_back(T&& value);
    
    template <typename... Args>
    bool emplace_back(Args&&... args);

    constexpr size_type capacity() const;
    size_type size() const;
    bool empty() const;

    reference operator[](size_type index);
    const_reference operator[](size_type index) const;
    reference at(size_type index);
    const_reference at(size_type index) const;

    reference front();
    const_reference front() const;
    reference back();
    const_reference back() const;

    pointer data();
    const_pointer data() const;

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;



    void pop_back();
    iterator erase(iterator pos);
    iterator erase(iterator first, iterator last);
    void clear();
};

} // namespace gp_std

#include "gp_fixed_vector_inl.hpp"  // Inline definitions

#endif // GP_FIXED_VECTOR_H
