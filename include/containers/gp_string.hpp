#ifndef GP_STD_STRING_HPP
#define GP_STD_STRING_HPP

#include <array>
#include <set>
#include <iostream>
#include <cstring>
#include <cassert>
#include <string>
#include <deque>

namespace gp_std
{
    // Forward declarations of the exposed classes

    // Class representing a view of non-owning const string data
    class const_string_view;

    // Class representing a view of non-owning mutable string data
    class string_view;

    // Class representing a fixed-size string
    // Fast stack-allocated string
    template <std::size_t N>
    class array_string;

    // Class representing a block allocator backed by std::string like class
    // Fast string class with small string optimization
    class string;

    ///*******************************************************************************************************************
    // * const_string_view
    class const_string_view
    {
    public:
        const_string_view(const char *str, std::size_t len)
            : data_(str), str_len(len) {}

        const_string_view() : data_(nullptr), str_len(0) {}

        const_string_view(const char *str)
            : data_(str), str_len(std::strlen(str)) {}

        const_string_view find(const char *str) const
        {
            const char *found = std::strstr(data_, str);

            if (found)
            {
                return const_string_view(found, std::strlen(str));
            }

            return const_string_view();
        }

        const char *data() const { return data_; }
        std::size_t size() const { return str_len; }

        const char *begin() const { return data_; }
        const char *end() const { return data_ + str_len; }

        char operator[](std::size_t i) const
        {
            return data_[i];
        }

        bool operator==(const const_string_view &other) const
        {
            if (str_len != other.str_len)
            {
                return false;
            }

            for (std::size_t i = 0; i < str_len; ++i)
            {
                if (data_[i] != other.data_[i])
                {
                    return false;
                }
            }
            return true;
        }

        operator bool() const
        {
            return data_ != nullptr || str_len != 0;
        }

        bool operator!=(const const_string_view &other) const
        {
            return !(*this == other);
        }

        bool operator==(const char *str) const
        {
            if (std::strlen(str) != str_len)
            {
                return false;
            }

            for (std::size_t i = 0; i < str_len; ++i)
            {
                if (data_[i] != str[i])
                {
                    return false;
                }
            }
            return true;
        }

        bool operator!=(const char *str) const
        {
            return !(*this == str);
        }

        bool operator==(const std::string &str) const
        {
            return *this == str.c_str();
        }

        bool operator!=(const std::string &str) const
        {
            return !(*this == str);
        }

        friend std::ostream &operator<<(std::ostream &os, const const_string_view &str)
        {
            // Copy only the string data to avoid printing garbage
            os.write(str.data_, str.str_len);
            os << "\0";
            return os;
        }

    private:
        const char* data_;
        std::size_t str_len;
    };

    class string_view
    {
    public:
        string_view(char *str, std::size_t len)
            : data_(str), str_len(len) {}

        string_view() : data_(nullptr), str_len(0) {}

        string_view(char *str)
            : data_(str), str_len(std::strlen(str)) {}

        string_view find(const char *str) const
        {
            char* found = std::strstr(data_, str);

            if (found)
            {
                return string_view(found, std::strlen(str));
            }

            return string_view();
        }

        string_view &operator=(const char *str)
        {
            size_t in_str_len = std::strlen(str);

            if (in_str_len > str_len)
            {
                assert(false && "Input String too long for replacing in the string_view");
            }

            for (std::size_t i = 0; i < in_str_len; ++i)
            {
                data_[i] = str[i];
            }

            for (std::size_t i = in_str_len; i <= str_len; ++i)
            {
                data_[i] = ' ';
            }

            return *this;
        }

        string_view &operator=(const std::string &str)
        {
            return *this = str.c_str();
        }

        const char* data() const { return data_; }
        std::size_t size() const { return str_len; }

        const char* begin() const { return data_; }
        const char* end() const   { return data_ + str_len; }

        char operator[](std::size_t i) const
        {
            return data_[i];
        }

        bool operator==(const string_view &other) const
        {
            if (str_len != other.str_len)
            {
                return false;
            }

            for (std::size_t i = 0; i < str_len; ++i)
            {
                if (data_[i] != other.data_[i])
                {
                    return false;
                }
            }

            return true;
        }

        operator bool() const
        {
            return data_ != nullptr || str_len != 0;
        }

