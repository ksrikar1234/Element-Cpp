#ifndef _GP_STD_128_BIT_HASH_MAP_H_
#define _GP_STD_128_BIT_HASH_MAP_H_

#include <deque>
#include <memory>
#include <vector>
#include <array>
#include <stack>
#include <stdexcept>
#include <algorithm>
#include "gp_atomic.hpp"
#include "gp_optional.hpp"
#include "gp_compute_device.hpp"
#include "gp_scopeguard.hpp"

namespace gp_std
{
    // Custom 128-bit hash struct
    struct _128_BIT_HASH_
    {
        /// @brief 128-bit numeber emulated using two 64-bit numbers
        struct _128_bit_type
        {
            uint64_t _64_bit_id[2];

            _128_bit_type() : _64_bit_id{0, 0} {}

            uint32_t operator[](const size_t &index) const
            {
                if (index == 0)
                {
                    return uint32_t(_64_bit_id[0] & 0xFFFFFFFF);
                }
                else if (index == 1)
                {
                    return uint32_t((_64_bit_id[0] >> 32) & 0xFFFFFFFF);
                }
                else if (index == 2)
                {
                    return uint32_t(_64_bit_id[1] & 0xFFFFFFFF);
                }
                else 
                {
                    return uint32_t((_64_bit_id[1] >> 32) & 0xFFFFFFFF);
                }
            }

            void set_32_bit_field(const size_t &index, const uint32_t &value)
            {
                if (index == 0)
                {
                    _64_bit_id[0] = (static_cast<uint64_t>(this->operator[](1)) << 32) | value;
                }
                else if (index == 1)
                {
                    _64_bit_id[0] = (static_cast<uint64_t>(value) << 32) | this->operator[](0);
                }
                else if (index == 2)
                {
                    _64_bit_id[1] = (static_cast<uint64_t>(this->operator[](3)) << 32) | value;
                }
                else if (index == 3)
                {
                    _64_bit_id[1] = (static_cast<uint64_t>(value) << 32) | this->operator[](2);
                }
                else
                {
                    printf("Bit Field Index out of range");
                }
            }
        }; // end of _128_bit_type
        
        private:
        
         _128_bit_type _128_bit_id;
        
        public :

        _128_BIT_HASH_() : _128_bit_id()
        {
            _128_bit_id._64_bit_id[0] = 0;
            _128_bit_id._64_bit_id[1] = 0;
        }

        _128_BIT_HASH_(const _128_BIT_HASH_ &hash)
        {
            _128_bit_id._64_bit_id[0] = hash._128_bit_id._64_bit_id[0];
            _128_bit_id._64_bit_id[1] = hash._128_bit_id._64_bit_id[1];
        }

        _128_BIT_HASH_(const uint64_t &id_1, const uint64_t &id_2)
        {
            if (id_1 < id_2)
            {
                _128_bit_id._64_bit_id[0] = id_1;
                _128_bit_id._64_bit_id[1] = id_2;
            }
            else
            {
                _128_bit_id._64_bit_id[0] = id_2;
                _128_bit_id._64_bit_id[1] = id_1;
            }
        }

        void encode_hash(const size_t &value0, const size_t &value1 = 0, const size_t &value2 = 0, const size_t &value3 = 0)
        {
            set_32_bit_field(0, value0); set_32_bit_field(1, value1);
            set_32_bit_field(2, value2); set_32_bit_field(3, value3);
        }

        _128_BIT_HASH_ &operator=(const _128_BIT_HASH_ &other)
        {
            _128_bit_id._64_bit_id[0] = other._128_bit_id._64_bit_id[0];
            _128_bit_id._64_bit_id[1] = other._128_bit_id._64_bit_id[1];
            return *this;
        }

        _128_BIT_HASH_ &operator=(_128_BIT_HASH_ &&other)
        {
            _128_bit_id._64_bit_id[0] = other._128_bit_id._64_bit_id[0];
            _128_bit_id._64_bit_id[1] = other._128_bit_id._64_bit_id[1];
            other.invalidate();
            return *this;
        }


        bool operator==(const _128_BIT_HASH_ &hash) const
        {
            return _128_bit_id._64_bit_id[0] == hash._128_bit_id._64_bit_id[0] && _128_bit_id._64_bit_id[1] == hash._128_bit_id._64_bit_id[1];
        }

