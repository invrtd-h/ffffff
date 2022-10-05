/**
 * @author Hyegeun Song (Github : invrtd-h)
 */

#ifndef UNDERSCORE_CPP_FFFFFF_H
#define UNDERSCORE_CPP_FFFFFF_H

#define fn auto
#define NOUA [[no_unique_address]]

#include <type_traits>
#include <functional>
#include <algorithm>
#include <iostream>
#include <concepts>
#include <utility>
#include <ranges>
#include <tuple>

#include "tmf.h"

using namespace std::placeholders;

namespace fff::pol {
    /**
     * @deprecated since we do not use Bloop class
     * @see class Bloop
     */
    struct NewDataPolicy {
        static constexpr bool is_new_data_policy = true;
    };
    
    /**
     * @deprecated since we do not use Bloop class
     * @see class Bloop
     */
    struct ExecutionPolicy {
        static constexpr bool is_execution_policy = true;
    };
    
    /**
     * Unique-address Policy.
     * @namespace fff::pol
     * @var YES : implies that the inner callable is NOT an empty struct
     * @var NO : implies not
     */
    enum class uniq_addr {YES, NO};
}

namespace fff {
    template<typename T>
    concept any = true;
}

/*
 * fff::Identity impl
 */
namespace fff {
    /**
     * The function object that returns the SZ-th value by perfect forwarding.
     * @example IdentityAt\<2>()(1, 2, std::string("a")) \n
     * Returns the rvalue of std::string("a")
     * @tparam SZ the position to return
     * @cite https://en.cppreference.com/w/cpp/utility/functional/identity
     */
    template<size_t SZ>
    struct IdentityAt {
        /**
         * @tparam T Any-type
         * @tparam Args Any-parameter-pack
         * @param t Any-variable
         * @param args Any-variable-pack
         * @return std::forward<T>(t)
         */
        template<class T, typename... Args>
        constexpr auto &&operator()(T &&t, Args &&...args) const noexcept {
            return std::invoke(IdentityAt<SZ - 1>(), std::forward<Args>(args)...);
        }
    };
    
    template<>
    struct IdentityAt<0> {
        /**
         * Template specification of IdentityAt<> (read upward)
         */
        template<class T, typename ...Args>
        constexpr T &&operator()(T &&t, Args &&...args) const noexcept {
            return std::forward<T>(t);
        }
    };
    
    using Identity = IdentityAt<0>;
}

/*
 * fff::CopyAt impl
 */
namespace fff {
    
    /**
     * The function object that returns the copy of the SZ-th value.
     * @tparam SZ the position to return
     */
    template<size_t SZ>
    struct CopyAt {
        /**
         * @tparam T Any-type
         * @tparam Args Any-parameter-pack
         * @param t Any-variable
         * @param args Any-variable-pack
         * @return std::forward<T>(t)
         */
        template<class T, typename ...Args>
        constexpr auto operator()(T &&t, Args &&...args) const noexcept {
            return std::invoke(CopyAt<SZ - 1>(), std::forward<Args>(args)...);
        }
    };
    
    template<>
    struct CopyAt<0> {
        /**
         * Template specification of IdentityAt<> (read upward)
         */
        template<class T, typename ...Args>
        constexpr T operator()(T &&t, Args &&...args) const noexcept {
            return t;
        }
    };
    
    using Copy = CopyAt<0>;
    inline auto copy = Copy();
}

/*
 * fff::Noop impl
 */
namespace fff {
    
    /**
     * No-operation function obj.
     */
    struct Noop {
        /**
         * No-operation operator().
         * @tparam Args parameter pack of any-types
         * @param args parameter pack of any-variables
         * @return nothing
         */
        template<class ...Args>
        constexpr void operator()(Args &&...args) const noexcept {
            // do nothing
        }
    };
}

/*
 * fff::AlwaysConstant impl
 */
namespace fff {
    
    /**
     * A callable that always Returns a constant.
     * @tparam T the type of the constant to return
     */
    template<typename T>
    struct AlwaysConstant {
        using type = T;
        /**
         * @tparam t the value of the constant to return
         */
        template<T t>
        struct Returns {
            /**
             * @tparam Args Any
             * @param args any
             * @return t, pre-defined value by definition of the callable (orthogonal to args)
             */
            template<class ...Args>
            constexpr T operator()(Args &&...args) const noexcept {
                return t;
            }
        };
    };
    
    using AlwaysTrue = AlwaysConstant<bool>::Returns<true>;
    using AlwaysFalse = AlwaysConstant<bool>::Returns<false>;
}

