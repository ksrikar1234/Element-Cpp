#ifndef _GP_SCOPEGUARD_H_
#define _GP_SCOPEGUARD_H_

#include <atomic>
#include <functional>
#include <type_traits>
#include <chrono>
#include <string>
#include <vector>
#include <tuple>

namespace gp_std
{
    template <typename T = std::function<void()>>
    class scope_guard
    {
        public:
        explicit scope_guard(T &&func) noexcept : func(std::move(func)), active(true) {}
        scope_guard(const scope_guard &) = delete;
        scope_guard &operator=(const scope_guard &) = delete;
        scope_guard(scope_guard &&other) noexcept : func(std::move(other.func)), active(other.active.load()) { other.dismiss(); }
        scope_guard &operator=(scope_guard &&other) noexcept
        {
            if (this != &other)
            {
                func = std::move(other.func);
                active.store(other.active.load());
                other.dismiss();
            }
            return *this;
        }

       ~scope_guard() { if (active.load()) func(); }

        void dismiss() { active.store(false); }

        private:
        T func;
        std::atomic<bool> active;
    };


    template <typename... Locks>
    class scoped_lock
    {
    public:
        explicit scoped_lock(Locks &...locks) noexcept
            : locks(locks...)
        {
            lock();
        }

        // Delete copy and move semantics
        scoped_lock(const scoped_lock &) = delete;
        scoped_lock &operator=(const scoped_lock &) = delete;
        scoped_lock(scoped_lock &&) = delete;
        scoped_lock &operator=(scoped_lock &&) = delete;

        ~scoped_lock()
        {
            unlock();
        }

    private:
        // Locks all the locks
        void lock()
        {
            for_each_in_tuple(locks, [](auto &lock)
                              { lock.lock(); });
        }

        // Unlocks all the locks
        void unlock()
        {
            for_each_in_tuple(locks, [](auto &lock)
                              { lock.unlock(); });
        }

        // Utility function to apply an operation to all locks
        template <typename Tuple, typename Func>
        void for_each_in_tuple(Tuple &t, Func func)
        {
            for_each_in_tuple_impl(t, func, std::integral_constant<std::size_t, 0>{});
        }

        // Recursive tuple iteration for C++11
        template <typename Tuple, typename Func, std::size_t Index>
        void for_each_in_tuple_impl(Tuple &t, Func func, std::integral_constant<std::size_t, Index>)
        {
            func(std::get<Index>(t));                                                          // Apply function to the current element
            for_each_in_tuple_impl(t, func, std::integral_constant<std::size_t, Index + 1>{}); // Recurse to the next element
        }

        // Base case to stop recursion
        template <typename Tuple, typename Func>
        void for_each_in_tuple_impl(Tuple &, Func, std::integral_constant<std::size_t, std::tuple_size<Tuple>::value>)
        {
            // End of recursion: do nothing
        }

        std::tuple<Locks &...> locks; // Store references to all locks
    };

} // namespace gp_std

#endif // _GP_SCOPEGUARD_H_
