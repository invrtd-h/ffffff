#ifndef UNDERSCORE_CPP_BIND_HPP
#define UNDERSCORE_CPP_BIND_HPP

#include <functional>

#include "tmf.hpp"
#include "interfaces.hpp"

namespace fff {

    template<class_as_value ...ValueHolders>
    struct static_l_bind_TD_impl {
        template<typename F, typename ...Args>
        struct inner {
            using type = std::invoke_result_t<F, const typename ValueHolders::type &..., Args...>;
        };
    };

    template<typename F, class_as_value ...ValueHolders>
    class static_l_bind_f
        : public callable_i<F, static_l_bind_f<F, ValueHolders...>,
                            static_l_bind_TD_impl<ValueHolders...>::template inner> {

        template<auto ...vp>
        friend class static_l_bind_factory;

        friend callable_i<F, static_l_bind_f<F, ValueHolders...>,
                          static_l_bind_TD_impl<ValueHolders...>::template inner>;

        [[no_unique_address]] F f;

        constexpr explicit static_l_bind_f(const F &f) noexcept : f(f) {}
        constexpr explicit static_l_bind_f(F &&f) noexcept : f(std::move(f)) {}

        template<similar<static_l_bind_f> Self, typename ...Args>
            requires std::invocable<F, const typename ValueHolders::type &..., Args...>
        constexpr static auto call_impl(Self &&self, Args &&...args)
            noexcept(std::is_nothrow_invocable_v<F, const typename ValueHolders::type &..., Args...>)
                -> std::invoke_result_t<F, const typename ValueHolders::type &..., Args...>
        {
            return std::invoke(std::forward<Self>(self).f_fwd(),
                               ValueHolders::value...,
                               std::forward<Args>(args)...);
        }

        constexpr       F &  f_fwd()       &  noexcept {return f;}
        constexpr const F &  f_fwd() const &  noexcept {return f;}
        constexpr       F && f_fwd()       && noexcept {return std::move(f);}
        constexpr const F && f_fwd() const && noexcept {return std::move(f);}

    public:
        using function_type = F;
    };

    template<auto ...vp>
    struct static_l_bind_factory {
        template<class F>
        constexpr auto operator()(F &&f) const noexcept
            -> static_l_bind_f<std::decay_t<F>, value_holder<vp>...>
        {
            return static_l_bind_f<std::decay_t<F>, value_holder<vp>...>{std::forward<F>(f)};
        }
    };

    template<auto ...vp>
    constexpr inline static_l_bind_factory<vp...> static_l_bind;



    template<class_as_value ...ValueHolders>
    struct static_r_bind_TD_impl {
        template<typename F, typename ...Args>
        struct inner {
            using type = std::invoke_result_t<F, Args..., const typename ValueHolders::type &...>;
        };
    };

    template<typename F, class_as_value ...ValueHolders>
    class static_r_bind_f
        : public callable_i<F, static_r_bind_f<F, ValueHolders...>,
                            static_r_bind_TD_impl<ValueHolders...>::template inner> {

        template<auto ...vp>
        friend class static_r_bind_factory;

        friend callable_i<F, static_r_bind_f<F, ValueHolders...>,
                          static_r_bind_TD_impl<ValueHolders...>::template inner>;

        [[no_unique_address]] F f;

        constexpr explicit static_r_bind_f(const F &f) noexcept : f(f) {}
        constexpr explicit static_r_bind_f(F &&f) noexcept : f(std::move(f)) {}

        template<similar<static_r_bind_f> Self, typename ...Args>
            requires std::invocable<F, Args..., const typename ValueHolders::type &...>
        constexpr static auto call_impl(Self &&self, Args &&...args)
            noexcept(std::is_nothrow_invocable_v<F, Args..., const typename ValueHolders::type &...>)
                -> std::invoke_result_t<F, Args..., const typename ValueHolders::type &...>
        {
            return std::invoke(std::forward<Self>(self).f_fwd(),
                               std::forward<Args>(args)...,
                               ValueHolders::value...);
        }

        constexpr       F &  f_fwd()       &  noexcept {return f;}
        constexpr const F &  f_fwd() const &  noexcept {return f;}
        constexpr       F && f_fwd()       && noexcept {return std::move(f);}
        constexpr const F && f_fwd() const && noexcept {return std::move(f);}

    public:
        using function_type = F;
    };

    template<auto ...vp>
    struct static_r_bind_factory {
        template<class F>
        constexpr auto operator()(F &&f) const noexcept
            -> static_r_bind_f<std::decay_t<F>, value_holder<vp>...>
        {
            return static_r_bind_f<std::decay_t<F>, value_holder<vp>...>{std::forward<F>(f)};
        }
    };

    template<auto ...vp>
    constexpr inline static_r_bind_factory<vp...> static_r_bind;




}

#endif//UNDERSCORE_CPP_BIND_HPP