        bool operator!=(const string_view &other) const
        {
            return !(*this == other);
        }

        bool operator==(const char *str) const
        {
            if (std::strlen(str) != str_len)
            {
                return false;
            }

            for (std::size_t i = 0; i < str_len; ++i)
            {
                if (data_[i] != str[i])
                {
                    return false;
                }
            }
            return true;
        }

        bool operator!=(const char *str) const
        {
            return !(*this == str);
        }

        bool operator==(const std::string &str) const
        {
            return *this == str.c_str();
        }

        bool operator!=(const std::string &str) const
        {
            return !(*this == str);
        }

        friend std::ostream &operator<<(std::ostream &os, const string_view &str)
        {
            // Copy only the string data to avoid printing garbage
            os.write(str.data_, str.str_len);
            os << "\0";
            return os;
        }

        friend std::istream &operator>>(std::istream &is, string_view &str)
        {
            is >> str.data_;
            return is;
        }

    private:
        char *data_;
        std::size_t str_len;
    };

    // Class representing a fixed-size string
    template <std::size_t N>
    class array_string
    {
    public:
        constexpr array_string(const char *str)
        {
            str_len = std::strlen(str);
            if (str_len > N)
            {
                assert(false && "String too long for array_string");
            }
            memccpy(data.data(), str, '\0', str_len);
        }

        constexpr const char *c_str() const
        {
            return data.data();
        }

        constexpr std::size_t size() const
        {
            return str_len;
        }

        constexpr std::size_t length() const
        {
            return str_len;
        }

        string_view view() const
        {
            return string_view(data.data());
        }

        string_view find(const char *str) const
        {
            if (std::strlen(str) > N)
            {
                return string_view();
            }

            char *found = std::strstr(data.data(), str);

            if (found)
            {
                return string_view(found, std::strlen(str));
            }

            return string_view();
        }

        constexpr char* begin()
        {
            return data.data();
        }

        constexpr char* end()
        {
            return data.data() + str_len + 1;
        }

        constexpr const char* cbegin() const
        {
            return data.data();
        }

        constexpr const char* cend() const
        {
            return data.data() + str_len + 1;
        }

        constexpr char operator[](std::size_t i) const
        {
            return data[i];
        }

        constexpr char& operator[](std::size_t i)
        {
            return data[i];
        }

        constexpr bool operator==(const array_string<N>& other) const
        {
            for (std::size_t i = 0; i < other.size() + 1; ++i)
            {
                if (data[i] != other.data[i])
                {
                    return false;
                }
            }
            return true;
        }

        constexpr bool operator!=(const array_string<N>& other) const
        {
            return !(*this == other);
        }

        constexpr bool operator==(const char* str) const
        {
            if (std::strlen(str) != N - 1)
            {
                return false;
            }

            for (std::size_t i = 0; i < N; ++i)
            {
                if (data[i] != str[i])
                {
                    return false;
                }
            }
            return true;
        }

        constexpr bool operator!=(const char* str) const
        {
            return !(*this == str);
        }

        constexpr bool operator==(const std::string &str) const
        {
            return *this == str.c_str();
        }

        constexpr array_string<N> &operator=(const char *str)
        {
            assertion(std::strlen(str));
            memccpy(data, str, '\0', N);
            return *this;
        }
        
        constexpr std::array<char, N>& get_data()
        {
            return data;
        }   

        constexpr array_string<N> &operator=(const array_string<N> &other)
        {
            data = other.data;
            return *this;
        }

        constexpr array_string<N> &operator=(const std::string &str)
        {
            assertion(str.size());
            memccpy(data, str.c_str(), '\0', N);
            return *this;
        }

        constexpr array_string<N> operator=(const string_view &view)
        {
            assertion(view.size());
            memccpy(data.data(), view.begin(), '\0', view.size());
            return *this;
        }

        constexpr array_string<N> &operator+=(const char *str)
        {
            uint32_t in_str_len = std::strlen(str);
            assertion( str_len + in_str_len);

            memccpy(data.data() + str_len , str, '\0', in_str_len + 1);
            return *this;
        }

        constexpr array_string<N> &operator+=(const std::string &str)
        {
            return *this += str.c_str();
        }

