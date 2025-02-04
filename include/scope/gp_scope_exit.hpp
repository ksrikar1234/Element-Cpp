#ifndef __GP_SCOPE_EXIT_HPP
#define __GP_SCOPE_EXIT_HPP

#include <utility>

namespace gp_std
{
    template <typename Callable>
    class scope_exit
    {
    public:
        explicit scope_exit(Callable &&func)
            : m_func(std::forward<Callable>(func)), m_active(true) {}

        ~scope_exit()
        {
            if (m_active)
            {
                m_func();
            }
        }

        // Disable copy semantics to prevent multiple calls
        scope_exit(const scope_exit &) = delete;
        scope_exit &operator=(const scope_exit &) = delete;

        // Move constructor to transfer ownership
        scope_exit(scope_exit &&other) noexcept
            : m_func(std::move(other.m_func)), m_active(other.m_active)
        {
            other.m_active = false;
        }

        // Move assignment to transfer ownership
        scope_exit &operator=(scope_exit&& other) noexcept
        {
            if (this != &other)
            {
                if (m_active)
                {
                    m_func();
                }
                m_func = std::move(other.m_func);
                m_active = other.m_active;
                other.m_active = false;
            }
            return *this;
        }

        // Release the function so it is not called on destruction
        void dismiss() { m_active = false; }

    private:
        Callable m_func;
        bool m_active;
    };

    template <typename Callable>
    scope_exit<Callable> make_scope_guard(Callable &&func)
    {
        return scope_exit<Callable>(std::forward<Callable>(func));
    }

} // namespace gp_std

#endif // __GP_SCOPE_EXIT_HPP
