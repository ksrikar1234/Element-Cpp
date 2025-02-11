#ifndef GP_STREAM_API_HPP
#define GP_STREAM_API_HPP

#include <stdio.h> 
#include <cassert>

#include <memory>
#include <vector>

#include <algorithm>
#include <numeric>

#include <thread>
#include <future>

#include "../function/gp_function_ref.hpp"

namespace gp_std
{
// int main()
// { 

// -----> Create a gp_stream with integers using std::vector
//     std::vector<int> data;
//     gp_stream<int> stream(data);

/// ----> Create the function_ref objects

//     auto print = [](int& x) { std::cout << x <<"\n"; };
//     auto is_even([](const int &x) -> bool  { return x % 2 == 0; });
//     auto square          = [](int &x)                              { x *= x; };
//     auto add             = [](const int &x1, const int &x2) -> int { return x1 + x2; };
//     auto multiply_by_ten = [](int &x) { x *= 10; };
//     std::function<void(int&)> add_one = [](const int& x) { return x + 1 ; };


// ---->   Apply the functions to the stream
//     int filtered_data = stream.
//                         transform(multiply_by_ten).
//                         filter(is_even).
//                         transform(square).
//                         for_each(print).
//                         map(add_one).
//                         parallel_reduce(add, 0);


//     printf("\nFiltered : %d\n", filtered_data);                       
//     printf("2");                     
//     return 0;


/// @brief Stream API for C++ similar to Java Stream API
/// @tparam T Type of the elements in the Stream
/// @tparam Container Container type for the Stream (By default it is std::vector)
/// @memberof gp_std
template <typename T, template <typename, typename> class Container = std::vector, typename Allocator = std::allocator<T>>
class Stream
{
public:
    using ContainerType = Container<T, Allocator>;

    using Value_Type = T;

    using Index_Type = const int&;

    using Stream_Type = Stream<T, Container, Allocator>;

    // +-----------------------------------------------------------------------------------------------------------------+
    // | Function Signatures for Stream Operations                                                                       |
    // +-----------------------------------------------------------------------------------------------------------------+

    // For Filter Operation
    using Predicate = function_ref<bool(const T&)>;  // (Promising no modification of the stream elements)
  
    // For Reduce Operation
    using Accumulator = function_ref<T(const T&, const T&)>;  // Promising no modification of the stream elements

    // For Map Operation 
    using Mapper = function_ref<T(const T&)>;  // (Promising no modification of the stream elements)

    // For TypeMap Operation
    template <typename NewType>
    using TypeMapper = function_ref<NewType(const T&)>;  // (Promising no modification of the stream elements)

    // For Transform Operation
    using Transformer = function_ref<void(T&)>;  // (Mostly for modifying the stream elements)

    // For ForEach Operation 
    using Action = function_ref<void(T&)>;                  
    using ConstAction = function_ref<void(const T&)>;  // (Promising no modification)    
    
    // For For in Range Operation
    using RangeAction = function_ref<void(T&, Index_Type)>;
    using ConstRangeAction = function_ref<void(const T&, Index_Type)>;  // (Promising no modification)

    // +-----------------------------------------------------------------------------------------------------------------+
    // | Constructors                                                                                                    |
    // +-----------------------------------------------------------------------------------------------------------------+
    
    Stream() = default;

    template <typename InputIt>
    Stream(InputIt first, InputIt last) : m_data(first, last) {}
    
    Stream(const Stream_Type &other) : m_data(other.m_data) {}

    Stream(Stream_Type &&other) : m_data(std::move(other.m_data)) {}

    Stream(const ContainerType &data) : m_data(data)       {}
    Stream(ContainerType &&data) : m_data(std::move(data)) {}
    Stream(std::initializer_list<T> init) : m_data(init)   {}
    
    ~Stream()
    {
        for(auto& exception : m_exceptions)
        {
           printf("Exception caught in Stream : %s\n", exception.c_str());
        }
    }


    Stream& operator=(const Stream_Type &other)
    {
        m_data = other.m_data;
        return *this;
    }

    Stream& operator=(Stream_Type &&other)
    {
        m_data = std::move(other.m_data);
        m_exceptions = std::move(other.m_exceptions);
        return *this;
    }

    Stream_Type& concat(const Stream_Type& other)
    {
        m_data.insert(m_data.end(), other.m_data.begin(), other.m_data.end());
        return *this;
    }

    Stream_Type& concat(Stream_Type&& other)
    {
        m_data.insert(m_data.end(), std::make_move_iterator(other.m_data.begin()), std::make_move_iterator(other.m_data.end()));
        return *this;
    }

    template <typename InputIt>
    Stream_Type& concat(InputIt first, InputIt last)
    {
        m_data.insert(m_data.end(), first, last);
        return *this;
    }

    template <typename InputMoveIt>
    void move_from(InputMoveIt first, InputMoveIt last)
    {
        m_data.clear();
        std::move(first, last, std::back_inserter(m_data));
    }

    // Collect: collects the elements into a standard base container of the Stream (By default it is std::vector)
    ContainerType collect() const
    {
        return m_data;
    }

