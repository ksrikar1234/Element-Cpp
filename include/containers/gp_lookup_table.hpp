#ifndef __GP_LOOKUP_TABLE_TYPES__
#define __GP_LOOKUP_TABLE_TYPES__

#include <memory>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <map>

namespace gp_std
{

// Primary API for lookup_table class. Go to this class for Actually using them
// A lookup table is a read-only data structure that maps keys to values
// Its Base Container is a std::vector of std::pair<Key, Value>
// When a key is looked up, the table returns a pointer to the value

// Advantages:
// It provides fast lookups and is optimized for read-only use cases
// The lookup table can be created from a std::map or std::unordered_map
// It supports both hash-based and tree-based lookup tables
// It is a header-only library

// Limitations:
// Insertions are not allowed in the lookup table
// The lookup table is not thread-safe yet but can be made thread-safe by using locks
// Its

// Usage:
// --> Create a lookup table from a std::map or std::unordered_map
// std::map<int, std::string> data = {{1, "one"}, {2, "two"}, {3, "three"}};
// std::unordered_map<int, std::string> data = {{1, "one"}, {2, "two"}, {3, "three"}};

// --> Construct a lookup table
// lookup_table<int, std::string> table(data);
// or 
// lookup_table<int, std::string> table(data.begin(), data.end(), size(if known));

// --> Query the lookup table whether its a hash table or tree table
//  table.is_tree_table() or table.is_hash_table()

// std::string* value = table.lookup(2); // or table[2]
// if (value)
// {
//     std::cout << *value << std::endl; // Output: two
// }
template <typename Key, typename Value>
class lookup_table;

namespace gp_private
{
template <typename Key, typename Value>
class base_lookup_table;

template <typename Key, typename Value, typename Hash, typename KeyEqual , typename Allocator>
class lookup_hashtable;

template <typename Key, typename Value, typename Compare , typename Allocator>
class lookup_treetable;

// Interface for lookup tables (Insertions are not allowed)
template <typename Key, typename Value>
class base_lookup_table
{
protected:
    std::vector<std::pair<Key, Value>> m_table;
    enum class table_type : uint8_t
    {
        hash_table = 0,
        tree_table = 1
    };
    
    table_type m_type;

public:
    virtual ~base_lookup_table() = default;
    virtual Value* lookup(const Key& key) const = 0;
    
    Value* operator[](const Key& key) const
    {
        Value* value = lookup(key);
        if(value) return value;
        else printf("Invalid Access !! Key Not Found in Looktable");
        return nullptr;
    }

    std::pair<const Key&, Value&>* begin() 
    { return this->m_table.begin(); }

    std::pair<const Key&, Value&>* end() 
    { return this->m_table.end();   }

    bool empty() const
    {
        return m_table.empty();
    }

    size_t size() const
    {
        return m_table.size();
    }

    bool operator==(const base_lookup_table<Key, Value>& other) const
    {
        return is_equal(other);
    }

    bool is_equal(const base_lookup_table<Key, Value>& other) const
    {
        if (m_table.size() != other.m_table.size())
        {
            return false;
        }

        for (size_t i = 0; i < m_table.size(); ++i)
        {
            if (m_table[i] != other.m_table[i])
            {
                return false;
            }
        }

        return true;
    }

    bool operator!=(const base_lookup_table<Key, Value>& other) const
    {
        return !(*this == other);
    }

    bool is_tree_table() const
    {
        return m_type == table_type::tree_table;
    }

    bool is_hash_table() const
    {
        return m_type == table_type::hash_table;
    }

    virtual std::shared_ptr<base_lookup_table<Key, Value>> clone() const = 0;
};

// Hash-based lookup table
template <typename Key, typename Value, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<const Key, Value>>>
class lookup_hashtable : public base_lookup_table<Key, Value>
{
private:
    std::vector<size_t> m_hashes;

public:
    explicit lookup_hashtable(const std::unordered_map<Key, Value, Hash, KeyEqual, Allocator>& data)
    {
       import_data(data);
       sort_hashtable();
    }

    explicit lookup_hashtable(std::unordered_map<Key, Value, Hash, KeyEqual, Allocator>&& data)
    {
        import_data(std::move(data));
        sort_hashtable();
    }