/*
 * fff::AsSingle impl
 */
namespace fff {
    /**
     * A type constructor that makes a singleton.
     * @tparam T The type of singleton\n
     * should hide constructors and should delete copy/move procedures\n
     * should declare AsSingle as a friend
     */
    template<typename T>
    struct AsSingle {
        T &get() {
            static T *data = new T();
            return *data;
        }
    };
}

/*
 * fff::Once impl
 */
namespace fff {
    
    using pol::uniq_addr;
    
    template<class F>
    class Once;
    
    template<class F>
        requires (not std::is_void_v<std::invoke_result_t<F>>
                and not std::is_empty_v<F>)
    class Once<F> {
        friend class OnceFactory;
    
        F f;
        mutable std::invoke_result_t<F> memo;
        mutable bool flag;
    
        constexpr explicit Once(const F &f) noexcept : f(f), flag(false) {}
        constexpr explicit Once(F &&f) noexcept : f(std::move(f)), flag(false) {}

    public:
        constexpr std::invoke_result_t<F> operator()() const
            noexcept(noexcept(std::invoke(f)))
        {
            if (flag) {
                return memo;
            }
            flag = true;
            return memo = std::invoke(f);
        }
    };
    
    template<class F>
    requires (not std::is_void_v<std::invoke_result_t<F>>
              and std::is_empty_v<F>)
    class Once<F> {
        friend class OnceFactory;
        
        mutable std::invoke_result_t<F> memo;
        mutable bool flag;
        
        constexpr explicit Once(const F &f) noexcept : flag(false) {}
        constexpr explicit Once(F &&f) noexcept : flag(false) {}
    
    public:
        constexpr std::invoke_result_t<F> operator()() const
            noexcept(noexcept(std::invoke(F())))
        {
            if (flag) {
                return memo;
            }
            flag = true;
            return memo = std::invoke(F());
        }
    };
    
    template<class F>
    requires (std::is_void_v<std::invoke_result_t<F>>
              and not std::is_empty_v<F>)
    class Once<F> {
        friend class OnceFactory;
        
        F f;
        mutable bool flag;
        
        constexpr explicit Once(const F &f) noexcept : f(f), flag(false) {}
        constexpr explicit Once(F &&f) noexcept : f(std::move(f)), flag(false) {}
    
    public:
        constexpr void operator()() const
            noexcept(noexcept(std::invoke(f)))
        {
            if (flag) {
                return;
            }
            flag = true;
            std::invoke(f);
        }
    };
    
    template<class F>
    requires (std::is_void_v<std::invoke_result_t<F>>
              and std::is_empty_v<F>)
    class Once<F> {
        friend class OnceFactory;
        
        mutable bool flag;
        
        constexpr explicit Once(const F &f) noexcept : flag(false) {}
        constexpr explicit Once(F &&f) noexcept : flag(false) {}
    
    public:
        constexpr void operator()() const
            noexcept(noexcept(std::invoke(F())))
        {
            if (flag) {
                return;
            }
            flag = true;
            std::invoke(F());
        }
    };
    
    struct OnceFactory {
        template<class F>
            requires std::invocable<F>
        constexpr auto operator()(F &&f) noexcept {
            return Once<F>(std::forward<F>(f));
        }
    };
}

/* fff::Count impl */
namespace fff {
    template<class F>
    class Count;
    
    template<nonempty_type F>
    class Count<F> {
        friend class CountFactory;
        
        F f;
        mutable int cnt;
        
        constexpr explicit Count(const F &f) noexcept : f(f), cnt(0) {}
        constexpr explicit Count(F &&f) noexcept : f(std::move(f)), cnt(0) {}
        
    public:
        template<class ...Args>
            requires std::invocable<F, Args...>
        constexpr auto operator()(Args ...args) const
            noexcept(noexcept(std::invoke(f, std::forward<Args>(args)...)))
        {
            ++cnt;
            return std::invoke(f, std::forward<Args>(args)...);
        }
        constexpr int get_count() const noexcept {
            return cnt;
        }
    };
    
    template<empty_type F>
    class Count<F> {
        friend class CountFactory;
        
        [[no_unique_address]] F f;
        mutable int cnt;
    
        constexpr explicit Count(const F &f) noexcept : cnt(0) {}
        constexpr explicit Count(F &&f) noexcept : cnt(0) {}
    
