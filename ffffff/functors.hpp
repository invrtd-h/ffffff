#ifndef UNDERSCORE_CPP_FUNCTORS_HPP
#define UNDERSCORE_CPP_FUNCTORS_HPP

#include <functional>
#include <algorithm>
#include <ranges>

#include "tmf.hpp"
#include "basic_ops.hpp"

namespace fff {

    /**
    * Making Result-Container function obj.
    * @param cont any std::(container) with type T
    * @param func any function obj with 1 param, say, T -> U
    * @return any std::(container) with type U
    */

    struct PreallocCont {
        template<typename T, class FuncObj, std::size_t N>
            requires std::invocable<FuncObj, T>
            and std::is_default_constructible_v<std::invoke_result_t<FuncObj, T>>
        constexpr auto operator()(const std::array<T, N> &cont, const FuncObj &func) const noexcept {
            return std::array<std::invoke_result_t<FuncObj, T>, N>();
        }

        template<template<class> class C, typename T, class FuncObj>
            requires std::ranges::range<C<T>>
            and std::invocable<FuncObj, T>
            and std::is_default_constructible_v<std::invoke_result_t<FuncObj, T>>
        constexpr auto operator()(const C<T> &cont, const FuncObj &func) const noexcept {
            return C<std::invoke_result_t<FuncObj, T>>(cont.size());
        }
    };

    struct NewCont {
        template<class Cont, class FuncObj>
            requires std::ranges::range<Cont>
            and std::is_default_constructible_v<Cont>
        constexpr auto operator()(const Cont &cont, const FuncObj &funcObj) const noexcept {
            return Cont();
        }
    };

    struct MapExecution {
        /**
        * @todo consider if the return value of func is void
        */
        template<class T_cont, class U_cont, class FuncObj>
            requires std::ranges::range<T_cont>
            and std::ranges::range<U_cont>
            and std::is_same_v<typename std::invoke_result<FuncObj, typename T_cont::value_type>::type,
                               typename U_cont::value_type>
            constexpr auto &operator()(U_cont &u_cont, T_cont &t_cont, const FuncObj &func) const
            noexcept(noexcept(func(t_cont[0])))
        {
            auto it_t = t_cont.begin();
            auto it_u = u_cont.begin();

            while (it_t != t_cont.end()) {
                *it_u = std::invoke(func, *it_t);
                ++it_t; ++it_u;
            }

            return u_cont;
        }
    };

    struct PushExecution {
        template<class T_cont, class FuncObj>
            requires std::ranges::range<T_cont>
            and std::convertible_to<std::invoke_result_t<FuncObj, typename T_cont::value_type>, bool>
            constexpr auto &operator()(T_cont &res_cont, T_cont &var_cont, const FuncObj &func) const
            noexcept(noexcept(func(var_cont[0])))
        {
            for (const auto &t : var_cont) {
                if (std::invoke(func, t)) {
                    PushPolicy()(res_cont, t);
                }
            }

            return res_cont;
        }

        struct PushPolicy {
            /**
            * If the container has push_back() method, apply it
            */
            template<template<class> class C, typename T>
                requires backpushable<C>
            constexpr void operator()(C<T> &res_cont, const T &val) const noexcept {
                res_cont.push_back(val);
            }

            /**
            * If the container has no push_back() method and has insert() method,
            * apply it
            */
            template<template<class> class C, typename T>
                requires (not backpushable<C> and insertible<C>)
            constexpr void operator()(C<T> &res_cont, const T &val) const noexcept {
                res_cont.insert(val);
            }
        };
    };

    struct Each {
        template<class Cont, class FuncObj>
            requires std::ranges::range<Cont>
            and std::invocable<FuncObj, typename Cont::value_type &>
        constexpr void operator()(Cont &cont, const FuncObj &func) const
            noexcept(noexcept(func(cont[0])))
        {
            std::ranges::for_each(cont, func);
        }
    };

    struct Map {
        template<class Cont, class FuncObj>
            requires std::ranges::range<Cont>
            and std::invocable<FuncObj, typename Cont::value_type &>
        constexpr auto operator()(const Cont &cont, const FuncObj &func) const
            noexcept(noexcept(func(cont[0])))
        {
            auto ret = PreallocCont()(cont, func);

            {
                auto it_t = cont.begin();
                auto it_u = ret.begin();

                while (it_t != cont.end()) {
                    *it_u = std::invoke(func, *it_t);
                    ++it_t; ++it_u;
                }
            }

            return ret;
        }
    };

    struct Filter {
        template<class Cont, class FuncObj>
            requires std::ranges::range<Cont>
            and std::convertible_to<std::invoke_result_t<FuncObj, typename Cont::value_type &>, bool>
            constexpr auto operator()(const Cont &cont, const FuncObj &func) const
            noexcept(noexcept(func(cont[0])))
        {
            auto ret = NewCont()(cont, copy);

            for (const auto &v : cont) {
                if (std::invoke(func, v)) {
                    PushPolicy()(ret, v);
                }
            }

            return ret;
        }

        struct PushPolicy {
            /**
            * If the container has push_back() method, apply it
            */
            template<template<class> class C, typename T>
                requires backpushable<C>
            constexpr void operator()(C<T> &res_cont, const T &val) const noexcept {
                res_cont.push_back(val);
            }

            /**
            * If the container has no push_back() method and has insert() method,
            * apply it
            */
            template<template<class> class C, typename T>
                requires (not backpushable<C> and insertible<C>)
            constexpr void operator()(C<T> &res_cont, const T &val) const noexcept {
                res_cont.insert(val);
            }
        };
    };

    struct Reject {
        template<class Cont, class FuncObj>
            requires std::ranges::range<Cont>
            and std::convertible_to<std::invoke_result_t<FuncObj, typename Cont::value_type>, bool>
            constexpr auto operator()(const Cont &cont, const FuncObj &func) const
            noexcept(noexcept(func(cont[0])))
        {
            return Filter()(cont, std::not_fn(func));
        }
    };

    template<bool func_ret, bool ret>
    struct LogicMake {
        template<class Cont, class FuncObj>
            requires std::ranges::range<Cont>
            and std::convertible_to<std::invoke_result_t
                                    <FuncObj, std::remove_cv_t<typename Cont::value_type &>>, bool>
            constexpr auto operator()(const Cont &cont, const FuncObj &func) const
            noexcept(noexcept(func(cont[0]))) -> bool
        {
            for (auto &v : cont) {
                if (static_cast<bool>(std::invoke(func, v)) == func_ret) {
                    return ret;
                }
            }
            return not ret;
        }
    };

    using Some = LogicMake<true, true>;
    using Every = LogicMake<false, false>;
    using None = LogicMake<true, false>;

    constexpr inline Some some;
    constexpr inline Every every;
    constexpr inline None none;
}

#endif//UNDERSCORE_CPP_FUNCTORS_HPP
