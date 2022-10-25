#ifndef UNDERSCORE_CPP_REDUCIBLE_HPP
#define UNDERSCORE_CPP_REDUCIBLE_HPP

#include "interfaces.hpp"
#include "tmf.hpp"
#include <functional>

namespace fff {

    template<typename F, typename Arg_1, typename Arg_2, typename ...Args>
    struct reducible_TD;

    template<typename F, typename Arg_1, typename Arg_2>
    struct reducible_TD<F, Arg_1, Arg_2> {
        using type = std::invoke_result_t<F, Arg_1, Arg_2>;

        constexpr static bool nothrow = std::is_nothrow_invocable_v<F, Arg_1, Arg_2>;
    };

    template<typename F, typename Arg_1, typename Arg_2, typename ...Args>
    struct reducible_TD {
        using type = typename reducible_TD<
            F, std::invoke_result_t<F, Arg_1, Arg_2>, Args...
        >::type;
    };

    template<typename F>
    class reducible_f : public callable_i<F, reducible_f<F>, reducible_TD> {

        friend callable_i<F, reducible_f<F>, reducible_TD>;
        friend class reducible_factory;

        [[no_unique_address]] F f;

        constexpr explicit reducible_f(const F &f) noexcept : f(f) {}
        constexpr explicit reducible_f(F &&f) noexcept : f(std::move(f)) {}

        template<typename Self, typename T1, typename T2>
        constexpr static auto call_impl(Self &&self, T1 &&t1, T2 &&t2)
            noexcept(std::is_nothrow_invocable_v<F, T1, T2>)
                -> std::invoke_result_t<F, T1, T2>
        {
            return std::invoke(self.f,
                               std::forward<T1>(t1),
                               std::forward<T2>(t2));
        }

        // @todo Determine noexcept condition

        template<typename Self, typename T1, typename T2, typename ...Args>
        constexpr static auto call_impl(Self &&self, T1 &&t1, T2 &&t2, Args &&...args)
            noexcept
                -> typename reducible_TD<F, T1, T2, Args...>::type
        {
            return call_impl(self,
                             std::invoke(self.f,
                                         std::forward<T1>(t1),
                                         std::forward<T2>(t2)),
                             args...);
        }
    };

    struct reducible_factory {
        template<typename F>
        constexpr auto operator()(F &&f) const noexcept
            -> reducible_f<std::decay_t<F>>
        {
            return reducible_f<std::decay_t<F>>(std::forward<F>(f));
        }
    };

    constexpr inline reducible_factory reducible;
}

#endif//UNDERSCORE_CPP_REDUCIBLE_HPP
