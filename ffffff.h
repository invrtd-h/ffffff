/**
 * @author Hyegeun Song (Github : invrtd-h)
 */

#ifndef UNDERSCORE_CPP_FFFFFF_H
#define UNDERSCORE_CPP_FFFFFF_H

#include <type_traits>
#include <functional>
#include <algorithm>
#include <iostream>
#include <concepts>
#include <sstream>
#include <utility>
#include <memory>
#include <ranges>
#include <tuple>

namespace fff {

    struct ConstexprDetermination {
        /**
         * determines if the parameter is constant-evaluated
         * @return true, always
         * @error if the parameter is NOT constant-evaluated
         */
        consteval static bool is_consteval_var(auto &&) noexcept {
            return true;
        }
    };
    
    /**
     * Determines the type of a value at compile time.
     * @example Typeof\<1> == int
     * @example Typeof\<std::make_pair(1, 1)> == std::pair\<int, int>
     */
    template<auto V>
    using TypeOf = std::decay_t<decltype(V)>;
    
    template<unsigned int N, typename T = void, typename ...U>
    struct AmongImpl {
        using type = typename AmongImpl<N - 1, U...>::type;
    };
    
    template<typename T, typename ...U>
    struct AmongImpl<0, T, U...> {
        using type = T;
    };
    
    template<typename ...T>
    struct Among {
        template<unsigned int N>
        using get = typename AmongImpl<N, T...>::type;
    };
    
    static_assert(std::is_same_v<int, Among<char, double, int>::get<2>>);
    
    template<unsigned int N, typename ...T>
    using NthAmong = typename Among<T...>::template get<N>;
    
    template<unsigned int N>
    struct TypeAt {
        template<template<class, class...> class C, typename T, typename ...U>
        constexpr static auto of(C<T, U...>) noexcept -> NthAmong<N, T, U...>;
    };
    
    template<typename T, unsigned int N = 0>
    using ValueType = decltype(TypeAt<N>::of(std::declval<T>()));
    
    static_assert(std::is_same_v<int, ValueType<std::unordered_map<double, int>, 1>>);
    
    template<typename T>
    concept typenested = requires {
        TypeAt<0>::of(std::declval<T>());
    };
    
    template<typename T>
    concept non_typenested = not typenested<T>;
    
    static_assert(non_typenested<int>);
    static_assert(typenested<std::vector<int>>);
    static_assert(typenested<std::pair<int, int>>);
    
    static_assert(non_typenested<std::array<int, 3>>);
    
    
    /**
     * A concept determines whether the given C is a unary predicate.
     * @tparam C Any unary type constructor
     * @example unary_pred\<std::is_class> is true, since std::is_class\<T>::value is evaluated as bool
     * @example unary_pred\<std::vector> is false
     */
    template<template<class> class C>
    concept unary_pred = requires {
        { C<int>::value } -> std::convertible_to<bool>;
        ConstexprDetermination::is_consteval_var(C<int>::value);
    };

    static_assert(unary_pred<std::is_class> and not unary_pred<std::vector>);
    
    
    
    /**
     * A temporal type holder that holds any type.
     * @tparam T, Ts any typename
     * @example TempTypeHolder\<int, double, char> is valid. (There is no requirement)
     * @using type, the type alias for T
     * @using next, the type alias for TempTypeHolder\<Ts...> i.e. a class that temporally holds all types except T
     * @example TempTypeHolder\<int, double, char>::next::next::type == char
     */
    template<typename T, typename ...Ts>
    struct TempTypeHolder {
        using type = T;
        using next = TempTypeHolder<Ts...>;
        
        /**
         * Determines whether every T and Ts... satisfies the predicate.
         * @tparam C A unary predicate that C\<T>::value is evaluated as a constexpr bool
         */
        template<template<class> class C>
            requires unary_pred<C>
        constexpr static const bool value = C<type>::value and
                (std::is_void_v<next> or next::template value<C>);
    };
    
    template<typename T>
    struct TempTypeHolder<T> {
        using type = T;
        using next = void;
        
        /**
         * Determines whether every T and Ts... satisfies the predicate.
         * @tparam C A unary predicate that C<T>::value is evaluated as a constexpr bool
         */
        template<template<class> class C>
        requires unary_pred<C>
        constexpr static const bool value = C<type>::value;
    };
    