    template <typename It>
    explicit lookup_hashtable(It Begin, It End)
    {
        import_data(Begin, End);
        sort_hashtable();
    }

    Value* lookup(const Key& key) const override
    {
        size_t key_hash = std::hash<Key>{}(key);

        auto it = std::lower_bound(m_hashes.begin(), m_hashes.end(), key_hash);
        
        // If the hash value is found
        if (it != m_hashes.end() && *it == key_hash)
        {
            // This Logic is to handle the case where multiple keys have the same hash value
            size_t index = std::distance(m_hashes.begin(), it);
            while (index < m_hashes.size() && m_hashes[index] == key_hash)
            {
                // If the key is found
                if (this->m_table[index].first == key)
                {
                    return &this->m_table[index].second;
                }
                ++index;
            }
        }
        
        return nullptr;
    }

    std::shared_ptr<base_lookup_table<Key, Value>> clone() const override
    {
        return std::make_shared<lookup_hashtable<Key, Value, Hash, KeyEqual, Allocator>>(*this);
    }

    private:
    void import_data(std::unordered_map<Key, Value, Hash, KeyEqual, Allocator>&& data)
    {       
        this->m_type = base_lookup_table<Key, Value>::table_type::hash_table;
        this->m_table.reserve(data.size());
        m_hashes.reserve(data.size());

        for (const auto &entry : data)
        {
            size_t hash_value = std::hash<Key>{}(entry.first);
            this->m_table.emplace_back(std::move(entry.first), std::move(entry.second));
            m_hashes.push_back(hash_value);
        }

    }

    void import_data(const std::unordered_map<Key, Value, Hash, KeyEqual, Allocator>& data)
    {
        this->m_table.reserve(data.size());
        m_hashes.reserve(data.size());

        for (const auto &entry : data)
        {
            size_t hash_value = std::hash<Key>{}(entry.first);
            this->m_table.emplace_back((entry.first), (entry.second));
            m_hashes.push_back(hash_value);
        }
    }

    template <typename It>
    void import_data(It begin, It end, 
        typename std::enable_if<
            std::is_same<typename std::iterator_traits<It>::value_type, 
                         typename std::unordered_map<Key, Value>::value_type>::value, 
            int>::type = 0, uint32_t size = 0)
    {
        this->m_table.reserve(size);
        m_hashes.reserve(size);

        while (begin != end) {
            size_t hash_value = std::hash<Key>{}(begin->first);
            this->m_table.emplace_back(begin->first, begin->second);
            m_hashes.push_back(hash_value);
            ++begin;
        }
    }

    void sort_hashtable()
    {
        std::vector<size_t> indices(m_hashes.size());

        for(size_t i = 0; i < indices.size(); i++)
        {
            indices[i] = i;
        }

        std::sort(indices.begin(), indices.end(), [&](size_t i, size_t j)
                  { return m_hashes[i] < m_hashes[j]; });

        std::vector<std::pair<Key, Value>> sorted_table(this->m_table.size());
        std::vector<size_t> sorted_hashes(indices.size());

        for (size_t i = 0; i < indices.size(); i++)
        {
            sorted_table[i]  = this->m_table[indices[i]];
            sorted_hashes[i] = m_hashes[indices[i]];
        }

        this->m_table = std::move(sorted_table);
        m_hashes = std::move(sorted_hashes);
    }
};

// Tree-based lookup table
template <typename Key, typename Value, typename Compare = std::less<Key> , typename Allocator = std::allocator<std::pair<const Key, Value>>>
class lookup_treetable : public base_lookup_table<Key, Value>
{
public:
    explicit lookup_treetable(const std::map<Key, Value, Compare, Allocator>& data)
    {
        this->m_table.reserve(data.size());
        for (const auto &entry : data)
        {
            this->m_table.emplace_back(entry.first, entry.second);
        }
    }

    explicit lookup_treetable(std::map<Key, Value, Compare, Allocator>&& data)
    {
        this->m_table.reserve(data.size());
        for (auto &entry : data)
        {
            this->m_table.emplace_back(std::move(entry.first), std::move(entry.second));
        }
    }

