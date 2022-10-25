#ifndef UNDERSCORE_CPP_MULTIARGS_HPP
#define UNDERSCORE_CPP_MULTIARGS_HPP

#include <tuple>

#include "interfaces.hpp"
#include "tmf.hpp"

namespace fff::factory {
    class Rotate;
    class MR;
}

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

    namespace factory {
        struct MR {
            template<typename ...Ts>
            constexpr auto operator()(Ts &&...ts) const noexcept
                -> MultiReturn<std::decay_t<Ts>...>
            {
                return MultiReturn<std::decay_t<Ts>...>{std::forward<Ts>(ts)...};
            }
        };
    }



    /**
     * The "mr" concept indicates that the type is multi-return.
     * @tparam T any
     * @return true if T is multi-return type, false otherwise
     */
    template<typename T>
    concept mr = made_by<T, MultiReturn> and
        requires (T t) {
            t.to_tuple();
        };

    /**
     * The "not_mr" concept indicates that the type is NOT multi-return.
     * @tparam T any
     * @return false if T is multi-return type, true otherwise
     */
    template<typename T>
    concept not_mr = not mr<T>;

    namespace liated {
        struct DeclMr {
            template<typename F, typename ...Ts>
            constexpr static auto declmr(F &&f, const MultiReturn<Ts...> &) noexcept
                -> std::invoke_result_t<F, Ts...>;
        };
    }

    template<typename F, typename T>
    concept applicable = mr<T> and
        requires (F f, T t) {
            std::apply(f, t.to_tuple());
        };

    template<typename F, mr T>
    using apply_as_mr_result_t =
        decltype(liated::DeclMr::declmr(std::declval<F>(), std::declval<T>()));

    constexpr inline factory::MR multi_return;
}


namespace fff {

    namespace liated {

        template<typename F, typename T>
        struct apply_TD {
            static_assert(true, "The second type must be a tuple");
        };

        template<typename F, typename ...Ts>
        struct apply_TD<F, std::tuple<Ts...>> {

            using apply_result = std::invoke_result_t<F, Ts...>;
            constexpr static bool is_nothrow = std::is_nothrow_invocable_v<F, Ts...>;
        };
    }

    template<typename F, made_by<std::tuple> Tuple>
    using apply_result_t = typename liated::apply_TD<F, Tuple>::apply_result;

    template<typename F, made_by<std::tuple> Tuple>
    constexpr inline bool is_nothrow_application_v = liated::apply_TD<F, Tuple>::is_nothrow;

    static_assert(std::is_same_v<int, apply_result_t<int(int, int), std::tuple<int, int>>>);

    namespace fs {
        struct Apply_f {

        private:
            template<typename F, made_by<std::tuple> Tuple, std::size_t ...I>
            constexpr auto impl(F &&f, Tuple &&tuple, std::index_sequence<I...>) const
                noexcept(is_nothrow_application_v<std::decay_t<F>, std::decay_t<Tuple>>)
                    -> apply_result_t<std::decay_t<F>, std::decay_t<Tuple>>
            {
                return std::invoke(std::forward<F>(f), std::get<I>(std::forward<Tuple>(tuple))...);
            }

        public:
            template<typename F, made_by<std::tuple> Tuple>
            constexpr auto operator()(F &&f, Tuple &&tuple) const
                noexcept(is_nothrow_application_v<std::decay_t<F>, std::decay_t<Tuple>>)
                    -> apply_result_t<std::decay_t<F>, std::decay_t<Tuple>>
            {
                return impl(std::forward<F>(f), std::forward<Tuple>(tuple),
                            std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>());
            }
        };
    }

    constexpr inline fs::Apply_f apply;



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

    namespace fs {
        struct L_Call_R_Invoke_f {
            template<typename F, made_by<std::tuple> Tuple, typename ...Args>
            constexpr auto operator()(F &&f, Tuple &&tuple, Args &&...args) const
                noexcept(is_nothrow_application_v<F, tuple_cat_type_result<Tuple, Args...>>)
                    -> apply_result_t<F, tuple_cat_type_result<Tuple, Args...>>
            {
                return apply(std::forward<F>(f),
                             std::tuple_cat(std::forward<Tuple>(tuple),
                                            std::forward_as_tuple(std::forward<Args>(args)...)));
            }
        };
    }

    constexpr inline fs::L_Call_R_Invoke_f l_call_r_invoke;



    namespace liated {
        template<typename F, typename T = void, typename ...Args>
        struct Rotate_f_TD {
            using type = std::invoke_result_t<F, Args..., T>;
        };

        template<typename F>
        struct Rotate_f_TD<F> {
            using type = std::invoke_result_t<F>;
        };
    }

    template<typename F>
    class Rotate_f : callable_i<F, Rotate_f<F>, liated::Rotate_f_TD> {

        friend callable_i<F, Rotate_f<F>, liated::Rotate_f_TD>;
        friend factory::Rotate;

        [[no_unique_address]] F f;

        constexpr explicit Rotate_f(const F &f) : f(f) {}
        constexpr explicit Rotate_f(F &&f) : f(std::move(f)) {}

        template<similar<Rotate_f> Self, typename T, typename ...Args>
            requires std::invocable<F, Args..., T>
        constexpr static auto call_impl(Self &&self, T &&t, Args &&...args)
            noexcept(std::is_nothrow_invocable_v<F, Args..., T>)
                -> std::invoke_result_t<F, Args..., T>
        {
            return std::invoke(std::forward<Self>(self).f,
                               std::forward<Args>(args)...,
                               std::forward<T>(t));
        }

        template<similar<Rotate_f> Self>
            requires std::invocable<F>
        constexpr static auto call_impl(Self &&self)
            noexcept(std::is_nothrow_invocable_v<F>)
                -> std::invoke_result_t<F>
        {
            return std::invoke(std::forward<Self>(self).f);
        }
    };

    namespace factory {

        struct Rotate {
            template<typename F>
            constexpr auto operator()(F &&f) const noexcept
            -> Rotate_f<std::decay_t<F>>
            {
                return Rotate_f<std::decay_t<F>>{std::forward<F>(f)};
            }
        };
    }

    constexpr inline factory::Rotate rotate;
}

#endif//UNDERSCORE_CPP_MULTIARGS_HPP