    public:
        template<class ...Args>
            requires std::invocable<F, Args...>
        constexpr auto operator()(Args ...args) const
            noexcept(noexcept(std::invoke(f, std::forward<Args>(args)...)))
        {
            ++cnt;
            return std::invoke(f, std::forward<Args>(args)...);
        }
        constexpr int get_count() const noexcept {
            return cnt;
        }
    };
    
    struct CountFactory {
        template<class F>
        constexpr auto operator()(F &&f) const noexcept {
            return Count<F>(std::forward<F>(f));
        }
    };
}

/*
 * fff::IdGiver impl
 */
namespace fff {
    struct IdGiverFactory {
        constexpr auto operator()(unsigned long long start) const noexcept {
            return [start]() mutable {return start++;};
        }
    };
}

/*
 * fff::Log impl
 */
namespace fff {

}

/*
 * fff::pipethrow(), fff::pipecatch impl
 */
namespace fff {
    struct PipeCatch {};
    
    template<typename T>
    class On {
        T data;
        
    public:
        constexpr explicit On(const T &t) noexcept(noexcept(T(t)))
                : data(t) {}
        constexpr explicit On(T &&t) noexcept(noexcept(T(t)))
                : data(t) {}
        
        constexpr explicit operator T() const noexcept {
            return data;
        }
        
        constexpr T operator>>(PipeCatch u) const noexcept {
            return data;
        }
    
        template<class F>
            requires (std::invocable<F, T> and not std::is_void_v<std::invoke_result_t<F, T>>)
        constexpr auto operator>>(F &&f) const
            noexcept(noexcept(std::invoke(std::forward<F>(f), data)))
        {
            return On<std::invoke_result_t<F, T>>(std::invoke(std::forward<F>(f), data));
        }
    };
    
    struct OnFactory {
        template<typename T>
        constexpr auto operator()(T t) const {
            return On<T>(t);
        }
    };
}

/*
 * fff::Maybe impl
 */
namespace fff {
    template<typename T>
    class Maybe : public std::optional<T> {
    public:
        constexpr static bool is_maybe = true;
        
        template<class ...Args>
            requires std::is_constructible_v<std::optional<T>, Args...>
        constexpr Maybe<T>(Args &&...args) noexcept : std::optional<T>(std::forward<Args>(args)...) {}
        
        /**
         * Lift : (T -> U) -> (M<T> -> M<U>)
         * @tparam F Function Object Type
         * @param f Function Object
         * @return f(x) if x is not empty, std::nullopt if empty
         */
        template<class F>
            requires (std::invocable<F, T>
                    and not std::is_void_v<std::invoke_result_t<F, T>>
                    and not maybetype<std::invoke_result_t<F, T>>)
        constexpr Maybe<std::invoke_result_t<F, T>> operator>>(F &&f) const
            noexcept(noexcept(std::invoke(std::forward<F>(f), this->value())))
        {
            if (this->has_value()) {
                return std::invoke(std::forward<F>(f), this->value());
            } else {
                return std::nullopt;
            }
        }
        
        /**
         * Flatlift : (T -> M<U>) -> (M<T> -> M<U>)
         * @tparam F Function Object Type
         * @param f Function Object
         * @return
         */
        template<class F>
            requires (std::invocable<F, T>
                  and maybetype<std::invoke_result_t<F, T>>)
        constexpr std::invoke_result_t<F, T> operator>>(F &&f) const
            noexcept(noexcept(std::invoke(std::forward<F>(f), this->value())))
        {
            if (this->has_value()) {
                return std::invoke(std::forward<F>(f), this->value());
            } else {
                return std::nullopt;
            }
        }
        
        template<class F>
            requires std::invocable<F, T &>
        constexpr Maybe<T> &operator<<(F &&f)
            noexcept(noexcept(std::invoke(std::forward<F>(f), this->value())))
        {
            if (this->has_value()) {
                std::invoke(std::forward<F>(f), this->value());
            }
            return *this;
        }
    };

    struct MaybeFactory {
        template<typename T>
        constexpr auto operator()(T &&t) const noexcept {
            return Maybe<T>(std::forward<T>(t));
        }
        
        template<typename T>
        constexpr auto make() const noexcept {
            return Maybe<T>();
        }
    };
}

/*
 * fff::Concat impl
 */
namespace fff {
    
    template<class Fn_1, class Fn_2>
    class Concat {
        Fn_1 f_1;
        Fn_2 f_2;
    public:
        using F1 = Fn_1;
        using F2 = Fn_2;
    