        bool operator==(_128_BIT_HASH_ &hash)
        {
            return _128_bit_id._64_bit_id[0] == hash._128_bit_id._64_bit_id[0] && _128_bit_id._64_bit_id[1] == hash._128_bit_id._64_bit_id[1];
        }

        bool operator!=(const _128_BIT_HASH_ &hash) const
        {
            return _128_bit_id._64_bit_id[0] != hash._128_bit_id._64_bit_id[0] || _128_bit_id._64_bit_id[1] != hash._128_bit_id._64_bit_id[1];
        }

        bool operator>(_128_BIT_HASH_ &hash)
        {
            if (_128_bit_id._64_bit_id[0] != hash._128_bit_id._64_bit_id[0])
                return _128_bit_id._64_bit_id[0] > hash._128_bit_id._64_bit_id[0];

            else
            {
                if (_128_bit_id._64_bit_id[1] > hash._128_bit_id._64_bit_id[1])
                    return true;
            }
        }

        bool operator<(const _128_BIT_HASH_ &hash) const
        {
            return (*this < hash);
        }

        uint64_t& operator[](const size_t &index)
        {
            if (index == 0)
            {
                return _128_bit_id._64_bit_id[0];
            }
            else 
            {
                return _128_bit_id._64_bit_id[1];
            }
        }

        const uint64_t& operator[](const size_t &index) const
        {
            if (index == 0)
                return _128_bit_id._64_bit_id[0];
            else 
                return _128_bit_id._64_bit_id[1];
        }

        _128_BIT_HASH_ &operator++()
        {
            if (_128_bit_id._64_bit_id[1] == 0xffffffffffffffff) 
                _128_bit_id._64_bit_id[0] += 1;

            _128_bit_id._64_bit_id[1] += 1;
            return *this;
        }

        _128_BIT_HASH_ operator++(int)
        {
            _128_BIT_HASH_ temp = *this; ++(*this);
            return temp;
        }

        _128_BIT_HASH_& operator--()
        {
            if (_128_bit_id._64_bit_id[1] == 0)
            {
                _128_bit_id._64_bit_id[0] -= 1;
            }
            _128_bit_id._64_bit_id[1] -= 1;
            return *this;
        }

        _128_BIT_HASH_ operator--(int)
        {
            _128_BIT_HASH_ temp = *this;  --(*this);
            return temp;
        }

        _128_BIT_HASH_ &operator+=(const _128_BIT_HASH_ &hash)
        {
            _128_bit_id._64_bit_id[0] += hash._128_bit_id._64_bit_id[0];
            _128_bit_id._64_bit_id[1] += hash._128_bit_id._64_bit_id[1];
            return *this;
        }

        _128_BIT_HASH_ operator+(const _128_BIT_HASH_ &hash) const
        {
            _128_BIT_HASH_ temp = *this; temp += hash;
            return temp;
        }

        _128_BIT_HASH_ &operator-=(const _128_BIT_HASH_ &hash)
        {
            _128_bit_id._64_bit_id[0] -= hash._128_bit_id._64_bit_id[0];
            _128_bit_id._64_bit_id[1] -= hash._128_bit_id._64_bit_id[1];
            return *this;
        }

        _128_BIT_HASH_ operator-(const _128_BIT_HASH_ &hash) const
        {
            _128_BIT_HASH_ temp = *this;
            temp -= hash;
            return temp;
        }

        void set_32_bit_field(const size_t &index, const uint32_t &value)
        {
            _128_bit_id.set_32_bit_field(index, value);
        }

        void invalidate()
        {
            _128_bit_id._64_bit_id[0] = 0xffffffffffffffff;
            _128_bit_id._64_bit_id[1] = 0xffffffffffffffff;
        }

        bool is_numeric_limit()
        {
            return _128_bit_id._64_bit_id[0] == 0xffffffffffffffff && _128_bit_id._64_bit_id[1] == 0xffffffffffffffff;
        }

        bool is_valid()
        {
            return !is_numeric_limit();
        }
    };
    
     /// @brief Custom hash128_t type
     using hash128_t = _128_BIT_HASH_;
 