    template <typename It>
    explicit lookup_treetable(It Begin, It End, 
        typename std::enable_if<
            std::is_same<typename std::iterator_traits<It>::value_type, 
                         typename std::map<Key, Value>::value_type>::value, 
            int>::type = 0, uint32_t size = 0)
    {
        this->m_table.reserve(size);
        while (Begin != End) {
            this->m_table.emplace_back(Begin->first, Begin->second);
            ++Begin;
        }
    }

    Value* lookup(const Key& key) const override
    {
        auto it = std::lower_bound(this->m_table.begin(), this->m_table.end(), key,
                                   [](const std::pair<Key, Value>& element, const Key &value)
                                   {
                                       return element.first < value;
                                   });

        if (it != this->m_table.end() && it->first == key)
        {
            return &it->second;
        }

        return nullptr;
    }

    std::shared_ptr<base_lookup_table<Key, Value>> clone() const override
    {
        return std::make_shared<lookup_treetable<Key, Value>>(*this);
    }
};
} // namespace gp_private


// Interface for lookup tables (Insertions are not allowed)
template <typename Key, typename Value>
class lookup_table
{
protected:
    std::shared_ptr<gp_private::base_lookup_table<Key, Value>> m_table;

public:
    
    template <typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<const Key, Value>>>
    explicit lookup_table(const std::unordered_map<Key, Value, Hash, KeyEqual, Allocator>& data)
    {
        m_table = std::make_shared<gp_private::lookup_hashtable<Key, Value, Hash, KeyEqual, Allocator>>(data);
    }

    template <typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<const Key, Value>>>
    explicit lookup_table(std::unordered_map<Key, Value, Hash, KeyEqual, Allocator>&& data)
    {
        m_table = std::make_shared<gp_private::lookup_hashtable<Key, Value, Hash, KeyEqual, Allocator>>(std::move(data));
    }

    template <typename It>
    explicit lookup_table(It Begin, It End, 
        typename std::enable_if<
            std::is_same<typename std::iterator_traits<It>::value_type, 
                         typename std::unordered_map<Key, Value>::value_type>::value, 
            int>::type = 0, uint32_t size = 0)
    {
        m_table = std::make_shared<gp_private::lookup_hashtable<Key, Value>>(Begin, End, size);
    }
    
    template <typename Compare = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, Value>>>
    explicit lookup_table(const std::map<Key, Value, Compare, Allocator>& data)
    {
        m_table = std::make_shared<gp_private::lookup_treetable<Key, Value>>(data);
    }

    template <typename Compare = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, Value>>>
    explicit lookup_table(std::map<Key, Value, Compare, Allocator>&& data)
    {
        m_table = std::make_shared<gp_private::lookup_treetable<Key, Value>>(std::move(data));
    }

    template <typename Compare = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, Value>>, typename It>
    explicit lookup_table(It Begin, It End, 
        typename std::enable_if<
            std::is_same<typename std::iterator_traits<It>::value_type, 
                         typename std::map<Key, Value>::value_type>::value, 
            int>::type = 0, uint32_t size = 0)
    {
        m_table = std::make_shared<gp_private::lookup_treetable<Key, Value>>(Begin, End, size);
    }

    Value* lookup(const Key& key) const
    {
        return m_table->lookup(key);
    }

    Value* operator[](const Key& key) const
    {
        Value* value = lookup(key);
        if(value) return value;
        else printf("Invalid Access !! Key Not Found in Looktable");
        return nullptr;
    }

    std::pair<const Key&, Value&>* begin() 
    { return m_table->begin(); }

    std::pair<const Key&, Value&>* end() 
    { return m_table->end();   }

    std::shared_ptr<gp_private::base_lookup_table<Key, Value>> clone() const
    {
        return m_table->clone();
    }

    bool empty() const
    {
        return m_table->empty();
    }

    size_t size() const
    {
        return m_table->size();
    }

    bool operator==(const lookup_table<Key, Value>& other) const
    {
        return *m_table == *other.m_table;
    }

    bool operator!=(const lookup_table<Key, Value>& other) const
    {
        return !(*this == other);
    }

    bool is_tree_table() const
    {
        return m_table->is_tree_table();
    }

    bool is_hash_table() const
    {
        return m_table->is_hash_table();
    }

    bool is_equal(const lookup_table<Key, Value>& other) const
    {
        return *m_table == *other.m_table;
    }
};
} // namespace gp_std
#endif 