        template<class U1, class U2>
        constexpr Concat(U1 &&f1, U2 &&f2) noexcept
                : f_1(std::forward<U1>(f1)), f_2(std::forward<U2>(f2)) {}
    
        template<class... Args>
            requires std::invocable<Fn_1, Args...>
        constexpr auto operator()(Args &&...args) const
            noexcept(noexcept(f_1(std::forward<Args>(args)...)))
        {
            return f_1(std::forward<Args>(args)...);
        }
    
        template<class... Args>
            requires (not std::invocable<Fn_1, Args...> and std::invocable<Fn_2, Args...>)
        constexpr auto operator()(Args &&...args) const
            noexcept(noexcept(f_2(std::forward<Args>(args)...)))
        {
            return f_2(std::forward<Args>(args)...);
        }
    };
    
    struct ConcatFactory {
        template<class F1, class F2>
        constexpr auto operator()(F1 &&f1, F2 &&f2) const noexcept {
            return Concat<F1, F2>(std::forward<F1>(f1), std::forward<F2>(f2));
        }
    
        template<class F1, class F2, class... Fp>
        constexpr auto operator()(F1 &&f1, F2 &&f2, Fp &&...fp) const noexcept {
            return operator()(Concat<F1, F2>(std::forward<F1>(f1), std::forward<F2>(f2)),
                              std::forward<Fp>(fp)...);
        }
    };
}

/*
 * fff::Overload impl
 */
namespace fff {
    template<class ...Fp>
    struct Overload : Fp... {
        using Fp::operator()...;
    };
    
    struct OverloadFactory {
        template<class ...Fp>
        constexpr auto operator()(Fp &&...fp) const noexcept {
            return Overload<std::remove_reference_t<Fp>...>{std::forward<Fp>(fp)...};
        }
    };
}

namespace fff {
    
    /**
     * @todo need to check whether this is thread-safe
     */
    template<class F, class ...Fp>
    struct Concaten : F, Concaten<Fp...> {
    
        template<class ...Args>
            requires std::invocable<F, Args...>
        constexpr auto operator()(Args &&...args) const
            noexcept(noexcept(F::operator()(std::forward<Args>(args)...)))
        {
            return F::operator()(std::forward<Args>(args)...);
        }
        
        template<class ...Args>
            requires (not std::invocable<F, Args...>)
        constexpr auto operator()(Args &&...args) const
            noexcept(noexcept(Concaten<Fp...>::operator()(std::forward<Args>(args)...)))
        {
            return Concaten<Fp...>::operator()(std::forward<Args>(args)...);
        }
    };
    
    template<class F>
    struct Concaten<F> : F {
        template<class ...Args>
            requires std::invocable<F, Args...>
        constexpr auto operator()(Args &&...args) const
            noexcept(noexcept(F::operator()(std::forward<Args>(args)...)))
        {
            return F::operator()(std::forward<Args>(args)...);
        }
    };
    
    struct ConcatenFactory {
        template<class F>
        constexpr auto operator()(F &&f) const noexcept {
            return Concaten<std::remove_reference_t<F>>{std::forward<F>(f)};
        }
        
        template<class F, class ...Fp>
        constexpr auto operator()(F &&f, Fp &&...fp) const noexcept {
            return Concaten<std::remove_reference_t<F>, std::remove_reference_t<Fp>...>
                    {std::forward<F>(f), operator()(std::forward<Fp>(fp)...)};
        }
    };
}

namespace fff {
    
    template<class F, class ...Fp>
    struct Compose;
    
    template<nonempty_type F>
    struct Compose<F> {
        F f;
    
        template<class ...Args>
        constexpr auto operator()(Args &&...args) const &
        noexcept(noexcept(std::invoke(f, std::forward<Args>(args)...)))
        {
            return std::invoke(f, std::forward<Args>(args)...);
        }
    
        template<class ...Args>
        constexpr auto operator()(Args &&...args) const &&
        noexcept(noexcept(std::invoke(std::move(f), std::forward<Args>(args)...)))
        {
            return std::invoke(std::move(f), std::forward<Args>(args)...);
        }
    };
    
    template<empty_type F>
    struct Compose<F> {
        [[no_unique_address]] F f;
        
        template<class ...Args>
        constexpr auto operator()(Args &&...args) const &
        noexcept(noexcept(std::invoke(f, std::forward<Args>(args)...)))
        {
            return std::invoke(f, std::forward<Args>(args)...);
        }
        