    /// @brief Custom hash function specialization for _128_BIT_HASH_
    /// Example : Custom hash function specialization for _128_BIT_HASH_
    template <typename Key>
    struct hash
    {
        hash128_t operator()(const Key &key) const
        {
            // Custom hash function implementation
            hash128_t hash_val;
            hash_val.encode_hash(std::hash<Key>{}(key) + 1, std::hash<Key>{}(key), std::hash<Key>{}(key) + 1, std::hash<Key>{}(key));
            return hash_val;
        }
    };

    /// @class  hash_map_128_t
    /// @brief  Custom hash map class with 128-bit hash tables
    /// @tparam Key The key type
    /// @tparam Value The value type
    /// @tparam max_domains The maximum number of domains
    /// @tparam HashFunc The hash function
    /// @tparam base_container The base container type
    /// @note   Custom implementation of a concurrent hash map with 128-bit hash tables
    /// @note   Provides support for  
    /// @note   single threaded insert, atomic_insert, concurrent_search, get, remove, and contains operations
    /// @note   Provides custom iterator support

    template <typename Key, typename Value, typename HashFunc = gp_std::hash<Key> , template <typename...> class base_container = std::deque>
    class hash_map_128_t
    {
        // // HashFunc Function should be a callable object and return hash128_t
        // static_assert(std::is_invocable<HashFunc, Key>::value, "HashFunc function must be a callable object");
        // static_assert(std::is_same<decltype(std::declval<HashFunc>()(std::declval<Key>())), hash128_t>::value, "HashFunc function must return hash128_t"); 
        
        // base_container class should have the following methods
        // 1. push_back
        // 2. size
        // 3. operator[]
        // 4. begin
        // 5. end
    public:
        /// @class hash_map_128_t::pair
        /// @brief Custom pair struct (equivalent to HashNode)
        template <typename Key_T, typename Value_T>
        struct pair
        {
            union
            {
                struct 
                {
                    Key_T   key;
                    mutable Value_T value;
                };

                struct 
                {
                    Key_T   first;
                    mutable Value_T second;
                };
            };
            
            hash128_t hash_value;

            pair(const Key_T& key, const Value_T &value, const hash128_t &hash_value) : key(key), value(value), hash_value(hash_value) {}
            pair(Key_T&& key, Value_T&& value, const hash128_t& hash_value) : key(std::move(key)), value(std::move(value)), hash_value(hash_value) {}
           ~pair() {}

            pair(const pair &p)
            {
                key = p.key;
                value = p.value;
                hash_value = p.hash_value;
            }

            pair &operator=(const pair &p)
            {
                key = p.key;
                value = p.value;
                hash_value = p.hash_value;
                return *this;
            }

            pair& operator=(pair&& p)
            {
                key = std::move(p.key);
                value = std::move(p.value);
                hash_value = std::move(p.hash_value);
                return *this;
            }

            void invalidate()
            {
                hash_value.invalidate();
            }

            bool is_valid()
            {
                return hash_value.is_valid();
            }

        }; // struct hash_map_128_t::pair
    
    private:
        HashFunc hash_fun;
        
        size_t max_domains;

        std::vector<base_container<pair<Key, Value>>> hash_table; 
        
        class domain_lock
        {
            public :
            domain_lock() : value_modifier_lock(), push_back_lock() {}
            gp_std::spinlock value_modifier_lock;
            gp_std::spinlock push_back_lock;
        };

        std::deque<domain_lock> domain_locks;

        gp_std::spinlock resize_lock;

        gp_std::cpu_compute_device* external_device;

    public:
        // Constructor
        hash_map_128_t(const uint32_t& bucket_count = 64) : hash_fun(), max_domains(bucket_count), hash_table(bucket_count), domain_locks(bucket_count), external_device(nullptr) { } 

        // Destructor
        ~hash_map_128_t() {  }

        hash_map_128_t& operator=(const hash_map_128_t& other)
        {
            gp_std::scoped_lock<gp_std::spinlock> lock(other.resize_lock);
            if(this == &other) return *this;
            hash_table = other.hash_table;
            return *this;
        }  
        
        hash_map_128_t& operator=(hash_map_128_t&& other)
        {
            gp_std::scoped_lock<gp_std::spinlock> lock(other.resize_lock);    
            if(this == &other) return *this;
            max_domains = other.max_domains;
            hash_table = std::move(other.hash_table);
            domain_locks = std::move(other.domain_locks);
            other.max_domains = 0;
            other.hash_table.clear();
            other.domain_locks.clear();
            return *this;
        }

