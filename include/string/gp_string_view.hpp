#ifndef _GP_STD_STRING_VIEW_HPP_
#ifndef _GP_STD_STRING_VIEW_HPP_

#include <cstdint>
#include <cstring>
#include <iostream>

namespace gp_std
{
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
        const char *end() const  { return data_ + str_len; }

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

    ///*******************************************************************************************************************
    //* const_string_view
    class string_view
    {
    public:
        string_view(char* str, std::size_t len)
            : data_(str), str_len(len) {}

        string_view() : data_(nullptr), str_len(0) {}

        string_view(char* str)
            : data_(str), str_len(std::strlen(str)) {}

        string_view find(const char* str) const
        {
            char* found = std::strstr(data_, str);

            if (found)
            {
                return string_view(found, std::strlen(str));
            }

            return string_view();
        }

        string_view &operator=(const char* str)
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

        string_view &operator=(const std::string& str)
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

        bool operator==(const string_view& other) const
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

        bool operator!=(const string_view& other) const
        {
            return !(*this == other);
        }

        bool operator==(const char* str) const
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

        bool operator!=(const char* str) const
        {
            return !(*this == str);
        }

        bool operator==(const std::string& str) const
        {
            return *this == str.c_str();
        }

        bool operator!=(const std::string& str) const
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
        char* data_;
        std::size_t str_len;
    };
}
