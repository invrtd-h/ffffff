#ifndef UNDERSCORE_CPP_MULTIRETURN_HPP
#define UNDERSCORE_CPP_MULTIRETURN_HPP

#include <tuple>

#include "tmf.hpp"

namespace fff {
    template<typename ...Ts>
    struct MultiReturn : public std::tuple<Ts...> {
        using std::tuple<Ts...>::tuple;

        constexpr auto to_tuple() const noexcept -> std::tuple<Ts...> {
            return static_cast<std::tuple<Ts...>>(*this);
        }

        template<std::invocable<Ts...> F>
        constexpr auto operator>>(F &&f) const
            noexcept(noexcept(std::apply(std::forward<F>(f), this->to_tuple())))
                -> std::invoke_result_t<F, Ts...>
        {
            return std::apply(std::forward<F>(f), this->to_tuple());
        }
    };

    struct MultiReturnFactory {
        template<typename ...Ts>
        constexpr auto operator()(Ts &&...ts) const noexcept
            -> MultiReturn<std::decay_t<Ts>...>
        {
            return MultiReturn<std::decay_t<Ts>...>{std::forward<Ts>(ts)...};
        }
    };

    template<typename T>
    concept mr = made_by<T, MultiReturn>;

    template<typename T>
    concept not_mr = not mr<T>;

    struct DeclMr {
        template<typename F, typename ...Ts>
        constexpr static auto declmr(F &&f, const MultiReturn<Ts...> &) noexcept
            -> std::invoke_result_t<F, Ts...>;
    };

    template<typename F, typename T>
    concept appliable = mr<T> and
        requires (F f, T t) {
            std::apply(f, t.to_tuple());
        };

    template<typename F, mr T>
    using apply_as_mr_result_t =
        decltype(DeclMr::declmr(std::declval<F>(), std::declval<T>()));

    constexpr inline MultiReturnFactory multi_return;
}

#endif//UNDERSCORE_CPP_MULTIRETURN_HPP
