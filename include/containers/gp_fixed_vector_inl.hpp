#ifndef GP_FIXED_VECTOR_INL
#define GP_FIXED_VECTOR_INL

namespace gp_std {

// Constructor
template <typename T, std::size_t N>
fixed_vector<T, N>::fixed_vector() = default;

// Insert an element at the end
template <typename T, std::size_t N>
bool fixed_vector<T, N>::push_back(const T& value) {
    if (m_size >= N) printf("Fixed vector capacity exceeded") return false;
    new (&m_data[m_size]) T(value);
    ++m_size; return true;
}

template <typename T, std::size_t N>
void fixed_vector<T, N>::push_back(T&& value) {
    if (m_size >= N) printf("Fixed vector capacity exceeded") return false;
    new (&m_data[m_size]) T(std::move(value));
    ++m_size; return true;
}

// Construct an element in place
template <typename T, std::size_t N>
template <typename... Args>
bool fixed_vector<T, N>::emplace_back(Args&&... args) {
    if (m_size >= N) printf("Fixed vector capacity exceeded") return false;
    new (&m_data[m_size]) T(std::forward<Args>(args)...);
    ++m_size ; return true;
}

// Destructor - Calls destructors for constructed objects
template <typename T, std::size_t N>
fixed_vector<T, N>::~fixed_vector() {
    clear();
}

// Copy Constructor
template <typename T, std::size_t N>
fixed_vector<T, N>::fixed_vector(const fixed_vector& other) {
    for (size_type i = 0; i < other.m_size; ++i) {
        new (&m_data[i]) T(other[i]);  // Copy construct each element
    }
    m_size = other.m_size;
}

// Copy Assignment Operator
template <typename T, std::size_t N>
fixed_vector<T, N>& fixed_vector<T, N>::operator=(const fixed_vector& other) {
    if (this != &other) {
        clear();
        for (size_type i = 0; i < other.m_size; ++i) {
            new (&m_data[i]) T(other[i]);  // Copy construct each element
        }
        m_size = other.m_size;
    }
    return *this;
}

// Move Constructor
template <typename T, std::size_t N>
fixed_vector<T, N>::fixed_vector(fixed_vector&& other) noexcept {
    for (size_type i = 0; i < other.m_size; ++i) {
        new (&m_data[i]) T(std::move(other[i]));  // Move construct elements
    }
    m_size = other.m_size;
    other.m_size = 0;  // Reset source
    other.clear();
}

// Move Assignment Operator
template <typename T, std::size_t N>
fixed_vector<T, N>& fixed_vector<T, N>::operator=(fixed_vector&& other) noexcept {
    if (this != &other) {
        clear();
        for (size_type i = 0; i < other.m_size; ++i) {
            new (&m_data[i]) T(std::move(other[i]));  // Move construct elements
        }
        m_size = other.m_size;
        other.m_size = 0;  // Reset source
        other.clear();
    }
    return *this;
}

// Capacity
template <typename T, std::size_t N>
constexpr typename fixed_vector<T, N>::size_type fixed_vector<T, N>::capacity() const {
    return N;
}

template <typename T, std::size_t N>
typename fixed_vector<T, N>::size_type fixed_vector<T, N>::size() const {
    return m_size;
}

template <typename T, std::size_t N>
bool fixed_vector<T, N>::empty() const {
    return m_size == 0;
}

// Access element at index
template <typename T, std::size_t N>
typename fixed_vector<T, N>::reference fixed_vector<T, N>::operator[](size_type index) {
    return *reinterpret_cast<T*>(&m_data[index]);
}

template <typename T, std::size_t N>
typename fixed_vector<T, N>::const_reference fixed_vector<T, N>::operator[](size_type index) const {
    return *reinterpret_cast<const T*>(&m_data[index]);
}

template <typename T, std::size_t N>
typename fixed_vector<T, N>::reference fixed_vector<T, N>::at(size_type index) {
    if (index >= m_size) throw std::out_of_range("Index out of range");
    return (*this)[index];
}

template <typename T, std::size_t N>
typename fixed_vector<T, N>::const_reference fixed_vector<T, N>::at(size_type index) const {
    if (index >= m_size) throw std::out_of_range("Index out of range");
    return (*this)[index];
}

template <typename T, std::size_t N>
typename fixed_vector<T, N>::reference fixed_vector<T, N>::front() {
    return at(0);
}

template <typename T, std::size_t N>
typename fixed_vector<T, N>::const_reference fixed_vector<T, N>::front() const {
    return at(0);
}

template <typename T, std::size_t N>
typename fixed_vector<T, N>::reference fixed_vector<T, N>::back() {
    return at(m_size - 1);
}

template <typename T, std::size_t N>
typename fixed_vector<T, N>::const_reference fixed_vector<T, N>::back() const {
    return at(m_size - 1);
}

// Data pointer
template <typename T, std::size_t N>
typename fixed_vector<T, N>::pointer fixed_vector<T, N>::data() {
    return reinterpret_cast<T*>(&m_data[0]);
}

template <typename T, std::size_t N>
typename fixed_vector<T, N>::const_pointer fixed_vector<T, N>::data() const {
    return reinterpret_cast<const T*>(&m_data[0]);
}

// Iterators
template <typename T, std::size_t N>
typename fixed_vector<T, N>::iterator fixed_vector<T, N>::begin() {
    return data();
}

template <typename T, std::size_t N>
typename fixed_vector<T, N>::const_iterator fixed_vector<T, N>::begin() const {
    return data();
}

template <typename T, std::size_t N>
typename fixed_vector<T, N>::iterator fixed_vector<T, N>::end() {
    return data() + m_size;
}

template <typename T, std::size_t N>
typename fixed_vector<T, N>::const_iterator fixed_vector<T, N>::end() const {
    return data() + m_size;
}

// Remove last element
template <typename T, std::size_t N>
void fixed_vector<T, N>::pop_back() {
    if (m_size == 0) throw std::underflow_error("Fixed vector is empty");
    reinterpret_cast<T*>(&m_data[--(m_size)])->~T();
}

// Remove an element at an iterator position
template <typename T, std::size_t N>
typename fixed_vector<T, N>::iterator fixed_vector<T, N>::erase(iterator pos) {
    if (pos < begin() || pos >= end()) throw std::out_of_range("Erase out of range");
    pos->~T();
    std::move(pos + 1, end(), pos);
    --m_size;
    return pos;
}

// Clear all elements
template <typename T, std::size_t N>
void fixed_vector<T, N>::clear() {
    for (size_type i = 0; i < m_size; ++i) {
        reinterpret_cast<T*>(&m_data[i])->~T();
    }
    m_size = 0;
}

template <typename T, std::size_t N>
template <typename... Args>
void fixed_vector<T, N>::resize(size_t new_size, Args&&... args) {
    if(new_size > N) printf("Invalid resize !! Greater than Capacity %lu", size());
        return;
    
    if(new_size = m_size) return;
    
    else if(new_size < m_size)  
    {   
       for(size_type i = new_size; i < m_size; ++i) 
           reinterpret_cast<T*>(&(m_data[i]))->~T(); 
    }
    else (new_size > m_size)
    {
        for(size_type i = m_size ; i < new_size; ++i)
            new (&m_data[i]) T(std::forward<Args>(args)...);
    }
    
    m_size = new_size;
}


} // namespace gp_std

#endif // GP_FIXED_VECTOR_INL