        hash_map_128_t(const hash_map_128_t& other)
        {
            gp_std::scoped_lock<gp_std::spinlock> lock(other.resize_lock);
            hash_table = other.hash_table;
        }

        hash_map_128_t(hash_map_128_t&& other)
        {
            gp_std::scoped_lock<gp_std::spinlock> lock(other.resize_lock);
            hash_table = std::move(other.hash_table);
        }
  
        ///@brief Move key-value pair to the hashmap
        const pair<Key,Value>& insert(Key&& key, Value&& value)
        {
            /// Search if the key already exists
            hash128_t hash_val  = hash_fun(key);
            size_t domain_index = eval_domain_index(hash_val);
            gp_std::optional<size_t> key_index = search_concurrent(key, hash_val, domain_index);

            if(key_index)
            {
                hash_table[domain_index][key_index.value()].value = value;
                return hash_table[domain_index][key_index.value()];
            };

            /// else create a new pair in the domain
            hash_table[domain_index].push_back(std::move(pair<Key, Value>(std::move(key), std::move(value), hash_val)));
            return hash_table[domain_index].back();
        }     

        ///@brief Copy key-value pair to the hashmap
        const pair<Key,Value>& insert(const Key &key, const Value &value) 
        {
            /// Search if the key already exists
            hash128_t hash_val = hash_fun(key);
            size_t domain_index = eval_domain_index(hash_val);
            gp_std::optional<size_t> key_index = search_concurrent(key, hash_val, domain_index);
        
            if(key_index)
            {
                hash_table[domain_index][key_index.value()].value = value;
                return hash_table[domain_index][key_index.value()];
            }
            
            /// else create a new pair in the domain
            hash_table[domain_index].push_back(std::move(pair<Key, Value>(key, value, hash_val)));
            return hash_table[domain_index].back();
        }

        ///@brief Atomically Move key-value pair to the hashmap
        const pair<Key,Value>& atomic_insert(Key &&key, Value &&value) noexcept
        {
            /// Search if the key already exists
            hash128_t hash_val = hash_fun(key);
            size_t domain_index = eval_domain_index(hash_val);
            gp_std::optional<size_t> key_index = search_concurrent(key, hash_val, domain_index);

            if(key_index)
            {
                gp_std::scoped_lock<gp_std::spinlock> lock(domain_locks[domain_index].value_modifier_lock);
                
                try
                { hash_table[domain_index][key_index.value()].value = value; }
                catch(...) { }
                
                return hash_table[domain_index][key_index.value()];
            };

            /// else create a new pair in the domain

            gp_std::scoped_lock<gp_std::spinlock> lock(domain_locks[domain_index].push_back_lock);
            
            try
            { hash_table[domain_index].push_back(std::move(pair<Key, Value>(std::move(key), std::move(value), hash_val))); }
            catch(...) { }

            return hash_table[domain_index].back();

        }     

        ///@brief Atomically Copy key-value pair to the hashmap
        const pair<Key,Value>& atomic_insert(const Key &key, const Value &value) noexcept
        {
            /// Search if the key already exists
            hash128_t hash_val = hash_fun(key);
            size_t domain_index = eval_domain_index(hash_val);
            gp_std::optional<size_t> key_index = search_concurrent(key, hash_val, domain_index);
        
            if(key_index)
            {
                gp_std::scoped_lock<gp_std::spinlock> lock(domain_locks[domain_index].value_modifier_lock);
                
                try
                { hash_table[domain_index][key_index.value()].value = value; }
                catch(...) { }
                
                return hash_table[domain_index][key_index.value()];
            };

            /// else create a new pair in the domain
            gp_std::scoped_lock<gp_std::spinlock> lock(domain_locks[domain_index].push_back_lock);

            try
            { hash_table[domain_index].push_back(std::move(pair<Key, Value>(key, value, hash_val))); }
            catch(...) { }  

            return hash_table[domain_index].back();          
        }

        const pair<Key,Value>& insert(std::pair<Key, Value>&& pair)
        {
            return insert(std::move(pair.first), std::move(pair.second));
        }

