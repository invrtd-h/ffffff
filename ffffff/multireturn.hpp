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


namespace fff {

    namespace liated {

        template<typename F, typename T>
        struct call_TD {
            static_assert(true, "The second type must be a tuple");
        };

        template<typename F, typename ...Ts>
        struct call_TD<F, std::tuple<Ts...>> {

            using call_result = std::invoke_result_t<F, Ts...>;
            constexpr static bool is_nothrow = std::is_nothrow_invocable_v<F, Ts...>;
        };
    }

    template<typename F, made_by<std::tuple> Tuple>
    using call_result = typename liated::call_TD<F, Tuple>::call_result;

    template<typename F, made_by<std::tuple> Tuple>
    constexpr inline bool is_nothrow_call_v = liated::call_TD<F, Tuple>::is_nothrow;

    static_assert(std::is_same_v<int, call_result<int(int, int), std::tuple<int, int>>>);

    struct call_f {

    private:
        template<typename F, made_by<std::tuple> Tuple, std::size_t ...I>
        constexpr auto impl(F &&f, Tuple &&tuple, std::index_sequence<I...>) const
            noexcept(is_nothrow_call_v<std::decay_t<F>, std::decay_t<Tuple>>)
                -> call_result<std::decay_t<F>, std::decay_t<Tuple>>
        {
            return std::invoke(std::forward<F>(f), std::get<I>(std::forward<Tuple>(tuple))...);
        }

    public:
        template<typename F, made_by<std::tuple> Tuple>
        constexpr auto operator()(F &&f, Tuple &&tuple) const
            noexcept(is_nothrow_call_v<std::decay_t<F>, std::decay_t<Tuple>>)
                -> call_result<std::decay_t<F>, std::decay_t<Tuple>>
        {
            return impl(std::forward<F>(f), std::forward<Tuple>(tuple),
                std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>());
        }
    };

    constexpr inline call_f call;



    namespace liated {

        template<typename T>
        struct tuple_cat_type_impl {
            static_assert(true, "The first type must be a tuple");
        };

        template<typename ...Args>
        struct tuple_cat_type_impl<std::tuple<Args...>> {

            template<typename ...Cats>
            using type = std::tuple<Args..., Cats...>;
        };
    }

    template<typename T, typename ...Args>
    using tuple_cat_type_result = typename liated::tuple_cat_type_impl<T>::template type<Args...>;

    struct l_call_r_invoke_f {
        template<typename F, made_by<std::tuple> Tuple, typename ...Args>
        constexpr auto operator()(F &&f, Tuple &&tuple, Args &&...args) const
            noexcept(is_nothrow_call_v<F, tuple_cat_type_result<Tuple, Args...>>)
                -> call_result<F, tuple_cat_type_result<Tuple, Args...>>
        {
            return call(std::forward<F>(f),
                        std::tuple_cat(std::forward<Tuple>(tuple),
                                       std::forward_as_tuple(std::forward<Args>(args)...)));
        }
    };

    constexpr inline l_call_r_invoke_f l_call_r_invoke;
}

#endif//UNDERSCORE_CPP_MULTIRETURN_HPP