        template<class ...Args>
        constexpr auto operator()(Args &&...args) const &&
        noexcept(noexcept(std::invoke(std::move(f), std::forward<Args>(args)...)))
        {
            return std::invoke(std::move(f), std::forward<Args>(args)...);
        }
    };
    
    template<nonempty_type F1, class ...Fp>
        requires nonempty_type<Compose<Fp...>>
    struct Compose<F1, Fp...> {
        F1 f1;
        Compose<Fp...> f2;
    
        template<class ...Args>
        constexpr auto operator()(Args &&...args) const &
            noexcept(noexcept(std::invoke(f1, std::invoke(f2, std::forward<Args>(args)...))))
        {
            return std::invoke(f1, std::invoke(f2, std::forward<Args>(args)...));
        }
        
        template<class ...Args>
        constexpr auto operator()(Args &&...args) const &&
            noexcept(noexcept(std::invoke(std::move(f1), std::invoke(std::move(f2), std::forward<Args>(args)...))))
        {
            return std::invoke(std::move(f1), std::invoke(std::move(f2), std::forward<Args>(args)...));
        }
    };
    
    template<empty_type F1, class ...Fp>
        requires nonempty_type<Compose<Fp...>>
    struct Compose<F1, Fp...> {
        [[no_unique_address]] F1 f1;
        Compose<Fp...> f2;
        
        template<class ...Args>
        constexpr auto operator()(Args &&...args) const &
            noexcept(noexcept(std::invoke(f1, std::invoke(f2, std::forward<Args>(args)...))))
        {
            return std::invoke(f1, std::invoke(f2, std::forward<Args>(args)...));
        }
        
        template<class ...Args>
        constexpr auto operator()(Args &&...args) const &&
            noexcept(noexcept(std::invoke(f1, std::invoke(std::move(f2), std::forward<Args>(args)...))))
        {
            return std::invoke(f1, std::invoke(std::move(f2), std::forward<Args>(args)...));
        }
    };
    
    template<nonempty_type F1, class ...Fp>
        requires empty_type<Compose<Fp...>>
    struct Compose<F1, Fp...> {
        F1 f1;
        [[no_unique_address]] Compose<Fp...> f2;
        
        template<class ...Args>
        constexpr auto operator()(Args &&...args) const &
            noexcept(noexcept(std::invoke(f1, std::invoke(f2, std::forward<Args>(args)...))))
        {
            return std::invoke(f1, std::invoke(f2, std::forward<Args>(args)...));
        }
        
        template<class ...Args>
        constexpr auto operator()(Args &&...args) const &&
            noexcept(noexcept(std::invoke(std::move(f1), std::invoke(f2, std::forward<Args>(args)...))))
        {
            return std::invoke(std::move(f1), std::invoke(f2, std::forward<Args>(args)...));
        }
    };
    
    template<empty_type F1, class ...Fp>
        requires empty_type<Compose<Fp...>>
    struct Compose<F1, Fp...> {
        [[no_unique_address]] F1 f1;
        [[no_unique_address]] Compose<Fp...> f2;
        
        template<class ...Args>
        constexpr auto operator()(Args &&...args) const
            noexcept(noexcept(std::invoke(f1, std::invoke(f2, std::forward<Args>(args)...))))
        {
            return std::invoke(f1, std::invoke(f2, std::forward<Args>(args)...));
        }
    };
    
    struct ComposeFactory {
        template<class F>
        constexpr auto operator()(F &&f) const noexcept {
            return Compose<std::decay_t<F>>{std::forward<F>(f)};
        }
        
        template<class F, class ...Fp>
        constexpr auto operator()(F &&f, Fp &&...fp) const noexcept {
            return Compose<std::decay_t<F>, std::decay_t<Fp>...>
                    {std::forward<F>(f), operator()(std::forward<Fp>(fp)...)};
        }
    };
}

namespace fff {
    
    /**
     * Making Result-Container function obj.
     * @param cont any std::(container) with type T
     * @param func any function obj with 1 param, say, T -> U
     * @return any std::(container) with type U
     */
    
    struct PreallocCont : public pol::NewDataPolicy {
        template<typename T, class FuncObj, size_t N>
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
    
    struct NewCont : public pol::NewDataPolicy {
        template<class Cont, class FuncObj>
            requires std::ranges::range<Cont>
                and std::is_default_constructible_v<Cont>
        constexpr auto operator()(const Cont &cont, const FuncObj &funcObj) const noexcept {
            return Cont();
        }
    };
    
    struct MapExecution : public pol::ExecutionPolicy {
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
    