        const pair<Key,Value>& insert(std::pair<Key&&, Value&&>&& pair)
        {
            return insert(std::move(pair.first), std::move(pair.second));
        }

        const pair<Key,Value>& insert(const std::pair<Key, Value> &pair)
        {
            return insert(pair.first, pair.second);
        }

        const pair<Key,Value>& insert(pair<Key, Value>& pair)
        {
            return insert(pair.first, pair.second);
        }

        const pair<Key,Value>& insert(pair<Key, Value>&& pair)
        {
            return insert(std::move(pair.first), std::move(pair.second));
        }

        const pair<Key,Value>& atomic_insert(std::pair<Key, Value>&& pair)
        {
            return atomic_insert(std::move(pair.first), std::move(pair.second));
        }

        const pair<Key,Value>& atomic_insert(std::pair<Key&&, Value&&>&& pair)
        {
            return atomic_insert(std::move(pair.first), std::move(pair.second));
        }

        const pair<Key,Value>& atomic_insert(const std::pair<Key, Value> &pair)
        {
            return atomic_insert(pair.first, pair.second);
        }

        const pair<Key,Value>& atomic_insert(pair<Key, Value>& pair)
        {
            return atomic_insert(pair.first, pair.second);
        }

        /// @brief Retrieve value associated with key
        /// @param key Key to search for
        /// @return Optional Value associated with key
        gp_std::optional<Value&> get(const Key &key) const noexcept
        {
            hash128_t hash_val = hash_fun(key);
            size_t domain_index = eval_domain_index(hash_val);

            gp_std::optional<size_t> key_index = search_concurrent(key);
            if (key_index)
            {
                return hash_table[eval_domain_index(hash_fun(key))][key_index.value()].value;
            }
            return gp_std::nullopt_t();
        }

        /// @brief Atomically Retrieve value associated with key
        /// @param key Key to search for
        /// @return Optional Value associated with key
        gp_std::optional<Value&> atomic_get(const Key &key) noexcept
        {
            hash128_t hash_val = hash_fun(key);
            size_t domain_index = eval_domain_index(hash_val);

            gp_std::optional<size_t> key_index = search_concurrent(key, hash_val, domain_index);
            if (key_index)
            {
                gp_std::scoped_lock<gp_std::spinlock> lock(domain_locks[domain_index].value_modifier_lock);
                Value& value = hash_table[eval_domain_index(hash_fun(key))][key_index.value()].value;
                return value;
            }
            return gp_std::nullopt_t();
        }
      
        ///@brief Remove key-value pair from the hashmap
        void remove(const Key &key) noexcept
        {
            hash128_t hash_val = hash_fun(key);
            size_t domain_index = eval_domain_index(hash_val);

            gp_std::optional<size_t> key_index = search_concurrent(key, hash_val, domain_index);

            if (key_index)
            {
                hash_table[domain_index][key_index.value()].invalidate();
            }
        }

        void resize(const size_t &new_domain_count)
        {
            rehash(new_domain_count);
        }

        void clear()
        {
            hash_table.clear();
        }

        uint32_t size() const 
        {
            return get_total_size();
        }

        uint32_t domain_size(const size_t &domain_index) const
        {
            return get_domain_size(domain_index);
        }

        uint32_t buckets_count() const
        {
            return get_domain_count();
        }

        ///@brief Check if key exists in the hashmap
        bool contains(const Key &key) const noexcept
        {
            gp_std::optional<size_t> key_index = search_concurrent(key);
            return key_index.has_value();
        }

        ///@brief Get the size of the hashmap for a specific domain
        size_t get_domain_size(const size_t &domain_index) const noexcept
        {
            if (domain_index >= max_domains)
            {
                return 0;
            }
            return hash_table[domain_index].size();
        }

        ///@brief Get the size of the hashmap for a specific domain
        size_t get_domain_count() const noexcept
        {
            return max_domains;
        }

        ///@brief Get the total size of the hashmap across all max_domains
        size_t get_total_size() const noexcept
        {
            size_t total_size = 0;
            for(size_t i = 0; i < max_domains; ++i)
            {
                total_size += hash_table[i].size();
            }
            return total_size;
        }