        constexpr array_string<N> &operator+=(const array_string<N> &other)
        {
            assertion(str_len + other.size());
            memccpy(data.data() + str_len, other.data.data(), '\0', other.size() + 1);
            return *this;
        }

        constexpr array_string<N> &operator+=(const string_view &view)
        {
            assertion(view.size());
            memccpy(data.data() + str_len, view.begin(), '\0', view.size() + 1);
            return *this;
        }

        friend std::ostream &operator<<(std::ostream &os, const array_string<N> &str)
        {
            os << str.c_str();
            return os;
        }

        friend std::istream &operator>>(std::istream &is, array_string<N> &str)
        {
            is >> str.c_str();
            return is;
        }

        bool assertion(const uint32_t &val)
        {
            if (val > N)
            {
                assert(false && "String too long for array_string");
            }
            return true;
        }

    private:
        std::array<char, N> data;
        uint32_t str_len;
    };

    // Constants for the block size and buffer size
    static constexpr std::size_t BLOCK_SIZE = 64;
    static constexpr std::size_t BUFFER_SIZE = 1024 * 1024 * 64; // 64 MB

    // Class representing a contiguous memory block segment
    class block
    {
    public:
        static constexpr std::size_t size = BLOCK_SIZE;

        block(char* ptr = nullptr, std::size_t blk_count = 0)
            : data_(ptr), blk_count_(blk_count) {}

        std::size_t block_count() const { return (blk_count_); }
        std::size_t capacity() const { return (blk_count_) * (size); }
        bool is_valid() const { return data_ != nullptr; }
        char* data() const { return data_; }

        block(const block &other)
            : data_(other.data_), blk_count_(other.blk_count_) {}

        block(block &&other)
            : data_(other.data_), blk_count_(other.blk_count_)
        {
            other.data_ = nullptr;
            other.blk_count_ = 0;
        }

        block &operator=(const block &other)
        {
            data_ = other.data_;
            blk_count_ = other.blk_count_;
            return *this;
        }

        block &&operator=(block &&other)
        {
            data_ = other.data_;
            blk_count_ = other.blk_count_;
            other.data_ = nullptr;
            other.blk_count_ = 0;
            return std::move(*this);
        }

        bool operator==(const block &other) const
        {
            return data_ == other.data_ && blk_count_ == other.blk_count_;
        }

        bool operator!=(const block &other) const
        {
            return !(*this == other);
        }

        bool operator<(const block &other) const
        {
            if (this->capacity() == other.capacity())
                return this->data() < other.data();

            return this->capacity() < other.capacity();
        }

        bool operator>(const block &other) const
        {
            if (this->capacity() == other.capacity())
                return this->data() > other.data();

            return this->capacity() > other.capacity();
        }

        void copy_contents_from(block &other) const
        {
            if(other.capacity() > capacity())
            {
                std::cout << "gp_std::string class : copy_contents_from : Capacity mismatch\n";
                return;
            }
            std::memcpy(data_, other.data_, other.capacity());
        }

    private:
        char *data_;            // Pointer to a segment in the buffer
        std::size_t blk_count_; // Number of contiguous 64-char blocks represented
    };

    // Allocator class that manages blocks within the shared buffer
    class block_allocator
    {
    public:
        // Singleton buffer for all strings
        static std::array<char, BUFFER_SIZE> &string_buffer()
        {
            static std::array<char, BUFFER_SIZE> buffer{};
            return buffer;
        }

        block_allocator()
            : m_buffer(string_buffer().data()), m_curr_offset(0) {}

        block_allocator(char *buffer, std::size_t size)
            : m_buffer(buffer), m_curr_offset(0) {}

        block_allocator(const block_allocator &other)
            : m_buffer(other.m_buffer), m_curr_offset(other.m_curr_offset) {}

        block_allocator(block_allocator &&other)
            : m_buffer(other.m_buffer), m_curr_offset(other.m_curr_offset)
        {
            other.m_buffer = nullptr;
            other.m_curr_offset = 0;
        }

        // Allocate a single contiguous block representing multiple 64-char blocks
        block* allocate_blocks(std::size_t size)
        {
            std::size_t num_blocks = (size + block::size - 1) / block::size;
            
            if(m_curr_offset + (num_blocks * block::size) > string_buffer().size() && m_free_blks_capacity < num_blocks)
            {
                defragment();
            }
            
            if(m_curr_offset + (num_blocks * block::size) > string_buffer().size())
            {
                std::cout << "gp_std::string class :Allocator Out of memory\n";
                return nullptr;
            }

            block blk(m_buffer + m_curr_offset, num_blocks);
            m_curr_offset += num_blocks * block::size;
            m_active_blks.push_back(blk);
            return &m_active_blks.back();
        }