    template<template<class...> class C>
    struct ChangeTemplateImpl {
        template<template<class...> class D, typename ...T>
        constexpr static auto instead_of(D<T...>) noexcept -> C<T...>;
    };
    
    template<template<class...> class C, typename T>
    using ChangeTemplate = decltype(ChangeTemplateImpl<C>::instead_of(std::declval<T>()));
    
    static_assert(std::is_same_v<std::vector<int>, ChangeTemplate<std::vector, std::optional<int>>>);
    
    template<template<class> class Pred, typename T = void, typename ...Ts>
    requires unary_pred<Pred>
    struct EveryType {
        constexpr const static bool value =
                std::is_void_v<T> or (Pred<T>::value and EveryType<Pred, Ts...>::value);
    };
    
    template<template<class> class Pred, typename T = void, typename ...Ts>
    constexpr const inline bool every_type_v = EveryType<Pred, T, Ts...>::value;
    
    static_assert(not every_type_v<std::is_class, std::string, std::vector<int>, double>);
    
    
    template<typename T, template<class> class C>
    concept made_by = std::is_same_v<T, ChangeTemplate<C, T>>;
    
    template<typename T, template<class> class C>
    concept not_made_by = not made_by<T, C>;
    
    static_assert(not made_by<std::vector<int>, std::optional>);
    
    
    
    /**
     * A temporal class for implementing the concept unrelated_with<T>.
     * Note that the definition is unrelated_with<T> = ThisUnaryTypeConstructor<C>::template IsUnrelatedWith<T>::value,
     * which means that "This unary type constructor 'C' is not related with 'T'".
     * @see below, the definition of concept unrelated_with
     */
    template<template<class> class C>
    struct ThisUnaryTypeConstructor {
        
        template<typename T>
        struct IsUnrelatedWith;
        
        // If T is not "nested" type, then T is unrelated with C.
        
        template<non_typenested T>
        struct IsUnrelatedWith<T> {
            constexpr static const bool value = true;
        };
    
        // If T is "nested" type, say, T = D<T1, T2, ..., Tp>,
        // then T is unrelated with C IFF (T1 is unrelated with C and T2 is unrelated with C and ...
        // Tp is unrelated with C).
        
        template<typenested T>
        struct IsUnrelatedWith<T> {
            using TypeMove = ChangeTemplate<TempTypeHolder, T>;
            constexpr static const bool value =
                    (not made_by<T, C> and TypeMove::template value<IsUnrelatedWith>);
        };
    };

    /**
     * determines whether every sub-types of T is "unrelated_with" C. This process is recursive.
     * @tparam T any-type
     * @tparam C any unary type constructor
     */
    template<typename T, template<class> class C>
    concept unrelated_with = ThisUnaryTypeConstructor<C>::template IsUnrelatedWith<T>::value;
    
    template<typename T, template<class> class C>
    concept related_with = not unrelated_with<T, C>;
    
    static_assert(related_with<std::pair<std::vector<int>, std::pair<int, int>>, std::vector>);
    static_assert(unrelated_with<std::tuple<int, std::tuple<int, double, int>>, std::vector>);


    struct DeclApply {
        template<typename F, typename ...Ts>
        constexpr static auto declapply(F &&f, const std::tuple<Ts...> &tts) noexcept
            -> std::invoke_result_t<F, Ts...>;
    };

    template<typename F, typename ...Ts>
    using apply_result_t = decltype(DeclApply::declapply(std::declval<F>(), std::declval<std::tuple<Ts...>>()));



    /**
     * determines whether the decayed versions of T and U are same type
     * @tparam T, U any
     */
    template<typename T, typename U>
    concept same = std::is_same_v<std::decay_t<T>, std::decay_t<U>>
                and std::is_same_v<std::decay_t<U>, std::decay_t<T>>;

    template<typename T, typename U>
    concept different = not same<T, U>;
}

namespace fff {

    template<typename T>
    concept dereferencible = requires (T t) {
        *t;
    };
    
    template<template<class> class C>
    concept backpushable = requires (C<int> cont) {
        cont.push_back(0);
    };
    
    template<template<class> class C>
    concept insertible = requires (C<int> cont) {
        cont.insert(0);
    };
    