        /// @brief hash_map_128_t::iterator class
        class iterator
        {
        public:
            iterator(hash_map_128_t<Key, Value, HashFunc, base_container>* hashmap) : m_hashmap(hashmap), m_domain_index(0), m_pair_index(0)
            {
                for(size_t i = 0; i < m_hashmap->get_domain_count(); i++)
                {
                    if(m_hashmap->hash_table[i].size() > 0)
                    {
                        m_domain_index = i;
                        m_pair_index = 0;
                        break;
                    }
                }
            }

            iterator(hash_map_128_t<Key, Value, HashFunc, base_container>* hashmap, const size_t& domain_index, const size_t& pair_index) : m_hashmap(hashmap), m_domain_index(domain_index), m_pair_index(pair_index), m_last_valid_pair(nullptr) {}

            void operator++() noexcept
            {
                if(m_domain_index == 0xffffffff && m_pair_index == 0xffffffff) return;
                forward();
            }

            pair<Key, Value>& operator*() noexcept
            {
                if(m_domain_index == 0xffffffff && m_pair_index == 0xffffffff)
                {
                    if(m_last_valid_pair) return *m_last_valid_pair; 
                    else throw std::out_of_range("Attempt to dereference end() iterator");
                }

                m_last_valid_pair = &m_hashmap->hash_table[m_domain_index][m_pair_index];
                
                while (!(m_last_valid_pair->is_valid()))
                {
                    forward();
                    if (m_domain_index == 0xffffffff && m_pair_index == 0xffffffff)
                        break;
                    m_last_valid_pair = &m_hashmap->hash_table[m_domain_index][m_pair_index];
                }

                return *m_last_valid_pair;
            }

            pair<Key, Value>* operator->() noexcept
            {
                return &operator*();
            }

            bool operator==(const iterator &it) noexcept
            {
                return (m_domain_index == it.m_domain_index && m_pair_index == it.m_pair_index);
            }

            bool operator!=(const iterator &it) noexcept
            {
                return (m_domain_index != it.m_domain_index || m_pair_index != it.m_pair_index);
            }

            void forward() noexcept
            {
                if(m_domain_index == 0xffffffff && m_pair_index == 0xffffffff)
                    return;

                if(m_pair_index < m_hashmap->hash_table[m_domain_index].size() - 1)
                {
                    ++m_pair_index;
                }

                else
                {
                    size_t old_domain_index = m_domain_index;
                    for (size_t i = m_domain_index + 1; i <  m_hashmap->get_domain_count(); ++i)
                    {
                        // Find the next domain with a valid size
                        if (m_hashmap->hash_table[i].size() > 0)
                        {
                            m_domain_index = i;
                            m_pair_index = 0;
                            break;
                        }
                    }
                    
                    // This means could not find a valid domain within the max_domains
                    if(old_domain_index == m_domain_index)
                    {
                        m_domain_index = 0xffffffff;
                        m_pair_index   = 0xffffffff;
                    }
                }
            }

        private:
            hash_map_128_t<Key, Value, HashFunc, base_container> *m_hashmap;
            size_t m_domain_index;
            size_t m_pair_index;
            pair<Key, Value>* m_last_valid_pair;
        }; // end of iterator class

        ///@brief begin iterator
        iterator begin()  noexcept
        {   
            if(this->get_total_size() != 0)
               return iterator(this);
            else
               return end();
        }

        ///@brief end iterator
        iterator end() noexcept
        {
            return iterator(this, 0xffffffff, 0xffffffff);
        }

        ///@brief find the key in the hashmap and return iter
        iterator find(const Key &key) const noexcept
        {
            hash128_t hash_val = hash_fun(key);
            size_t domain_index = eval_domain_index(hash_val);
            gp_std::optional<size_t> key_index = search_concurrent(key, hash_val, domain_index);
            return (key_index) ? iterator(this, domain_index, key_index.value()) : end();
        }

        void load_compute_device(gp_std::cpu_compute_device& device)
        {
            external_device = &device;
        }

        void unload_compute_device()
        {
            external_device = nullptr;
        }

    private:

        class search_kernel : public base_kernel
        {
            public:
            search_kernel(const hash_map_128_t<Key, Value, HashFunc, base_container>* hashmap, const Key &key, const hash128_t &hash_val, const size_t &domain_index, gp_std::optional<size_t>& key_index)
                : m_hashmap(hashmap), m_key(key), m_hash_val(hash_val), m_domain_index(domain_index), m_key_index(key_index) {}   
            
