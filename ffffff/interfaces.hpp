#ifndef UNDERSCORE_CPP_INTERFACES_HPP
#define UNDERSCORE_CPP_INTERFACES_HPP

#include <utility>

namespace fff::factory {
    // declaration
}

namespace fff {

    template<class, class...>
    struct auto_decl {};

    /**
     * A CRTP Pattern that gives "operator()" function.\n
     * To implement "operator()" function, it is sufficient to implement a static 'call_impl' template function.
     * @example template\<class F> class Foo : callable_i\<F, Foo\<F>, std::invoke_result> { (implements...) }
     * @tparam F function argument type of the derived type, Derived::function_type
     * @tparam Derived Self
     * @tparam TypeDeduction (optional) A TMP guideline that tells compilers about the return type
     */
    template<typename F, typename Derived,
             template<class, class...> class TypeDeduction = auto_decl>
    class callable_i {
    public:
        using function_type = F;

        template<typename ...Args>
        constexpr auto operator()(Args &&...args) const &
            noexcept(noexcept(Derived::call_impl(*static_cast<Derived*>(this), std::forward<Args>(args)...)))
                -> typename TypeDeduction<F, Args...>::type
        {
            return Derived::call_impl(*static_cast<Derived*>(this), std::forward<Args>(args)...);
        }

        template<typename ...Args>
        constexpr auto operator()(Args &&...args) &&
            noexcept(noexcept(Derived::call_impl(std::move(*static_cast<Derived*>(this)), std::forward<Args>(args)...)))
                -> typename TypeDeduction<F, Args...>::type
        {
            return Derived::call_impl(std::move(*static_cast<Derived*>(this)), std::forward<Args>(args)...);
        }

        template<typename ...Args>
        constexpr auto operator()(Args &&...args) const &&
            noexcept(noexcept(Derived::call_impl(std::move(*static_cast<Derived*>(this)), std::forward<Args>(args)...)))
                -> typename TypeDeduction<F, Args...>::type
        {
            return Derived::call_impl(std::move(*static_cast<Derived*>(this)), std::forward<Args>(args)...);
        }

        template<typename ...Args>
        constexpr auto operator()(Args &&...args) &
            noexcept(noexcept(Derived::call_impl(*static_cast<Derived*>(this), std::forward<Args>(args)...)))
                -> typename TypeDeduction<F, Args...>::type
        {
            return Derived::call_impl(*static_cast<Derived*>(this), std::forward<Args>(args)...);
        }
    };

    template<typename F, typename Derived>
    class callable_i<F, Derived, auto_decl> {
    public:
        using function_type = F;

        template<typename ...Args>
        constexpr auto operator()(Args &&...args) &
            noexcept(noexcept(Derived::call_impl(*static_cast<Derived*>(this), std::forward<Args>(args)...)))
        {
            return Derived::call_impl(*static_cast<Derived*>(this), std::forward<Args>(args)...);
        }

        template<typename ...Args>
        constexpr auto operator()(Args &&...args) const &
            noexcept(noexcept(Derived::call_impl(*static_cast<Derived*>(this), std::forward<Args>(args)...)))
        {
            return Derived::call_impl(*static_cast<Derived*>(this), std::forward<Args>(args)...);
        }

        template<typename ...Args>
        constexpr auto operator()(Args &&...args) &&
            noexcept(noexcept(Derived::call_impl(std::move(*static_cast<Derived*>(this)), std::forward<Args>(args)...)))
        {
            return Derived::call_impl(std::move(*static_cast<Derived*>(this)), std::forward<Args>(args)...);
        }

        template<typename ...Args>
        constexpr auto operator()(Args &&...args) const &&
            noexcept(noexcept(Derived::call_impl(std::move(*static_cast<Derived*>(this)), std::forward<Args>(args)...)))
        {
            return Derived::call_impl(std::move(*static_cast<Derived*>(this)), std::forward<Args>(args)...);
        }
    };

    /**
    * An interface that offers lift() method. \n
    * To use this, the derived class should implement lift_impl() method.
    * @tparam T value-type
    * @tparam C a type constructor that will offer lift() method
    */
    template<typename T, template<class, class...> class C>
    class Lift_i {
    public:
        using Derived = C<T>;
        using value_type = T;

        template<std::invocable<T> F>
        constexpr auto lift(F &&f) &
            noexcept(noexcept(Derived::lift_impl(*static_cast<Derived*>(this), std::forward<F>(f))))
                -> C<std::invoke_result_t<F, T>>
        {
            return Derived::lift_impl(*static_cast<Derived*>(this), std::forward<F>(f));
        }

        template<std::invocable<T> F>
        constexpr auto lift(F &&f) const &
            noexcept(noexcept(Derived::lift_impl(*static_cast<Derived*>(this), std::forward<F>(f))))
                -> C<std::invoke_result_t<F, T>>
        {
            return Derived::lift_impl(*static_cast<Derived*>(this), std::forward<F>(f));
        }

        template<std::invocable<T> F>
        constexpr auto lift(F &&f) &&
            noexcept(noexcept(Derived::lift_impl(std::move(*static_cast<Derived*>(this)), std::forward<F>(f))))
                -> C<std::invoke_result_t<F, T>>
        {
            return Derived::lift_impl(std::move(*static_cast<Derived*>(this)), std::forward<F>(f));
        }

        template<std::invocable<T> F>
        constexpr auto lift(F &&f) const &&
            noexcept(noexcept(Derived::lift_impl(std::move(*static_cast<Derived*>(this)), std::forward<F>(f))))
                -> C<std::invoke_result_t<F, T>>
        {
            return Derived::lift_impl(std::move(*static_cast<Derived*>(this)), std::forward<F>(f));
        }
    };
};

#endif//UNDERSCORE_CPP_INTERFACES_HPP
