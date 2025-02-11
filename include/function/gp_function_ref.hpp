
#ifndef GP_FUNCTION_REF_HPP
#define GP_FUNCTION_REF_HPP

#include <type_traits>
#include <cassert>
#include <utility>


namespace gp_std 
{
/// @brief A wrapper to hold any function object as reference complying with a specific signature
/// @tparam Signature The function signature of the function_ref
template <typename Signature>
class function_ref;

/// Specialization for function_ref objects with a specific signature
/// e.g., bool(const T&)
template <typename ReturnType, typename... Args>
class function_ref<ReturnType(Args...)>
{
public:    
    function_ref() : m_callable(nullptr), m_invoke(nullptr) {}
    
    // Construct from any function_ref
    template <typename Callable>
    function_ref(Callable&& func) : m_callable(static_cast<void*>(&func)), m_invoke(&invoke_impl<typename std::remove_reference<Callable>::type>)
    {
    }

    // Copy constructor
    function_ref(const function_ref& other) : m_callable(other.m_callable), m_invoke(other.m_invoke) {}

    // Move constructor
    function_ref(function_ref&& other) : m_callable(other.m_callable), m_invoke(other.m_invoke)
    {
        other.m_callable = nullptr;
        other.m_invoke   = nullptr;
    }

    // Copy assignment
    function_ref& operator=(const function_ref& other)
    {
        m_callable = other.m_callable;
        m_invoke   = other.m_invoke;
        return *this;
    }

    template<typename Callable>
    function_ref& operator=(const Callable& func)
    {
        m_callable = static_cast<void*>(&func);
        m_invoke = &invoke_impl<typename std::remove_reference<Callable>::type>;
        return *this;
    }

    // Callable operator
    ReturnType operator()(Args... args) const
    {
        if(m_invoke == nullptr)
        { assert(false && "Callable object is not initialized"); }
        if(m_callable == nullptr) { assert(false  && "m_callable is nullptr"); }
        return m_invoke(m_callable, std::forward<Args>(args)...);
    }
    
    ~function_ref() = default;
 
private:
    void* m_callable;
    ReturnType (*m_invoke)(void*, Args...);

    template <typename Callable>
    static ReturnType invoke_impl(void* function_ref, Args... args)
    {
        return (*static_cast<Callable*>(function_ref))(std::forward<Args>(args)...);
    }  
};
}// namespace gp_std
#endif // GP_CALLABLE_REF_HPP