    struct PushExecution : public pol::ExecutionPolicy {
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
    
    template<class FuncObj>
    struct FilterWith {
        template<class Cont>
        constexpr auto operator()(Cont &cont) const
            noexcept(noexcept(FuncObj()(cont[0])))
        {
            return Filter()(cont, FuncObj());
        }
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
        constexpr bool operator()(const Cont &cont, const FuncObj &func) const
            noexcept(noexcept(func(cont[0])))
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
    
    template<class ANewDataPolicy, class AnExecutionPolicy>
    struct Bloop {
        using new_data_policy = ANewDataPolicy;
        using execution_policy = AnExecutionPolicy;
        
        template<class Cont, class FuncObj>
            requires std::ranges::range<Cont>
                 and std::invocable<FuncObj, typename Cont::value_type &>
        constexpr auto operator()(Cont &cont, const FuncObj &func) const {
            
            decltype(auto) ret = ANewDataPolicy()(cont, func);
            
            AnExecutionPolicy()(ret, cont, func);
            
            return ret;
        }
    };
    
    using BloopEach = Bloop<IdentityAt<0>, MapExecution>;
    using BloopMap = Bloop<PreallocCont, MapExecution>;
    using BloopFilter = Bloop<NewCont, PushExecution>;
}

/**
 * The laboratory : New features are tested in this space.
 */
namespace impl::lab {
    /**
     * get class Cont == C<T>, typename T
     * return C<U>
     */
    
    struct Foo {
        template<template<class> class C, typename T>
        constexpr auto operator()(const C<T> &cont) {
            std::cout << typeid(C<T>).name() << ' ' << typeid(T).name() << '\n';
        }
    };
    
    template<class Validator, class FuncObj_T, class FuncObj_F>
    struct Conditional {
        template<typename ...Args>
            requires std::is_same_v<std::invoke_result_t<FuncObj_T, Args...>, std::invoke_result_t<FuncObj_F, Args...>>
        constexpr auto operator()(Args &&...args) const noexcept {
            if (Validator()(std::forward<Args>(args)...)) {
                return FuncObj_T()(std::forward<Args>(args)...);
            } else {
                return FuncObj_F()(std::forward<Args>(args)...);
            }
        }
    };
}

namespace fff {
    inline Each                     each;
    inline Map                      map;
    inline Filter                   filter;
    inline Reject                   reject;
    
    inline Some                     some;
    inline Every                    every;
    inline None                     none;
    
    inline AlwaysTrue               always_true;
    inline AlwaysFalse              always_false;
    
    template<std::size_t SZ>
    inline IdentityAt<SZ>           identity_at;
    template<std::size_t SZ>
    inline CopyAt<SZ>               copy_at;
    
    inline OnceFactory              once;
    inline CountFactory             count;
    inline IdGiverFactory           id_giver;
    
    inline MaybeFactory             maybe;
    
    inline PipeCatch                pipecatch;
    inline OnFactory                pipethrow;
    
    inline ConcatFactory            concat;
    inline ConcatenFactory          concaten;
    inline OverloadFactory          overload;
    inline ComposeFactory           compose;
}

namespace fff {
    struct Package {
        NOUA Each each{};
        NOUA Map map{};
        NOUA Filter filter{};
        NOUA Reject reject{};
        
        NOUA Some some{};
        NOUA Every every{};
        NOUA None none{};
        
        NOUA AlwaysTrue always_true{};
        NOUA AlwaysFalse always_false{};
        
        NOUA IdentityAt<0> identity{};
        NOUA CopyAt<0> copy{};
        
        NOUA OnceFactory once{};
        NOUA CountFactory count{};
        NOUA IdGiverFactory id_giver{};
        
        NOUA MaybeFactory maybe{};
        NOUA PipeCatch pipecatch{};
        NOUA OnFactory pipethrow{};
        
        NOUA ConcatFactory concat{};
        NOUA ConcatenFactory concaten{};
        NOUA OverloadFactory overload{};
        NOUA ComposeFactory compose{};
        
        Package() = default;
    };
}

static_assert(std::is_empty_v<fff::Package>, "the Package class should be empty");

#undef NOUA
#undef fn

#endif //UNDERSCORE_CPP_FFFFFF_H

/*

template<typename T, template<class,class...> class C, class... Args>
std::ostream& operator<<(std::ostream& os, const C<T,Args...>& objs)
{
    os << __PRETTY_FUNCTION__ << '\n';
    for (auto const& obj : objs)
        os << obj << ' ';
    return os;
}
 
 */