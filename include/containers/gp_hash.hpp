#ifndef _GP_STD_HASH_TYPE_HPP_
#define _GP_STD_HASH_TYPE_HPP_

#include <cstdint>
#include <sstream>
#include "gp_array.hpp"

namespace gp_std
{
    template <size_t Size>
    class hash
    {
        static_assert(Size >= 32 && Size % 32 == 0, "Size must be a multiple of 32 and above 32 * 2^n {n = {0,1 ... }}");

    private:
        gp_std::array<uint32_t, Size / 32> data;

    public:
        hash()
        {
            data.fill(0);
        }

        template <typename... Fields>
        hash(Fields... fields)
        {
            data.fill(0);
            encode_hash(fields...);
        }

        bool operator==(const hash<Size> &other) const  { return data == other.data; }
        bool operator!=(const hash<Size> &other) const  { return !(*this == other);  }
        bool operator<(const hash<Size> &other)  const  { return data < other.data;  }
        bool operator>(const hash<Size> &other)  const  { return data > other.data;  }
        bool operator<=(const hash<Size> &other) const  { return data <= other.data; }
        bool operator>=(const hash<Size> &other) const  { return data >= other.data; }

        // Encode hash with variadic template parameters
        template <typename... Fields>
        void encode_hash(Fields... fields)
        {
            static_assert(sizeof...(Fields) <= Size / 32, "Exceeded maximum number of fields, must be less than Size / 32");

            gp_std::array<uint32_t, Size / 32> temp = {0}; // Ensure all elements are initialized to 0
            gp_std::array<uint32_t, sizeof...(Fields)> field_values = {static_cast<uint32_t>(fields)...};

            for (size_t i = 0; i < field_values.size(); ++i)
            { temp[i] = field_values[i]; }

            data = temp;
        }

        // Stream output operator (formatted as hex)
        friend std::ostream& operator<<(std::ostream &os, const hash<Size> &hash)
        {
            os << std::hex;
            for (size_t i = 0; i < hash.data.size(); ++i)
            {
                if (i > 0)
                    os << ":";
                os << hash.data[i];
            }
            os << std::dec;
            return os;
        }

        // Stream input operator (reads hex values)
        friend std::istream &operator>>(std::istream &is, hash<Size> &hash)
        {
            for (auto &val : hash.data)
            {
                is >> std::hex >> val;
            }
            is >> std::dec; // Restore decimal format
            return is;
        }
    };

} // namespace gp_std
#endif
