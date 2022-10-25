#ifndef UNDERSCORE_CPP_REDUCIBLE_HPP
#define UNDERSCORE_CPP_REDUCIBLE_HPP

#include "interfaces.hpp"
#include "tmf.hpp"
#include <functional>

namespace fff::factory {
    class Reducible;
}

namespace fff {

    namespace liated {
        template<typename F, typename Arg_1, typename Arg_2, typename ...Args>
        struct Reducible_TD;

        template<typename F, typename Arg_1, typename Arg_2>
        struct Reducible_TD<F, Arg_1, Arg_2> {
            using type = std::invoke_result_t<F, Arg_1, Arg_2>;

            constexpr static bool nothrow = std::is_nothrow_invocable_v<F, Arg_1, Arg_2>;
        };

        template<typename F, typename Arg_1, typename Arg_2, typename ...Args>
        struct Reducible_TD {
            using type = typename Reducible_TD<
                F, std::invoke_result_t<F, Arg_1, Arg_2>, Args...
                >::type;
        };
    }

    template<typename F>
    class Reducible_f : public callable_i<F, Reducible_f<F>, liated::Reducible_TD> {

        friend callable_i<F, Reducible_f<F>, liated::Reducible_TD>;
        friend factory::Reducible;

        [[no_unique_address]] F f;

        constexpr explicit Reducible_f(const F &f) noexcept : f(f) {}
        constexpr explicit Reducible_f(F &&f) noexcept : f(std::move(f)) {}

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
                -> typename liated::Reducible_TD<F, T1, T2, Args...>::type
        {
            return call_impl(self,
                             std::invoke(self.f,
                                         std::forward<T1>(t1),
                                         std::forward<T2>(t2)),
                             args...);
        }
    };

    namespace factory {
        struct Reducible {
            template<typename F>
            constexpr auto operator()(F &&f) const noexcept
                -> Reducible_f<std::decay_t<F>>
            {
                return Reducible_f<std::decay_t<F>>(std::forward<F>(f));
            }
        };
    }

    constexpr inline factory::Reducible reducible;
}

#endif//UNDERSCORE_CPP_REDUCIBLE_HPP
