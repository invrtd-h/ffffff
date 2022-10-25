#ifndef UNDERSCORE_CPP_BIND_HPP
#define UNDERSCORE_CPP_BIND_HPP

#include <functional>

#include "interfaces.hpp"
#include "multiargs.hpp"
#include "tmf.hpp"

namespace fff {

    namespace liated {

        template<class_as_value ...ValueHolders>
        struct static_l_bind_TD_impl {
            template<typename F, typename ...Args>
            struct inner {
                using type = std::invoke_result_t<F, const typename ValueHolders::type &..., Args...>;
            };
        };
    }

    template<typename F, class_as_value ...ValueHolders>
    class Static_L_Bind_f
        : public callable_i<F, Static_L_Bind_f<F, ValueHolders...>,
                            liated::static_l_bind_TD_impl<ValueHolders...>::template inner> {

        template<auto ...vp>
        friend class StaticLBindFactory;

        friend callable_i<F, Static_L_Bind_f<F, ValueHolders...>,
                          liated::static_l_bind_TD_impl<ValueHolders...>::template inner>;

        [[no_unique_address]] F f;

        constexpr explicit Static_L_Bind_f(const F &f) noexcept : f(f) {}
        constexpr explicit Static_L_Bind_f(F &&f) noexcept : f(std::move(f)) {}

        template<similar<Static_L_Bind_f> Self, typename ...Args>
            requires std::invocable<F, const typename ValueHolders::type &..., Args...>
        constexpr static auto call_impl(Self &&self, Args &&...args)
            noexcept(std::is_nothrow_invocable_v<F, const typename ValueHolders::type &..., Args...>)
                -> std::invoke_result_t<F, const typename ValueHolders::type &..., Args...>
        {
            return std::invoke(std::forward<Self>(self).f,
                               ValueHolders::value...,
                               std::forward<Args>(args)...);
        }

    public:
        using function_type = F;
    };

    template<auto ...vp>
    struct StaticLBindFactory {
        template<class F>
        constexpr auto operator()(F &&f) const noexcept
            -> Static_L_Bind_f<std::decay_t<F>, value_holder<vp>...>
        {
            return Static_L_Bind_f<std::decay_t<F>, value_holder<vp>...>{std::forward<F>(f)};
        }
    };

    template<auto ...vp>
    constexpr inline StaticLBindFactory<vp...> static_l_bind;



    namespace liated {

        template<class_as_value ...ValueHolders>
        struct static_r_bind_TD_impl {
            template<typename F, typename ...Args>
            struct inner {
                using type = std::invoke_result_t<F, Args..., const typename ValueHolders::type &...>;
            };
        };
    }

    template<typename F, class_as_value ...ValueHolders>
    class Static_R_Bind_f
        : public callable_i<F, Static_R_Bind_f<F, ValueHolders...>,
                            liated::static_r_bind_TD_impl<ValueHolders...>::template inner> {

        template<auto ...vp>
        friend class StaticRBindFactory;

        friend callable_i<F, Static_R_Bind_f<F, ValueHolders...>,
                          liated::static_r_bind_TD_impl<ValueHolders...>::template inner>;

        [[no_unique_address]] F f;

        constexpr explicit Static_R_Bind_f(const F &f) noexcept : f(f) {}
        constexpr explicit Static_R_Bind_f(F &&f) noexcept : f(std::move(f)) {}

        template<similar<Static_R_Bind_f> Self, typename ...Args>
            requires std::invocable<F, Args..., const typename ValueHolders::type &...>
        constexpr static auto call_impl(Self &&self, Args &&...args)
            noexcept(std::is_nothrow_invocable_v<F, Args..., const typename ValueHolders::type &...>)
                -> std::invoke_result_t<F, Args..., const typename ValueHolders::type &...>
        {
            return std::invoke(std::forward<Self>(self).f,
                               std::forward<Args>(args)...,
                               ValueHolders::value...);
        }

    public:
        using function_type = F;
    };

    template<auto ...vp>
    struct StaticRBindFactory {
        template<class F>
        constexpr auto operator()(F &&f) const noexcept
            -> Static_R_Bind_f<std::decay_t<F>, value_holder<vp>...>
        {
            return Static_R_Bind_f<std::decay_t<F>, value_holder<vp>...>{std::forward<F>(f)};
        }
    };

    template<auto ...vp>
    constexpr inline StaticRBindFactory<vp...> static_r_bind;
}


namespace fff {

    template<typename F, typename ...Args>
    class L_Bind_f {
        [[no_unique_address]] F f;
        [[no_unique_address]] std::tuple<Args...> args;

        template<std::convertible_to<F> G, typename ...Brgs>
        constexpr explicit L_Bind_f(G &&g, Brgs &&...brgs)
            : F(std::forward<G>(g)), args(std::make_tuple(std::forward<Brgs>(brgs)...)) {}


    };
}

#endif//UNDERSCORE_CPP_BIND_HPP