    template<typename T>
    concept maybetype = requires {
        T::is_maybe;
    };


}

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

    template<typename F, mr T>
    using apply_as_mr_result_t =
        decltype(DeclMr::declmr(std::declval<F>(), std::declval<T>()));
    
    inline MultiReturnFactory multi_return;
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
        constexpr auto &&operator()(T &&, Args &&...args) const noexcept {
            return std::invoke(IdentityAt<SZ - 1>(), std::forward<Args>(args)...);
        }
    };
    
    template<>
    struct IdentityAt<0> {
        /**
         * Template specification of IdentityAt<> (read upward)
         */
        template<class T, typename ...Args>
        constexpr T &&operator()(T &&t, Args &&...) const noexcept {
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
        constexpr auto operator()(T &&, Args &&...args) const noexcept {
            return std::invoke(CopyAt<SZ - 1>(), std::forward<Args>(args)...);
        }
    };
    
    template<>
    struct CopyAt<0> {
        /**
         * Template specification of IdentityAt<> (read upward)
         */
        template<class T, typename ...Args>
        constexpr T operator()(T &&t, Args &&...) const noexcept {
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
        constexpr void operator()(Args &&...) const noexcept {
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
            constexpr T operator()(Args &&...) const noexcept {
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
        T &get() noexcept(noexcept(new T())) {
            static T *data = new T();
            return *data;
        }
    };
}

/**
 * fff::(Null object class) impl
 * @todo null object implement
 */

namespace fff {
    struct NullAssignment_i {

    };
}

/*
 * fff::Once impl
 */
namespace fff {
    
    template<class F>
    class Once;
    
    template<class F>
        requires (not std::is_void_v<std::invoke_result_t<F>>)
    class Once<F> {
        friend class OnceFactory;
    
        [[no_unique_address]] F f;
        mutable std::invoke_result_t<F> memo;
        mutable bool flag;
    
        constexpr explicit Once(const F &f) noexcept : f(f), flag(false) {}
        constexpr explicit Once(F &&f) noexcept : f(std::move(f)), flag(false) {}

    public:
        constexpr auto operator()() const
            noexcept(noexcept(std::invoke(f)))
        -> std::invoke_result_t<F>
        {
            if (flag) {
                return memo;
            }
            flag = true;
            return memo = std::invoke(f);
        }
    };
    
    template<class F>
    requires std::is_void_v<std::invoke_result_t<F>>
    class Once<F> {
        friend class OnceFactory;
        
        [[no_unique_address]] F f;
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
    
    struct OnceFactory {
        template<std::invocable F>
        constexpr auto operator()(F &&f) noexcept
        -> Once<F>
        {
            return Once<F>{std::forward<F>(f)};
        }
    };
}

/* fff::Count impl */
namespace fff {
    
    template<class F>
    class Count {
        friend class CountFactory;
        
        [[no_unique_address]] F f;
        mutable int cnt;
        
        constexpr explicit Count(const F &f) noexcept : f(f), cnt(0) {}
        constexpr explicit Count(F &&f) noexcept : f(std::move(f)), cnt(0) {}
        
    public:
        template<class ...Args>
            requires std::invocable<F, Args...>
        constexpr auto operator()(Args ...args) const
            noexcept(noexcept(std::invoke(f, std::forward<Args>(args)...)))
        -> std::invoke_result_t<F, Args...>
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
        constexpr auto operator()(F &&f) const noexcept
        -> Count<F>
        {
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
 * fff::Fly impl
 */
namespace fff {
    template<class F>
    class Fly {
        std::unique_ptr<F> p;
        
    public:
        constexpr explicit Fly(const F &f) : p(new F(f)) {}
        constexpr explicit Fly(F &&f) : p(new F(std::move(f))) {}
        
        constexpr Fly(const Fly &fly) : p(new F(*fly.p)) {}
        constexpr Fly &operator=(const Fly &fly) {
            p = std::make_unique<F>(*fly.p);
            return *this;
        }
    
        template<class ...Args>
        requires std::invocable<F, Args...>
        constexpr auto operator()(Args &&...args) const
        noexcept(noexcept(std::invoke(*p, std::forward<Args>(args)...)))
        -> std::invoke_result_t<F, Args...>
        {
            return std::invoke(*p, std::forward<Args>(args)...);
        }

    };
    
    struct FlyFactory {
        template<class F>
        constexpr auto operator()(F &&f) const noexcept -> Fly<F> {
            return Fly<F>(std::forward<F>(f));
        }
    };
}

/**
 * A simple practice for CRTP pattern (If you are not interested, then just ignore...)
 */
namespace fff {

    template<typename T>
    class ObjectCounter {
        inline static int created_ = 0, alive_ = 0;

    public:
        constexpr ObjectCounter() noexcept {
            ++created_; ++alive_;
        }
        constexpr ~ObjectCounter() noexcept {
            --alive_;
        }

        [[nodiscard]] static int created() noexcept {
            return created_;
        }
        [[nodiscard]] static int alive() noexcept {
            return alive_;
        }

        constexpr explicit operator bool() const noexcept {
            return true;
        }
    };

    struct MyClass : ObjectCounter<MyClass> {};

    inline MyClass m, m2, m3;
}


/*
 * fff::go(), fff::stop impl
 */
namespace fff {
    struct Stop {};
    
    template<typename T>
    class On {
        static_assert(not std::is_void_v<T>, "The template type must not be void");
        static_assert(not_made_by<T, On>, "Nested type like On<On<T>> is not allowed");
        
        T data;
        
    public:
        constexpr explicit On(const T &t) noexcept(noexcept(T(t)))
                : data(t) {}
        constexpr explicit On(T &&t) noexcept(noexcept(T(t)))
                : data(t) {}
        
        constexpr explicit operator T() const noexcept {
            return data;
        }
        
        constexpr T &&operator>>(Stop) const noexcept {
            return std::move(data);
        }
    
        /**
         * Lift : (T -> U) -> (M<T> -> M<U>)
         * @tparam F function object type
         * @param f function object
         * @return any On<U> object which holds the value f(t)
         */
        template<std::invocable<T> F>
        constexpr auto operator>>(F &&f) const
            noexcept(noexcept(std::invoke(std::forward<F>(f), data)))
        -> On<std::invoke_result_t<F, T>>
        {
            return On<std::invoke_result_t<F, T>>(std::invoke(std::forward<F>(f), data));
        }
        
        constexpr T &operator*() & noexcept {
            return data;
        }
        constexpr const T &operator*() const & noexcept {
            return data;
        }
        constexpr T &&operator*() && noexcept {
            return std::move(data);
        }
        constexpr const T &&operator*() const && noexcept {
            return std::move(data);
        }
        constexpr T *operator->() noexcept {
            return &data;
        }
        constexpr const T *operator->() const noexcept {
            return &data;
        }
        
        constexpr T &&unpack() const noexcept {
            return std::move(data);
        }
        
        using value_type = T;
    };
    
    struct GoFactory {
        template<typename T>
        constexpr auto operator()(T t) const noexcept
        -> On<T>
        {
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
        
        using std::optional<T>::optional;
        
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
        constexpr auto operator()(T &&t) const noexcept
        -> Maybe<std::decay_t<T>>
        {
            return Maybe<std::decay_t<T>>(std::forward<T>(t));
        }
        
        template<typename T>
        constexpr auto make_nullopt() const noexcept
        -> Maybe<T>
        {
            return Maybe<T>();
        }
    };
}

/**
 * fff::NoThrow impl
 * @todo nothrow implement
 */
namespace fff {

    template<typename F>
    class NoThrow {
        [[no_unique_address]] F f;



    public:

    };
}

/**
 * fff::Log impl
 */
 
namespace fff {
    template<typename T>
    class Log {
        T data;
        std::stringstream log;
        
    public:
        constexpr explicit Log(const T &data) noexcept : data(data) {}
        constexpr explicit Log(T &&data) noexcept : data(std::move(data)) {}
        
        template<std::invocable<T> F>
        constexpr auto operator>>(F &&f) const
        noexcept(noexcept(1))
        -> Log<std::invoke_result_t<F, T>>
        {
            // @todo log implement
        }
        
        constexpr std::string emit() noexcept {
            std::string temp;
            log >> temp;
            
            return temp;
        }
    };
}

/**
 * fff::If impl
 */
namespace fff {
    template<template<class...> class Pred, class F1, class F2>
    class If {
        friend class IfFactory;
        
        [[no_unique_address]] F1 f1;
        [[no_unique_address]] F2 f2;
    
        template<class G1, class G2>
        constexpr If(G1 &&g1, G2 &&g2) noexcept
            : f1(std::forward<G1>(g1)), f2(std::forward<G2>(g2)) {}
    public:
        
        template<typename ...Args>
            requires Pred<F1, F2, Args...>::value
        constexpr auto operator()(Args &&...args) const
            noexcept(noexcept(std::invoke(f1, std::forward<Args>(args)...)))
        -> std::invoke_result_t<F1, Args...>
        {
            return std::invoke(f1, std::forward<Args>(args)...);
        }
    
        template<typename ...Args>
            requires (not Pred<F1, F2, Args...>::value)
        constexpr auto operator()(Args &&...args) const
            noexcept(noexcept(std::invoke(f2, std::forward<Args>(args)...)))
        -> std::invoke_result_t<F2, Args...>
        {
            return std::invoke(f2, std::forward<Args>(args)...);
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
        constexpr auto operator()(Fp &&...fp) const noexcept
        -> Overload<std::remove_reference_t<Fp>...>
        {
            return {std::forward<Fp>(fp)...};
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
        constexpr auto operator()(F &&f) const noexcept
        -> Concaten<std::remove_reference_t<F>>
        {
            return {std::forward<F>(f)};
        }
        
        template<class F, class ...Fp>
        constexpr auto operator()(F &&f, Fp &&...fp) const noexcept
        -> Concaten<std::remove_reference_t<F>, std::remove_reference_t<Fp>...>
        {
            return {std::forward<F>(f), operator()(std::forward<Fp>(fp)...)};
        }
    };
}

/*
 * fff::Parallel impl
 */

namespace fff {
    template<class F, class ...Fp>
    class Parallel;
    
    template<class F>
    class Parallel<F> {
        friend class ParallelFactory;
        
        [[no_unique_address]] F f;
        
        constexpr explicit Parallel(const F &f) noexcept : f(f) {}
        constexpr explicit Parallel(F &&f) noexcept : f(std::move(f)) {}
        
    public:
        template<typename ...Args>
            requires std::invocable<F, Args...>
        constexpr auto operator()(Args &&...args) const &
            noexcept(noexcept(std::invoke(f, std::forward<Args>(args)...)))
        -> std::invoke_result_t<F, Args...>
        {
            return std::invoke(f, std::forward<Args>(args)...);
        }
    
        template<typename ...Args>
            requires std::invocable<F, Args...>
        constexpr auto operator()(Args &&...args) const &&
            noexcept(noexcept(std::invoke(std::move(f), std::forward<Args>(args)...)))
        -> std::invoke_result_t<F, Args...>
        {
            return std::invoke(std::move(f), std::forward<Args>(args)...);
        }
    };
    
    template<class F, class ...Fp>
    class Parallel {
        friend class ParallelFactory;
        
        [[no_unique_address]] F f;
        [[no_unique_address]] Parallel<Fp...> pfp;
    
        template<class U1, class U2>
        constexpr Parallel(U1 &&f, U2 &&pfp) noexcept
            : f(std::forward<U1>(f)), pfp(std::forward<U2>(pfp)) {}
        
    public:
        template<typename ...Args>
        requires std::invocable<F, Args...>
        constexpr auto operator()(Args &&...args) const &
        noexcept(noexcept(std::invoke(f, std::forward<Args>(args)...)))
        -> std::invoke_result_t<F, Args...>
        {
            return std::invoke(f, std::forward<Args>(args)...);
        }
    
        template<typename ...Args>
        requires std::invocable<F, Args...>
        constexpr auto operator()(Args &&...args) const &&
        noexcept(noexcept(std::invoke(std::move(f), std::forward<Args>(args)...)))
        -> std::invoke_result_t<F, Args...>
        {
            return std::invoke(std::move(f), std::forward<Args>(args)...);
        }
    
        template<typename ...Args>
        requires (not std::invocable<F, Args...>
                and std::invocable<Parallel<Fp...>, Args...>)
        constexpr auto operator()(Args &&...args) const &
        noexcept(noexcept(std::invoke(pfp, std::forward<Args>(args)...)))
        -> std::invoke_result_t<Parallel<Fp...>, Args...>
        {
            return std::invoke(pfp, std::forward<Args>(args)...);
        }
    
        template<typename ...Args>
        requires (not std::invocable<F, Args...>
                  and std::invocable<Parallel<Fp...>, Args...>)
        constexpr auto operator()(Args &&...args) const &&
        noexcept(noexcept(std::invoke(std::move(pfp), std::forward<Args>(args)...)))
        -> std::invoke_result_t<Parallel<Fp...>, Args...>
        {
            return std::invoke(std::move(pfp), std::forward<Args>(args)...);
        }
    };
    
    struct ParallelFactory {
        template<class F>
        constexpr auto operator()(F &&f) const noexcept -> Parallel<std::decay_t<F>>
        {
            return Parallel<std::decay_t<F>>(std::forward<F>(f));
        }
    
        template<class F, class ...Fp>
        constexpr auto operator()(F &&f, Fp &&...fp) const noexcept
        -> Parallel<std::decay_t<F>, std::decay_t<Fp>...>
        {
            return Parallel<std::decay_t<F>, std::decay_t<Fp>...>
                    (std::forward<F>(f), operator()(std::forward<Fp>(fp)...));
        }
    };
}

/*
 * fff::Pipeline impl
 */
namespace fff {
    
    template<class F, class ...Fp>
    struct Pipeline;
    
    template<class F>
    class Pipeline<F> {
        friend class PipelineFactory;
        
        [[no_unique_address]] F f;

    public:
        constexpr explicit Pipeline(const F &f) noexcept : f(f) {}
        constexpr explicit Pipeline(F &&f) noexcept : f(std::move(f)) {}

        template<class ...Args>
        requires std::invocable<F, Args...>
        constexpr auto operator()(Args &&...args) const &
        noexcept(noexcept(std::invoke(f, std::forward<Args>(args)...)))
        -> std::invoke_result_t<F, Args...>
        {
            return std::invoke(f, std::forward<Args>(args)...);
        }
    
        template<class ...Args>
        requires std::invocable<F, Args...>
        constexpr auto operator()(Args &&...args) const &&
        noexcept(noexcept(std::invoke(std::move(f), std::forward<Args>(args)...)))
        -> std::invoke_result_t<F, Args...>
        {
            return std::invoke(std::move(f), std::forward<Args>(args)...);
        }
    
        template<class G>
        constexpr auto operator>>(G &&g) const & noexcept -> Pipeline<F, std::decay_t<G>> {
            return Pipeline<F, std::decay_t<G>>{f, std::forward<G>(g)};
        }
    
        template<class G>
        constexpr auto operator>>(G &&g) const && noexcept -> Pipeline<F, std::decay_t<G>> {
            return Pipeline<F, std::decay_t<G>>{std::move(f), std::forward<G>(g)};
        }
    };
    
    template<class F1, class ...Fp>
    class Pipeline {
        friend class PipelineFactory;
        
        [[no_unique_address]] F1 f1;
        [[no_unique_address]] Pipeline<Fp...> f2;

    public:
        template<class U1, class U2>
        constexpr Pipeline(U1 &&f1, U2 &&f2) noexcept
            : f1(std::forward<U1>(f1)), f2(std::forward<U2>(f2)) {}

        template<class ...Args>
            requires not_mr<std::invoke_result_t<F1, Args...>>
        constexpr auto operator()(Args &&...args) const &
            noexcept(noexcept(std::invoke(f2, std::invoke(f1, std::forward<Args>(args)...))))
        -> std::invoke_result_t<Pipeline<Fp...>, std::invoke_result_t<F1, Args...>>
        {
            return std::invoke(f2, std::invoke(f1, std::forward<Args>(args)...));
        }
        
        template<class ...Args>
            requires not_mr<std::invoke_result_t<F1, Args...>>
        constexpr auto operator()(Args &&...args) const &&
            noexcept(noexcept(std::invoke(std::move(f2), std::invoke(std::move(f1), std::forward<Args>(args)...))))
        -> std::invoke_result_t<Pipeline<Fp...>, std::invoke_result_t<F1, Args...>>
        {
            return std::invoke(std::move(f2), std::invoke(std::move(f1), std::forward<Args>(args)...));
        }
    
        template<class ...Args>
            requires mr<std::invoke_result_t<F1, Args...>>
        constexpr auto operator()(Args &&...args) const &
        noexcept(noexcept(std::apply(f2, std::invoke(f1, std::forward<Args>(args)...).to_tuple())))
        -> apply_as_mr_result_t<Pipeline<Fp...>, std::invoke_result_t<F1, Args...>>
        {
            return std::apply(f2, std::invoke(f1, std::forward<Args>(args)...).to_tuple());
        }
    
        template<class ...Args>
            requires mr<std::invoke_result_t<F1, Args...>>
        constexpr auto operator()(Args &&...args) const &&
        noexcept(noexcept(std::apply(std::move(f2), std::invoke(std::move(f1), std::forward<Args>(args)...).to_tuple())))
        -> apply_as_mr_result_t<Pipeline<Fp...>, std::invoke_result_t<F1, Args...>>
        {
            return std::apply(std::move(f2), std::invoke(std::move(f1), std::forward<Args>(args)...).to_tuple());
        }
    
        template<class G>
        constexpr auto operator>>(G &&g) const & noexcept
        -> Pipeline<F1, Fp..., std::decay_t<G>>
        {
            return Pipeline<F1, Fp..., std::decay_t<G>>{f1, f2 >> std::forward<G>(g)};
        }
    
        template<class G>
        constexpr auto operator>>(G &&g) const && noexcept
        -> Pipeline<F1, Fp..., std::decay_t<G>>
        {
            return Pipeline<F1, Fp..., std::decay_t<G>>{std::move(f1), std::move(f2) >> std::forward<G>(g)};
        }
    };
    
    struct PipelineFactory {
        template<class F>
        constexpr auto operator()(F &&f) const noexcept
        -> Pipeline<std::decay_t<F>>
        {
            return Pipeline<std::decay_t<F>>{std::forward<F>(f)};
        }
        
        template<class F, class ...Fp>
        constexpr auto operator()(F &&f, Fp &&...fp) const noexcept
        -> Pipeline<std::decay_t<F>, std::decay_t<Fp>...>
        {
            return Pipeline<std::decay_t<F>, std::decay_t<Fp>...>
                    {std::forward<F>(f), operator()(std::forward<Fp>(fp)...)};
        }
    
        template<class F>
        constexpr auto operator>>(F &&f) const noexcept
        -> Pipeline<std::decay_t<F>>
        {
            return Pipeline<std::decay_t<F>>{std::forward<F>(f)};
        }
    };
}

/*
 * fff::StaticBind impl
 */
namespace fff {

}

/**
 * fff::(Some Unary Operators)
 */

namespace fff {
    struct GetAddress {
        template<typename T>
        constexpr auto operator()(const T &t) const noexcept -> decltype(&t) {
            return &t;
        }
    };

    struct Dereference {
        template<dereferencible T>
        constexpr auto operator()(T &&t) const noexcept -> decltype(*t) {
            return *t;
        }
    };

    template<typename T>
    struct ConvertTo {
        template<std::convertible_to<T> U>
        constexpr auto operator()(U &&u) const noexcept -> T {
            return static_cast<T>(std::forward<U>(u));
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
    
    inline FlyFactory               fly;
    
    inline MaybeFactory             maybe;
    inline Stop                     stop;
    inline GoFactory                go;
    
    inline ConcatenFactory          concaten;
    inline ParallelFactory          parallel;
    inline OverloadFactory          overload;
    inline PipelineFactory          pipeline;
}

namespace fff {
    struct Package {
        [[no_unique_address]] MultiReturnFactory multi_return{};
    
        [[no_unique_address]] Each each{};
        [[no_unique_address]] Map map{};
        [[no_unique_address]] Filter filter{};
        [[no_unique_address]] Reject reject{};
    
        [[no_unique_address]] Some some{};
        [[no_unique_address]] Every every{};
        [[no_unique_address]] None none{};
    
        [[no_unique_address]] AlwaysTrue always_true{};
        [[no_unique_address]] AlwaysFalse always_false{};
    
        [[no_unique_address]] IdentityAt<0> identity{};
        [[no_unique_address]] CopyAt<0> copy{};
    
        [[no_unique_address]] OnceFactory once{};
        [[no_unique_address]] CountFactory count{};
        [[no_unique_address]] IdGiverFactory id_giver{};
    
        [[no_unique_address]] FlyFactory fly;
    
        [[no_unique_address]] MaybeFactory maybe{};
        [[no_unique_address]] Stop stop{};
        [[no_unique_address]] GoFactory go{};
    
        [[no_unique_address]] ConcatenFactory concaten{};
        [[no_unique_address]] ParallelFactory parallel{};
        [[no_unique_address]] OverloadFactory overload{};
        [[no_unique_address]] PipelineFactory pipeline{};
        
        Package() = default;
    };
}

static_assert(std::is_empty_v<fff::Package>, "the Package class should be empty");

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