            void operator()() override
            {
                min_max mm = get_current_wave_min_max(m_hashmap->hash_table[m_domain_index].size());
                
                // Split the min max into work batches
                std::array<min_max, 16> batch_mm;

                for(size_t i = 0; i < 16; ++i)
                {
                    batch_mm[i].min = mm.min + i * 16;
                    batch_mm[i].max = batch_mm[i].min + 16;
                }

                batch_mm[15].max = mm.max;
                
                for(auto& local_mm : batch_mm)
                {
                    if(task_progress->is_finished()) return;  // We Ocassionaly check if other threads have already found the key
                    for(size_t i = local_mm.min; i < local_mm.max; ++i)
                    {
                        if (m_hashmap->hash_table[m_domain_index][i].hash_value == m_hash_val)
                        {
                            m_key_index = i;
                            task_progress->finish(); // We signal that we have found the key
                            return;
                        }
                    }
                }
            }

            base_kernel* clone() const override
            {
                return new search_kernel(*this);
            }

            private:
            const hash_map_128_t<Key, Value, HashFunc, base_container> *m_hashmap;
            const Key& m_key;
            const hash128_t& m_hash_val;
            const size_t& m_domain_index;
            gp_std::optional<size_t>& m_key_index;
        };

        size_t eval_domain_index(const hash128_t& hash_val)
        {
            size_t domain_index = ((hash_val[0] % max_domains) + (hash_val[1] % max_domains)) % max_domains;
            return domain_index;
        }
        
        gp_std::optional<size_t> search_concurrent(const Key &key)
        {
            hash128_t hash_val = hash_fun(key);
            size_t domain_index = eval_domain_index(hash_val);
            return search_concurrent(key, hash_val, domain_index);
        }    

        gp_std::optional<size_t> search_concurrent(const Key &key, const hash128_t& in_hash_val, const size_t& in_domain_index)
        {
            hash128_t hash_val = in_hash_val;
            size_t domain_index = in_domain_index;
            
            gp_std::optional<size_t> key_index;

            external_device =  gp_std::compute_device::active_device();
            
            if(hash_table[domain_index].size() > 100 && external_device != nullptr) 
            {
                search_kernel kernel(this, key, hash_val, domain_index, key_index);
                external_device->load_kernel(&kernel);
                while(domain_locks[domain_index].push_back_lock.is_locked())
                {
                   // Wait for the push_back_lock to be released
                   // May be another thread is adding a new key-value pair in this domain
                }
                external_device->launch_waves();
                external_device->wait();
                return key_index;
            }
            else
            {
                for (size_t i = 0; i < hash_table[domain_index].size(); ++i)
                {
                    if (hash_table[domain_index][i].hash_value == hash_val)
                        return i;
                }
            }

            return gp_std::nullopt;
        }

        void rehash(const size_t &new_domain_count)
        {
            gp_std::scoped_lock<gp_std::spinlock> lock(resize_lock);

            hash_map_128_t<Key, Value, HashFunc, base_container> new_hash_map(new_domain_count);

            for(pair<Key, Value>& iter : *this)
            {
                if(iter.is_valid())
                    new_hash_map.no_check_insert(std::move(iter));
            }
            *this = std::move(new_hash_map);
        }

        /// @brief Insert the key-value pair without checking if the key already exists
        /// @param key Key to insert
        /// @param value Value to insert
        /// Usefull for rehashing
        void no_check_insert(const Key &key, const Value &value)
        {
            hash128_t hash_val = hash_fun(key);
            size_t domain_index = eval_domain_index(hash_val); 
            hash_table[domain_index].push_back(pair<Key, Value>(key, value, hash_val));
        }

        void no_check_insert(Key &&key, Value &&value)
        {
            hash128_t hash_val = hash_fun(key);
            size_t domain_index = eval_domain_index(hash_val);
            hash_table[domain_index].push_back(std::move(pair<Key, Value>(std::move(key), std::move(value), hash_val)));
        }

        void no_check_insert(pair<Key, Value>&& pair)
        {
            no_check_insert(std::move(pair.first), std::move(pair.second));
        }
    };
} // end of namespace gp_std
#endif
