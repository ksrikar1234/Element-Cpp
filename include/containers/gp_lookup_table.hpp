#ifndef __GP_LOOKUP_TABLE_TYPES__
#define __GP_LOOKUP_TABLE_TYPES__
#include <memory>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <map>

namespace gp_std
{

template <typename Key, typename Value>
class lookup_table;

template <typename Key, typename Value, typename Hash, typename KeyEqual , typename Allocator>
class lookup_hashtable;

template <typename Key, typename Value, typename Compare , typename Allocator>
class lookup_treetable;

// Interface for lookup tables
template <typename Key, typename Value>
class lookup_table
{
protected:
    std::vector<std::pair<Key, Value>> m_table;

public:
    virtual ~lookup_table() = default;
    virtual Value* lookup(const Key& key) const = 0;
    virtual std::shared_ptr<lookup_table<Key, Value>> clone() const = 0;
};

// Hash-based lookup table
template <typename Key, typename Value, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<const Key, Value>>>
class lookup_hashtable : public lookup_table<Key, Value>
{
private:
    std::vector<size_t> m_hashes;

public:
    explicit lookup_hashtable(const std::unordered_map<Key, Value, Hash, KeyEqual, Allocator>& data)
    {
       import_data(data);
       sort_table();
    }

    explicit lookup_hashtable(std::unordered_map<Key, Value, Hash, KeyEqual, Allocator>&& data)
    {
        import_data(std::move(data));
        sort_table();
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

    std::shared_ptr<lookup_table<Key, Value>> clone() const override
    {
        return std::make_shared<lookup_hashtable<Key, Value, Hash, KeyEqual, Allocator>>(*this);
    }

    private:
    void import_data(std::unordered_map<Key, Value, Hash, KeyEqual, Allocator>&& data)
    {
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

    void sort_table()
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
            sorted_table[i] = this->m_table[indices[i]];
            sorted_hashes[i] = m_hashes[indices[i]];
        }

        this->m_table = std::move(sorted_table);
        m_hashes = std::move(sorted_hashes);
    }
};

// Tree-based lookup table
template <typename Key, typename Value, typename Compare = std::less<Key> , typename Allocator = std::allocator<std::pair<const Key, Value>>>
class lookup_treetable : public lookup_table<Key, Value>
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

    std::shared_ptr<lookup_table<Key, Value>> clone() const override
    {
        return std::make_shared<lookup_treetable<Key, Value>>(*this);
    }
};
} // namespace gp_std
#endif 
