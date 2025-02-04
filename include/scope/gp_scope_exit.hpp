#ifndef __GP_SCOPE_EXIT_HPP
#define __GP_SCOPE_EXIT_HPP

#include <utility>

namespace gp_std
{
    // Public API
    // Example To Create ScopeExit 
    // auto file = load_file("file"); 
    // auto file_closer = gp_std::scope_exit_guard([&file] { file.close });
    template <typename Callable>
    scope_exit_guard<Callable> scope_exit(Callable &&func);

    template <typename Callable>
    class scope_exit_guard
    {
    public:
        explicit scope_exit_guard(Callable &&func)
            : m_func(std::forward<Callable>(func)), m_active(true) {}

        ~scope_exit_guard()
        {
            if (m_active)
            {
                m_func();
            }
        }

        scope_exit_guard(const scope_exit_guard &) = delete;
        scope_exit_guard &operator=(const scope_exit_guard &) = delete;

        scope_exit_guard(scope_exit_guard &&other) noexcept
            : m_func(std::move(other.m_func)), m_active(other.m_active)
        {
            other.m_active = false;
        }

        scope_exit_guard &operator=(scope_exit_guard&& other) noexcept
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
    scope_exit_guard<Callable> scope_exit(Callable &&func)
    {
        return scope_exit_guard<Callable>(std::forward<Callable>(func));
    }

} // namespace gp_std

#endif // __GP_SCOPE_EXIT_HPP