    T& operator[](Index_Type index) { return m_data[index]; }
    const T& operator[](Index_Type index) const { return m_data[index]; }

    size_t size() const { return m_data.size(); }   

    ///+-----------------------------------------------------------------------------------------------------------------+
    ///| Stream Operations                                                                                               |
    ///+-----------------------------------------------------------------------------------------------------------------+
    
    void broadcast(const T& value)
    {
        for (auto& elem : m_data) { elem = value;}
    }

    void parallel_broadcast(const T& value)
    {
        int num_threads = std::thread::hardware_concurrency();
        int work_per_thread = m_data.size() / num_threads;
        std::vector<std::future<void>> futures;

        for (int i = 0; i < num_threads; ++i)
        {
            futures.emplace_back(std::async(std::launch::async, [&, i]()
            {
                int start = i * work_per_thread;
                int end = (i == num_threads - 1) ? m_data.size() : start + work_per_thread;
                for (int j = start; j < end; ++j)
                {
                    try
                    {
                        m_data[j] = value;
                    }
                    catch (const std::exception& e)
                    {
                        printf("Exception caught in Stream.parallel_broadcast: %s\n", e.what());
                    }
                    catch (...)
                    {
                        printf("Unknown exception caught in Stream.parallel_broadcast\n");
                    }
                }
            }));
        }

        for (auto& future : futures)
        {
            future.wait();
        }
    }

    // Filter: retains only elements that satisfy the predicate
    // Returns a new Stream
    Stream_Type filter(const Predicate& predicate) const
    {
        ContainerType filtered;
        std::copy_if(m_data.begin(), m_data.end(), std::back_inserter(filtered), predicate);
        return Stream_Type(std::move(filtered));
    }

    // Parallel Filter: retains only elements that satisfy the predicate in parallel
    // Returns a new Stream
    Stream_Type parallel_filter(const Predicate& predicate) const
    {
        ContainerType filtered;
        int num_threads = std::thread::hardware_concurrency();
        int work_per_thread = m_data.size() / num_threads;
        std::vector<std::future<void>> futures;

        for (int i = 0; i < num_threads; ++i)
        {
            futures.emplace_back(std::async(std::launch::async, [&, i]()
            {
                int start = i * work_per_thread;
                int end = (i == num_threads - 1) ? m_data.size() : start + work_per_thread;
                for (int j = start; j < end; ++j)
                {
                    try
                    {
                        if (predicate(m_data[j]))
                        {
                            filtered.push_back(m_data[j]);
                        }
                    }
                    catch (const std::exception& e)
                    {
                        printf("Exception caught in Stream.parallel_filter: %s\n", e.what());
                    }
                    catch (...)
                    {
                        printf("Unknown exception caught in Stream.parallel_filter\n");
                    }
                }
            }));
        }

        for (auto& future : futures)
        {
            future.wait();
        }

        return Stream_Type(std::move(filtered));
    }

    // Map: applies a function to all elements and returns a new Stream
    // Returns a new Stream
    Stream_Type map(const Mapper& mapper) const
    {
        Container<T, Allocator> mapped(m_data.size());
        auto it = mapped.begin();
        for (const auto &elem : m_data)
        {
            *it++ = mapper(elem);
        }
        return Stream_Type(std::move(mapped));
    }

    // Parallel Map: applies a function to all elements and returns a new Stream in parallel
    // Returns a new Stream
    Stream_Type parallel_map(const Mapper& mapper) const
    {
        Container<T, Allocator> mapped(m_data.size());
        auto it = mapped.begin();
        int num_threads = std::thread::hardware_concurrency();
        int work_per_thread = m_data.size() / num_threads;
        std::vector<std::future<void>> futures;

        for (int i = 0; i < num_threads; ++i)
        {
            futures.emplace_back(std::async(std::launch::async, [&, i]()
            {
                int start = i * work_per_thread;
                int end = (i == num_threads - 1) ? m_data.size() : start + work_per_thread;
                for (int j = start; j < end; ++j)
                {
                    try
                    {
                        *it++ = mapper(m_data[j]);
                    }
                    catch (const std::exception& e)
                    {
                        printf("Exception caught in Stream.parallel_map: %s\n", e.what());
                    }
                    catch (...)
                    {
                        printf("Unknown exception caught in Stream.parallel_map\n");
                    }
                }
            }));
        }

        for (auto& future : futures)
        {
            future.wait();
        }

        return Stream_Type(std::move(mapped));
    }


    /// @brief Map Type: applies a function to all elements and returns a new Stream with a different type
    /// @returns Returns a new Stream
    /// @note Example: Stream<int> -> map_type<std::string>([](const int& x) { return std::to_string(x); });
    /// @note The above example will return a Stream<std::string>
    template <typename NewType>
    Stream<NewType, Container, Allocator> map_to_type(const TypeMapper<NewType>& mapper) const
    {
        Container<NewType, Allocator> mapped(m_data.size());
        auto it = mapped.begin();
        for (const auto &elem : m_data)
        {
            *it++ = mapper(elem);
        }
        return Stream<NewType, Container, Allocator>(std::move(mapped));
    }