        void free_block(const block &blk)
        {
            if (blk.is_valid())
            {
                m_free_blks_capacity += blk.block_count();
            }
        }

        size_t capacity() const
        {
            return string_buffer().size() - m_curr_offset;
        }

        void defragment()
        {
           uint32_t new_offset = 0;
           for(auto& blk : m_active_blks)
           {
               new_offset += blk.block_count() * block::size;
               block new_blk(m_buffer + new_offset, blk.block_count());
               new_blk.copy_contents_from(blk);
               blk = static_cast<block&&>(new_blk);
           }
           
           m_curr_offset = new_offset;
           m_free_blks_capacity = 0;
        }

    private:
        char* m_buffer;
        std::size_t m_curr_offset;
        uint32_t m_free_blks_capacity;
        std::deque<block> m_active_blks;
    };

    /// @class string
    /// @brief Fast Dynamic string class with small string optimization
    class string
    {
    public:
        string() : blk_(), str_len(0), m_allocator(default_allocator())
        {
            allocate("");
        }

        string(uint32_t len) : blk_(), str_len(len), m_allocator(default_allocator())
        {
            blk_ = allocator()->allocate_blocks(len + 1 + (block::size * 2));
            str_len = len;
            blk_->data()[len] = '\0'; // Null-terminate the string
        }

        string(const char *str) : str_len(0), m_allocator(default_allocator())
        {
            allocate(str);
        }

        string(const string &other) : str_len(0), m_allocator(default_allocator())
        {
            allocate(other.c_str());
        }

        string(string &&other) : str_len(0), m_allocator(default_allocator())
        {
            blk_ = other.blk_;
            other.blk_ = other.allocator()->allocate_blocks((block::size * 2));
            str_len = other.str_len;
            other.str_len = 0;
        }

        string(const char *begin, const char *end) : str_len(0), m_allocator(default_allocator())
        {
            std::size_t len = end - begin;
            blk_ = allocator()->allocate_blocks(len + 1 + (block::size * 2));
            copy_data_to_block(begin, len);
            str_len = len;
        }

        string &&operator=(string &&other)
        {
            if (this != &other)
            {
                other.blk_ = blk_;
                other.str_len = str_len;
                other.m_allocator = m_allocator;
                blk_ = allocator()->allocate_blocks((block::size * 2));
                str_len = 0;
            }
            return std::move(*this);
        }

        string(const std::string &str)
        {
            allocate(str.c_str());
        }

        string &operator=(const char *new_str)
        {
            std::size_t new_len = std::strlen(new_str);

            // Reuse current block if it has enough capacity
            if (new_len + 1 <= blk_->capacity())
            {
                copy_data_to_block(new_str, new_len);
                str_len = new_len;
            }
            else
            {
                allocator()->free_block(*blk_);
                allocate(new_str);
            }
            return *this;
        }

        string &operator=(const string &other)
        {
            if (this != &other)
            {
                this->blk_ = other.blk_;
                this->str_len = other.str_len;
                this->m_allocator = other.m_allocator;
            }
            return *this;
        }

        friend std::ostream &operator<<(std::ostream &os, const gp_std::string &str)
        {
            os << str.c_str();
            return os;
        }

        friend std::istream &operator>>(std::istream &is, gp_std::string &str)
        {
            std::string temp;
            is >> temp;
            str = temp.c_str();
            return is;
        }

        string operator+(const char* str) const
        {
            size_t new_len = std::strlen(str) + capacity();
            string new_str(new_len);
            std::strncat(new_str.blk_->data(), c_str(), str_len);
            std::strncat(new_str.blk_->data(), str, new_len);
            return new_str;
        }

        string operator+(const std::string &str) const
        {
            return *this + str.c_str();
        }

        string operator+(const string &other) const
        {
            return *this + other.c_str();
        }