    /// @brief Parallel Map Type: applies a function to all elements and returns a new Stream with a different type in parallel
    /// @returns Returns a new Stream
    /// @note Example: Stream<int> -> parallel_map_type<std::string>([](const int& x) { return std::to_string(x); });
    /// @note The above example will return a Stream<std::string>
    template <typename NewType>
    Stream<NewType, Container, Allocator> parallel_map_to_type(const TypeMapper<NewType>& mapper) const
    {
        Container<NewType, Allocator> mapped(m_data.size());
        auto it = mapped.begin();
        int num_threads = std::thread::hardware_concurrency();
        int work_per_thread = m_data.size() / num_threads;
        std::vector<std::future<void>> futures;

        for (int i = 0; i < num_threads; ++i)
        {
            futures.emplace_back(std::async(std::launch::async, [&, i]()
            {
                int start = i * work_per_thread;
                int end = (i == num_threads - 1) ? m_data.size() : start + work_per_thread;
                for (int j = start; j < end; ++j)
                {
                    try
                    {
                        *it++ = mapper(m_data[j]);
                    }
                    catch (const std::exception& e)
                    {
                        printf("Exception caught in Stream.parallel_map_to_type: %s\n", e.what());
                    }
                    catch (...)
                    {
                        printf("Unknown exception caught in Stream.parallel_map_to_type\n");
                    }
                }
            }));
        }

        for (auto& future : futures)
        {
            future.wait();
        }

        return Stream<NewType, Container, Allocator>(std::move(mapped));
    }

    // Transform: modifies the elements in place
    Stream_Type& transform(const Transformer& transformer)
    {
        for (auto &elem : m_data)
        {
            transformer(elem);
        }
        return *this;
    }

    // Parallel Transform: modifies the elements in place in parallel
    Stream_Type& parallel_transform(const Transformer& transformer)
    {
        int num_threads = std::thread::hardware_concurrency();
        int work_per_thread = m_data.size() / num_threads;
        std::vector<std::future<void>> futures;

        for (int i = 0; i < num_threads; ++i)
        {
            futures.emplace_back(std::async(std::launch::async, [&, i]()
            {
                int start = i * work_per_thread;
                int end = (i == num_threads - 1) ? m_data.size() : start + work_per_thread;
                for (int j = start; j < end; ++j)
                {
                    try
                    {
                        transformer(m_data[j]);
                    }
                    catch (const std::exception& e)
                    {
                        printf("Exception caught in Stream.parallel_transform: %s\n", e.what());
                    }
                    catch (...)
                    {
                        printf("Unknown exception caught in Stream.parallel_transform\n");
                    }
                }
            }));
        }

        for (auto& future : futures)
        {
            future.wait();
        }

        return *this;
    }

    // Reduce: reduces the elements to a single value using an accumulator function
    T reduce(const Accumulator& accumulator, const T& identity) const
    {
        return std::accumulate(m_data.begin(), m_data.end(), identity, accumulator);
    }

    T parallel_reduce(const Accumulator& accumulator, const T& identity) const
    {
        int num_threads = std::thread::hardware_concurrency();
        int work_per_thread = m_data.size() / num_threads;
        std::vector<std::future<T>> futures;

        for (int i = 0; i < num_threads; ++i)
        {
            futures.emplace_back(std::async(std::launch::async, [&, i]()
            {
                int start = i * work_per_thread;
                int end = (i == num_threads - 1) ? m_data.size() : start + work_per_thread;
                T result = identity;
                for (int j = start; j < end; ++j)
                {
                    try
                    {
                        result = accumulator(result, m_data[j]);
                    }
                    catch (const std::exception& e)
                    {
                        printf("Exception caught in Stream.parallel_reduce: %s\n", e.what());
                    }
                    catch (...)
                    {
                        printf("Unknown exception caught in Stream.parallel_reduce\n");
                    }
                }
                return result;
            }));
        }

        T result = identity;
        for (auto& future : futures)
        {
            result = accumulator(result, future.get());
        }

        return result;
    }
    
    /// @note ForEach: applies a function to each element (Might modify)
    /// @note This function is not const as it might modify the elements
    Stream_Type& for_each(const Action& action) 
    {
        for (auto& elem : m_data) { action(elem); }
        return *this;
    }

    // ForEach: applies a function to each element (Promising no modification)
    Stream_Type for_each(const ConstAction& action) const
    {
        for (const auto &elem : m_data)
        {
            action(elem);
        }
        return *this;
    }

    Stream_Type& for_in_range(const RangeAction& action)
    {
        int index = 0;
        for (auto& elem : m_data)
        {
            action(elem, index++);
        }
        return *this;
    }
    
    Stream_Type for_in_range(const ConstRangeAction& action) const
    {
        int index = 0;
        for (const auto &elem : m_data)
        {
            action(elem, index++);
        }
        return *this;
    }
private:
    ContainerType m_data;
    std::vector<std::string> m_exceptions;
};

} // namespace gp_std

template <typename T, template <typename, typename> class Container = std::vector, typename Allocator = std::allocator<T>>
using gp_stream = gp_std::Stream<T, Container, Allocator>;  

#endif