        string operator+(const string_view &view) const
        {
            size_t new_len = view.size() + capacity();
            string new_str(new_len);
            std::strncat(new_str.blk_->data(), c_str(), str_len);
            std::strncat(new_str.blk_->data(), view.data(), new_len);
            return new_str;
        }

        string& operator+=(const char *str)
        {
            size_t new_len = std::strlen(str) + capacity();
            this->reserve(new_len);
            std::strncat(blk_->data(), str, new_len);
            str_len = new_len;
            return *this;
        }

        string& operator+=(const std::string &str)
        {
            *this += str.c_str();
            return *this;
        }

        string &operator+=(const string &other)
        {
            *this = *this + other;
            return *this;
        }

        string &operator+=(const string_view &view)
        {
            *this = *this + view;
            return *this;
        }

        string_view view()
        {
            return string_view(begin(), size());
        }

        string_view find(const char *str)
        {
            char *found = std::strstr(begin(), str);

            if (found)
            {
                return string_view(found, std::strlen(str));
            }

            return string_view();
        }

        void clear()
        {
            str_len = 0;
        }

        void swap(string &other)
        {
            std::swap(blk_, other.blk_);
            std::swap(str_len, other.str_len);
            std::swap(m_allocator, other.m_allocator);
        }

        operator bool() const
        {
            return str_len != 0;
        }

        // Returns the stored C-string
        const char *c_str() const { return blk_->is_valid() ? blk_->data() : ""; }

        char *begin() { return blk_->data(); }
        char *end() { return blk_->data() + str_len; }

        const char *begin() const { return blk_->data(); }
        const char *end() const { return blk_->data() + str_len; }

        std::size_t size() const { return str_len; }
        std::size_t length() const { return str_len; }
        std::size_t capacity() const { return blk_->capacity(); }

        char operator[](std::size_t i) const
        {
            return blk_->data()[i];
        }

        char &operator[](std::size_t i)
        {
            return blk_->data()[i];
        }

        bool operator==(const gp_std::string &other) const
        {
            return std::strcmp(c_str(), other.c_str()) == 0;
        }

        bool operator!=(const gp_std::string &other) const
        {
            return !(*this == other);
        }

        bool operator==(const char *str) const
        {
            return std::strcmp(c_str(), str) == 0;
        }

        bool operator!=(const char *str) const
        {
            return !(*this == str);
        }

        bool operator==(const std::string &str) const
        {
            return *this == str.c_str();
        }

        bool operator!=(const std::string &str) const
        {
            return !(*this == str);
        }

        void reserve(const std::size_t &len) const
        {
            m_reserve(len);
        }

        static std::size_t block_size() { return block::size; }
        static std::size_t default_buffer_capacity() { return default_allocator()->capacity(); }

        std::size_t buffer_capacity() { return allocator()->capacity(); }

        // Destructor to free the block upon object destruction
        ~string()
        {
            allocator()->free_block(*blk_);
        }

        block_allocator *allocator() const
        {
            return m_allocator;
        }

        void set_allocator(block_allocator *allocator)
        {
            m_allocator = allocator;
        }

    private:
        // Internal function to allocate blocks for a given string
        void allocate(const char *str)
        {
            std::size_t len = std::strlen(str);
            blk_ = allocator()->allocate_blocks(len + 1 + (block::size * 2));
            copy_data_to_block(str, len);
            str_len = len;
        }

        void m_reserve(const std::size_t &len) const
        {
            if (len + 1 <= blk_->capacity())
            {
                return;
            }

            string temp = *this;

            auto new_blk = allocator()->allocate_blocks(len + 1 + (block::size * 2));

            std::strncpy(new_blk->data(), c_str(), len);

            new_blk->data()[len] = '\0'; // Null-terminate the string

            allocator()->free_block(*blk_);

            blk_ = new_blk;
        }

        // Helper function to copy data into the block
        void copy_data_to_block(const char *str, std::size_t len)
        {
            std::strncpy(blk_->data(), str, len);
            blk_->data()[len] = '\0'; // Null-terminate the string
        }

        static block_allocator* default_allocator()
        {
            static block_allocator _allocator;
            return &_allocator;
        }

    private:
        mutable block* blk_;          // Single contiguous block for this string
        mutable uint32_t str_len;     // Size of the string
        block_allocator* m_allocator; // Allocator for the string
    };

} // namespace gp_std

#endif // GP_STD_STRING_HPP
