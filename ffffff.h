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

   struct constexpr_determination {
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
   using type_of = std::decay_t<decltype(V)>;

   template<unsigned int N, typename T = void, typename ...U>
   struct among_impl {
       using type = typename among_impl<N - 1, U...>::type;
   };

   template<typename T, typename ...U>
   struct among_impl<0, T, U...> {
       using type = T;
   };

   template<typename ...T>
   struct among {
       template<unsigned int N>
       using get = typename among_impl<N, T...>::type;
   };

   static_assert(std::is_same_v<int, among<char, double, int>::get<2>>);

   template<unsigned int N, typename ...T>
   using nth_among = typename among<T...>::template get<N>;

   template<unsigned int N>
   struct type_at {
       template<template<class, class...> class C, typename T, typename ...U>
       constexpr static auto of(C<T, U...>) noexcept -> nth_among<N, T, U...>;
   };

   template<typename T, unsigned int N = 0>
   using value_type_t = decltype(type_at<N>::of(std::declval<T>()));

   static_assert(std::is_same_v<int, value_type_t<std::unordered_map<double, int>, 1>>);

   /**
    * Determines whether T is made by type constructor that holds ONLY types, like std::vector\<int> or std::pair\<int, int>
    * @warning If T is made by type constructor but holds non-type param (like std::array\<int, 3>), type_nested\<T> is evaluated as false
    * @example std::pair\<std::vector\<int>, int> is type_nested
    * @example int is not type_nested
    * @example std::array\<int, 3> is NOT type_nested
    * @tparam T any
    */
   template<typename T>
   concept type_nested =
       requires {
           type_at<0>::of(std::declval<T>());
       };

   template<typename T>
   concept non_type_nested =
       not type_nested<T>;

   static_assert(non_type_nested<int>);
   static_assert(type_nested<std::vector<int>>);
   static_assert(type_nested<std::pair<int, int>>);
   static_assert(non_type_nested<std::array<int, 3>>);

   /**
    * A concept determines whether the given C is a unary predicate.
    * @tparam C Any unary type constructor
    * @example unary_pred\<std::is_class> is true, since std::is_class\<T>::value is evaluated as bool
    * @example unary_pred\<std::vector> is false
    */
   template<template<class> class C>
   concept unary_pred = requires {
                            { C<int>::value } -> std::convertible_to<bool>;
                            constexpr_determination::is_consteval_var(C<int>::value);
                        };

   static_assert(unary_pred<std::is_class> and not unary_pred<std::vector>);



   /**
    * A temporal type holder that holds any type.
    * @tparam T, Ts any typename
    * @example temp_type_holder\<int, double, char> is valid. (There is no requirement)
    * @using type, the type alias for T
    * @using next, the type alias for temp_type_holder\<Ts...> i.e. a class that temporally holds all types except T
    * @example temp_type_holder\<int, double, char>::next::next::type == char
    */
   template<typename T, typename ...Ts>
   struct temp_type_holder {
       using type = T;
       using next = temp_type_holder<Ts...>;

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
   struct temp_type_holder<T> {
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

   /**
    * @see change_template_t (just below)
    */
   template<template<class...> class C>
   struct template_replace {
       template<template<class...> class D, typename ...T>
       constexpr static auto instead_of(D<T...>) noexcept -> C<T...>;
   };

   /**
    * change_template_t\<C, D\<T>> = C\<T>
    * @example change_template_t\<std::vector, std::optional\<int>> == std::vector\<int>
    */
   template<template<class...> class C, typename T>
   using change_template_t = decltype(template_replace<C>::instead_of(std::declval<T>()));

   static_assert(std::is_same_v<std::vector<int>, change_template_t<std::vector, std::optional<int>>>);

   /**
    * @see change_value_type_t (just below)
    */
   template<typename T>
   struct value_type_replace {
       template<template<class, class...> class C, typename U>
       constexpr static auto instead_of(C<U>) noexcept -> C<T>;
   };

   /**
    * change_template_type_t\<T, C\<U>> == C\<T>
    * @example change_template_type_t\<int, std::vector\<double>> == std::vector\<int>
    */
   template<typename T, type_nested U>
   using change_value_type_t = decltype(value_type_replace<T>::instead_of(std::declval<U>()));

   template<template<class> class Pred, typename T = void, typename ...Ts>
       requires unary_pred<Pred>
   struct every_type_satisfies {
       constexpr const static bool value =
           std::is_void_v<T> or (Pred<T>::value and every_type_satisfies<Pred, Ts...>::value);
   };

   template<template<class> class Pred, typename T = void, typename ...Ts>
   constexpr const inline bool every_type_satisfies_v = every_type_satisfies<Pred, T, Ts...>::value;

   static_assert(not every_type_satisfies_v<std::is_class, std::string, std::vector<int>, double>);


   template<typename T, template<class, class...> class C>
   concept made_by = std::is_same_v<std::decay_t<T>,
   change_template_t <C, std::decay_t<T>>>;

   template<typename T, template<class, class...> class C>
   concept not_made_by = not made_by<T, C>;

   static_assert(not made_by<std::vector<int>, std::optional>);

   /**
    * @see below, the definition of concept unrelated_with
    */
   template<template<class> class C>
   struct is_unrelated_with_impl {

       template<typename T>
       struct is_unrelated_with : std::true_type {};

       /**
        * If T is not "type_nested" type, then T is unrelated with C.\n
        * If T is "type_nested" type, say, T = D\<T1, T2, ..., Tp>, then T is unrelated with C IFF (T1 is unrelated with C and T2 is unrelated with C and ... Tp is unrelated with C).
        * @tparam T any
        */
       template<type_nested T>
       struct is_unrelated_with<T> {
           using TypeMove = change_template_t<temp_type_holder, T>;
           constexpr static bool value =
               (not made_by<T, C> and TypeMove::template value<is_unrelated_with>);
       };
   };

   /**
    * determines whether every sub-types of T is "unrelated_with" C. This process is recursive.
    * @tparam T any-type
    * @tparam C any unary type constructor
    */
   template<typename T, template<class> class C>
   concept unrelated_with = is_unrelated_with_impl<C>::template is_unrelated_with<T>::value;

   template<typename T, template<class> class C>
   concept related_with = not unrelated_with<T, C>;

   static_assert(related_with<std::pair<std::vector<int>, std::pair<int, int>>, std::vector>);
   static_assert(unrelated_with<std::tuple<int, std::tuple<int, double, int>>, std::vector>);


   /**
    * @see apply_result_t (just below)
    */
   struct declapply_impl {
       template<typename F, typename ...Ts>
       constexpr static auto declapply(F &&f, const std::tuple<Ts...> &tts) noexcept
           -> std::invoke_result_t<F, Ts...>;
   };

   /**
    * the "apply" version of std::invoke_result_t.
    * @tparam F function type
    * @tparam Ts function argument type param pack
    */
   template<typename F, typename ...Ts>
   using apply_result_t = decltype(declapply_impl::declapply(std::declval<F>(), std::declval<std::tuple<Ts...>>()));

   /**
    * determines whether the decayed versions of T and U are similar type
    * @tparam T, U any
    */
   template<typename T, typename U>
   concept similar = std::is_same_v<std::decay_t<T>, std::decay_t<U>>
       and std::is_same_v<std::decay_t<U>, std::decay_t<T>>;

   template<typename T, typename U>
   concept different = not similar<T, U>;

   static_assert(similar<int*, int[]>);



   template<typename T, template<class> class C>
   concept derived_as_crtp = std::derived_from<T, C<T>>;

   template<typename F, typename ...Args>
   concept nonvoid_invocable = std::invocable<F, Args...>
       and not std::is_void_v<std::invoke_result_t<F, Args...>>;

   template<typename F, typename ...Args>
   concept void_invocable = std::invocable<F, Args...>
       and std::is_void_v<std::invoke_result_t<F, Args...>>;

   template<typename T>
   concept class_as_value =
       requires {
           T::value;
       };

   template<typename T>
   concept class_as_type =
       requires {
           typename T::type;
       };
}

namespace fff {

   template<typename T>
   concept dereferencible = requires (T t) {*t;};

   template<typename T>
   concept negatable = requires (T t) {!t;};

   template<typename T>
   concept flippable = requires (T t) {~t;};

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


   template<typename T>
   class TD;
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

   constexpr inline MultiReturnFactory multi_return;
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
       constexpr std::decay_t<T> operator()(T &&t, Args &&...) const noexcept {
           return t;
       }
   };

   using Copy = CopyAt<0>;
   constexpr inline Copy copy;
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

   constexpr inline AlwaysTrue always_true;
   constexpr inline AlwaysFalse always_false;
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

namespace fff {

    template<class, class...>
    struct auto_decl {};

    /**
     * A CRTP Pattern that gives "operator()" function.\n
     * To implement "operator()" function, it is sufficient to implement a static 'call_impl' template function.
     * @example template\<class F> class Foo : Callable_i\<F, Foo> { (implements...) }
     */
    template<typename F, template<class, class...> class C,
            template<class, class...> class TypeDeduction = auto_decl,
            typename Derived = C<F>>
    class Callable_i {
    public:
        using value_type = F;

        template<typename ...Args>
        constexpr auto operator()(Args &&...args) &
            noexcept(noexcept(Derived::call_impl(*static_cast<Derived*>(this), std::forward<Args>(args)...)))
                -> typename TypeDeduction<F, Args...>::type
        {
            return Derived::call_impl(*static_cast<Derived*>(this), std::forward<Args>(args)...);
        }

        template<typename ...Args>
        constexpr auto operator()(Args &&...args) const &
            noexcept(noexcept(Derived::call_impl(*static_cast<Derived*>(this), std::forward<Args>(args)...)))
                -> typename TypeDeduction<F, Args...>::type
        {
            return Derived::call_impl(*static_cast<Derived*>(this), std::forward<Args>(args)...);
        }

        template<typename ...Args>
        constexpr auto operator()(Args &&...args) &&
            noexcept(noexcept(Derived::call_impl(std::move(*static_cast<Derived*>(this)), std::forward<Args>(args)...)))
                -> typename TypeDeduction<F, Args...>::type
        {
            return Derived::call_impl(std::move(*static_cast<Derived*>(this)), std::forward<Args>(args)...);
        }

        template<typename ...Args>
        constexpr auto operator()(Args &&...args) const &&
            noexcept(noexcept(Derived::call_impl(std::move(*static_cast<Derived*>(this)), std::forward<Args>(args)...)))
                -> typename TypeDeduction<F, Args...>::type
        {
            return Derived::call_impl(std::move(*static_cast<Derived*>(this)), std::forward<Args>(args)...);
        }
    };

    template<typename F, template<class, class...> class C>
    class Callable_i<F, C, auto_decl> {
    public:
        using Derived = C<F>;
        using value_type = F;

        template<typename ...Args>
        constexpr auto operator()(Args &&...args) &
            noexcept(noexcept(Derived::call_impl(*static_cast<Derived*>(this), std::forward<Args>(args)...)))
        {
            return Derived::call_impl(*static_cast<Derived*>(this), std::forward<Args>(args)...);
        }

        template<typename ...Args>
        constexpr auto operator()(Args &&...args) const &
            noexcept(noexcept(Derived::call_impl(*static_cast<Derived*>(this), std::forward<Args>(args)...)))
        {
            return Derived::call_impl(*static_cast<Derived*>(this), std::forward<Args>(args)...);
        }

        template<typename ...Args>
        constexpr auto operator()(Args &&...args) &&
            noexcept(noexcept(Derived::call_impl(std::move(*static_cast<Derived*>(this)), std::forward<Args>(args)...)))
        {
            return Derived::call_impl(std::move(*static_cast<Derived*>(this)), std::forward<Args>(args)...);
        }

        template<typename ...Args>
        constexpr auto operator()(Args &&...args) const &&
            noexcept(noexcept(Derived::call_impl(std::move(*static_cast<Derived*>(this)), std::forward<Args>(args)...)))
        {
            return Derived::call_impl(std::move(*static_cast<Derived*>(this)), std::forward<Args>(args)...);
        }
    };
};

/**
* fff::(Null object class) impl
*/

namespace fff {

   /**
    * A null type that works as a "Null Object".
    * Use CRTP pattern to inherit this type and make any "Null Object" you want.
    */
   template<typename T = void>
   struct Null_i {
       const static bool nullity = true;
   };

   using Null = Null_i<>;

   template<typename T>
   using Null_or_t = std::conditional_t<std::is_void_v<T>, Null, T>;

   template<typename F, typename ...Args>
   struct NullLifted_TD_Impl {
       using type = Null_or_t<std::invoke_result_t<F, Args...>>;
   };

   template<class F>
   class NullLifted : public Callable_i<F, NullLifted, NullLifted_TD_Impl> {
       friend Callable_i<F, NullLifted, NullLifted_TD_Impl>;
       friend class NullLiftFactory;

       [[no_unique_address]] F f;

       constexpr explicit NullLifted(const F &f) noexcept : f(f) {}
       constexpr explicit NullLifted(F &&f) noexcept : f(std::move(f)) {}

       template<similar<NullLifted> Self, typename ...Args>
       constexpr static auto call_impl(Self &&self, Args &&...args)
           noexcept(noexcept(std::invoke(std::forward<Self>(self).f_fwd(), std::forward<Args>(args)...)))
               -> Null_or_t<std::invoke_result_t<F, Args...>>
       {
           if constexpr (std::is_void_v<std::invoke_result_t<F, Args...>>) {
               std::invoke(std::forward<Self>(self).f_fwd(), std::forward<Args>(args)...);
               return Null{};
           } else {
               return std::invoke(std::forward<Self>(self).f_fwd(), std::forward<Args>(args)...);
           }
       }

       constexpr       F &  f_fwd()       &  noexcept {return f;}
       constexpr const F &  f_fwd() const &  noexcept {return f;}
       constexpr       F && f_fwd()       && noexcept {return std::move(f);}
       constexpr const F && f_fwd() const && noexcept {return std::move(f);}
   };

   struct NullLiftFactory {
       template<typename F>
       constexpr auto operator()(F &&f) const noexcept -> NullLifted<F> {
           return NullLifted<F>{std::forward<F>(f)};
       }
   };

   constexpr inline NullLiftFactory null_lift;

}

/**
 * fff::static_bind impl
 */

namespace fff {
    template<auto v>
    struct value_holder {
        using type = decltype(v);
        constexpr static decltype(v) value = v;
    };

    template<class_as_value ...ValueHolders>
    struct static_l_bind_TD_impl {
        template<typename F, typename ...Args>
        struct Inner {
            using type = std::invoke_result_t<F, typename ValueHolders::type..., Args...>;
        };
    };

    template<typename F, class_as_value ...ValueHolders>
    class static_l_bind_f
        : public Callable_i<F, static_l_bind_f,
                            static_l_bind_TD_impl<ValueHolders...>::template Inner,
                            static_l_bind_f<F, ValueHolders...>> {

        template<auto ...vp>
        friend class static_l_bind_factory;

        friend Callable_i<F, static_l_bind_f,
                          static_l_bind_TD_impl<ValueHolders...>::template Inner,
                          static_l_bind_f<F, ValueHolders...>>;

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
           -> Once<std::decay_t<F>>
       {
           return Once<std::decay_t<F>>{std::forward<F>(f)};
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
       explicit Fly(const F &f) : p(new F(f)) {}
       explicit Fly(F &&f) : p(new F(std::move(f))) {}

       Fly(const Fly &fly) : p(new F(*fly.p)) {}
       Fly &operator=(const Fly &fly) {
           p = std::make_unique<F>(*fly.p);
           return *this;
       }
       Fly &operator=(Fly &&) noexcept = default;

       template<class ...Args>
           requires std::invocable<F, Args...>
       auto operator()(Args &&...args) const
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
           noexcept
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
           -> Overload<std::decay_t<Fp>...>
       {
           return {std::forward<Fp>(fp)...};
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

   public:
       constexpr explicit Parallel(const F &f) noexcept : f(f) {}
       constexpr explicit Parallel(F &&f) noexcept : f(std::move(f)) {}

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
       constexpr auto operator()(Args &&...args) &&
           noexcept(noexcept(std::invoke(std::move(f), std::forward<Args>(args)...)))
               -> std::invoke_result_t<F, Args...>
       {
           return std::invoke(std::move(f), std::forward<Args>(args)...);
       }

       template<typename ...Args>
           requires std::invocable<F, Args...>
       constexpr auto operator()(Args &&...args) const &&
           noexcept(noexcept(std::invoke(std::move(f), std::forward<Args>(args)...)))
               -> std::invoke_result_t<F, Args...>
       {
           return std::invoke(std::move(f), std::forward<Args>(args)...);
       }

       template<typename G>
       constexpr auto make_chain(G &&g) const & noexcept -> Parallel<F, std::decay_t<G>> {
           return Parallel<F, std::decay_t<G>>{f, std::forward<G>(g)};
       }

       template<typename G>
       constexpr auto make_chain(G &&g) && noexcept -> Parallel<F, std::decay_t<G>> {
           return Parallel<F, std::decay_t<G>>{std::move(f), std::forward<G>(g)};
       }

       template<typename G>
       constexpr auto make_chain(G &&g) const && noexcept -> Parallel<F, std::decay_t<G>> {
           return Parallel<F, std::decay_t<G>>{std::move(f), std::forward<G>(g)};
       }
   };

   template<class F, class ...Fp>
   class Parallel {
       friend class ParallelFactory;

       [[no_unique_address]] F f;
       [[no_unique_address]] Parallel<Fp...> pfp;

   public:
       template<class U1, class U2>
       constexpr Parallel(U1 &&f, U2 &&pfp) noexcept
           : f(std::forward<U1>(f)), pfp(std::forward<U2>(pfp)) {}

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
       constexpr auto operator()(Args &&...args) &&
           noexcept(noexcept(std::invoke(std::move(f), std::forward<Args>(args)...)))
               -> std::invoke_result_t<F, Args...>
       {
           return std::invoke(std::move(f), std::forward<Args>(args)...);
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
       constexpr auto operator()(Args &&...args) &&
           noexcept(noexcept(std::invoke(std::move(pfp), std::forward<Args>(args)...)))
               -> std::invoke_result_t<Parallel<Fp...>, Args...>
       {
           return std::invoke(std::move(pfp), std::forward<Args>(args)...);
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

       template<typename G>
       constexpr auto make_chain(G &&g) const & noexcept
           -> Parallel<F, Fp..., std::decay_t<G>>
       {
           return Parallel<F, Fp..., std::decay_t<G>>{f, pfp.make_chain(std::forward<G>(g))};
       }

       template<typename G>
       constexpr auto make_chain(G &&g) && noexcept
           -> Parallel<F, Fp..., std::decay_t<G>>
       {
           return Parallel<F, Fp..., std::decay_t<G>>{std::move(f), pfp.make_chain(std::forward<G>(g))};
       }

       template<typename G>
       constexpr auto make_chain(G &&g) const && noexcept
           -> Parallel<F, Fp..., std::decay_t<G>>
       {
           return Parallel<F, Fp..., std::decay_t<G>>{std::move(f), pfp.make_chain(std::forward<G>(g))};
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

       template<typename F>
       constexpr auto make_chain(F &&f) const noexcept -> Parallel<std::decay_t<F>> {
           return Parallel<F>{std::forward<F>(f)};
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
       constexpr auto operator()(Args &&...args) &&
           noexcept(noexcept(std::invoke(std::move(f), std::forward<Args>(args)...)))
               -> std::invoke_result_t<F, Args...>
       {
           return std::invoke(std::move(f), std::forward<Args>(args)...);
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
       constexpr auto operator>>(G &&g) && noexcept -> Pipeline<F, std::decay_t<G>> {
           return Pipeline<F, std::decay_t<G>>{std::move(f), std::forward<G>(g)};
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
       constexpr auto operator()(Args &&...args) &&
           noexcept(noexcept(std::invoke(std::move(f2), std::invoke(std::move(f1), std::forward<Args>(args)...))))
               -> std::invoke_result_t<Pipeline<Fp...>, std::invoke_result_t<F1, Args...>>
       {
           return std::invoke(std::move(f2), std::invoke(std::move(f1), std::forward<Args>(args)...));
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
       constexpr auto operator()(Args &&...args) &&
           noexcept(noexcept(std::apply(std::move(f2), std::invoke(std::move(f1), std::forward<Args>(args)...).to_tuple())))
               -> apply_as_mr_result_t<Pipeline<Fp...>, std::invoke_result_t<F1, Args...>>
       {
           return std::apply(std::move(f2), std::invoke(std::move(f1), std::forward<Args>(args)...).to_tuple());
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
       constexpr auto operator>>(G &&g) && noexcept
           -> Pipeline<F1, Fp..., std::decay_t<G>>
       {
           return Pipeline<F1, Fp..., std::decay_t<G>>{std::move(f1), std::move(f2) >> std::forward<G>(g)};
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

   inline constexpr GetAddress get_address;

   struct Dereference {
       template<dereferencible T>
       constexpr auto operator()(T &&t) const
           noexcept(noexcept(*t))
               -> decltype(*t) {
           return *t;
       }
   };

   inline constexpr Dereference dereference;

   struct Negate {
       template<negatable T>
       constexpr auto operator()(T &&t) const
           noexcept(noexcept(!t))
               -> decltype(!t) {
           return !t;
       }
   };

   inline constexpr Negate negate;

   struct Flip {
       template<flippable T>
       constexpr auto operator()(T &&t) const noexcept -> decltype(~t) {
           return ~t;
       }
   };

   inline constexpr Flip flip;

   template<typename T>
   struct ConvertTo {
       template<std::convertible_to<T> U>
       constexpr auto operator()(U &&u) const noexcept -> T {
           return static_cast<T>(std::forward<U>(u));
       }
   };

   template<typename T>
   inline constexpr ConvertTo<T> convert_to;
}


namespace fff {
   /**
    * An interface that offers lift() method. \n
    * To use this, the derived class should implement lift_impl() method.
    * @tparam T value-type
    * @tparam C a type constructor that will offer lift() method
    */
   template<typename T, template<class, class...> class C>
   class Lift_i {
   public:
       using Derived = C<T>;
       using value_type = T;

       template<std::invocable<T> F>
       constexpr auto lift(F &&f) &
           noexcept(noexcept(Derived::lift_impl(*static_cast<Derived*>(this), std::forward<F>(f))))
               -> C<std::invoke_result_t<F, T>>
       {
           return Derived::lift_impl(*static_cast<Derived*>(this), std::forward<F>(f));
       }

       template<std::invocable<T> F>
       constexpr auto lift(F &&f) const &
           noexcept(noexcept(Derived::lift_impl(*static_cast<Derived*>(this), std::forward<F>(f))))
               -> C<std::invoke_result_t<F, T>>
       {
           return Derived::lift_impl(*static_cast<Derived*>(this), std::forward<F>(f));
       }

       template<std::invocable<T> F>
       constexpr auto lift(F &&f) &&
           noexcept(noexcept(Derived::lift_impl(std::move(*static_cast<Derived*>(this)), std::forward<F>(f))))
               -> C<std::invoke_result_t<F, T>>
       {
           return Derived::lift_impl(std::move(*static_cast<Derived*>(this)), std::forward<F>(f));
       }

       template<std::invocable<T> F>
       constexpr auto lift(F &&f) const &&
           noexcept(noexcept(Derived::lift_impl(std::move(*static_cast<Derived*>(this)), std::forward<F>(f))))
               -> C<std::invoke_result_t<F, T>>
       {
           return Derived::lift_impl(std::move(*static_cast<Derived*>(this)), std::forward<F>(f));
       }
   };

   template<typename T, template<class, class...> class C>
   class FlatLift_i {
   public:
       using Derived = C<T>;
       using value_type = T;

       template<std::invocable<T> F>
           requires made_by<std::invoke_result_t<F, T>, C>
           constexpr auto flat_lift(F &&f)
               noexcept(noexcept(static_cast<Derived*>(this)->flatlift_impl(std::forward<F>(f))))
                   -> std::invoke_result_t<F, T>
       {
           return static_cast<Derived*>(this)->flatlift_impl(std::forward<F>(f));
       }

       template<std::invocable<T> F>
           requires made_by<std::invoke_result_t<F, T>, C>
           constexpr auto flat_lift(F &&f) const
           noexcept(noexcept(static_cast<Derived*>(this)->flatlift_impl(std::forward<F>(f))))
               -> std::invoke_result_t<F, T>
       {
           return static_cast<Derived*>(this)->flatlift_impl(std::forward<F>(f));
       }
   };

   template<typename T>
   struct Test : Lift_i<T, Test> {
       T data;

       constexpr explicit Test(T data) : data(data) {}

       template<similar<Test> Self, std::invocable<T> F>
       constexpr static auto lift_impl(Self &&self, F &&f)
           -> Test<std::invoke_result_t<F, T>>
       {
           return Test<std::invoke_result_t<F, T>>(std::invoke(std::forward<F>(f), std::forward<Self>(self).data_fwd()));
       }

       constexpr       T &  data_fwd()       &  noexcept {return data;}
       constexpr const T &  data_fwd() const &  noexcept {return data;}
       constexpr       T && data_fwd()       && noexcept {return std::move(data);}
       constexpr const T && data_fwd() const && noexcept {return std::move(data);}
   };
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
       static_assert(not std::is_reference_v<T>, "Type T must not be a reference");

       T data;

   public:
       constexpr explicit On(const T &t) noexcept(noexcept(T(t)))
           : data(t) {}
       constexpr explicit On(T &&t) noexcept(noexcept(T(std::move(t))))
           : data(std::move(t)) {}

       constexpr explicit(false) operator T() const & noexcept {
           return data;
       }
       constexpr explicit(false) operator T() && noexcept {
           return std::move(data);
       }
       constexpr explicit(false) operator T() const && noexcept {
           return std::move(data);
       }

       constexpr T &&operator>>(Stop) noexcept {
           return std::move(data);
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
       constexpr auto operator>>(F &&f) &
           noexcept(noexcept(std::invoke(std::forward<F>(f), data)))
               -> On<std::invoke_result_t<F, T>>
       {
           return On<std::invoke_result_t<F, T>>(std::invoke(std::forward<F>(f), data));
       }

       template<std::invocable<T> F>
       constexpr auto operator>>(F &&f) const &
           noexcept(noexcept(std::invoke(std::forward<F>(f), data)))
               -> On<std::invoke_result_t<F, T>>
       {
           return On<std::invoke_result_t<F, T>>(std::invoke(std::forward<F>(f), data));
       }

       template<std::invocable<T> F>
       constexpr auto operator>>(F &&f) &&
           noexcept(noexcept(std::invoke(std::forward<F>(f), std::move(data))))
               -> On<std::invoke_result_t<F, T>>
       {
           return On<std::invoke_result_t<F, T>>(std::invoke(std::forward<F>(f), std::move(data)));
       }

       template<std::invocable<T> F>
       constexpr auto operator>>(F &&f) const &&
           noexcept(noexcept(std::invoke(std::forward<F>(f), std::move(data))))
               -> On<std::invoke_result_t<F, T>>
       {
           return On<std::invoke_result_t<F, T>>(std::invoke(std::forward<F>(f), std::move(data)));
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
       constexpr auto operator()(T &&t) const noexcept
           -> On<std::decay_t<T>>
       {
           return On<std::decay_t<T>>(std::forward<T>(t));
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
        * Lift : (T -> U) -> (M\<T> -> M\<U>)
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
        * Flatlift : (T -> M\<U>) -> (M\<T> -> M\<U>)
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

   constexpr inline Some some;
   constexpr inline Every every;
   constexpr inline None none;
}

namespace fff {
   inline Each                     each;
   inline Map                      map;
   inline Filter                   filter;
   inline Reject                   reject;

   template<std::size_t SZ>
   inline IdentityAt<SZ>           identity_at;
   template<std::size_t SZ>
   inline CopyAt<SZ>               copy_at;

   inline OnceFactory              once;
   inline CountFactory             count;

   inline FlyFactory               fly;

   inline MaybeFactory             maybe;
   inline Stop                     stop;
   inline GoFactory                go;

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

       [[no_unique_address]] NullLiftFactory null_lift{};

       [[no_unique_address]] IdentityAt<0> identity{};
       [[no_unique_address]] CopyAt<0> copy{};

       [[no_unique_address]] OnceFactory once{};
       [[no_unique_address]] CountFactory count{};

       [[no_unique_address]] FlyFactory fly;

       [[no_unique_address]] MaybeFactory maybe{};
       [[no_unique_address]] Stop stop{};
       [[no_unique_address]] GoFactory go{};

       [[no_unique_address]] ParallelFactory parallel{};
       [[no_unique_address]] OverloadFactory overload{};
       [[no_unique_address]] PipelineFactory pipeline{};

       Package() = default;
   };
}

static_assert(std::is_empty_v<fff::Package>, "the Package class should be empty");

#endif //UNDERSCORE_CPP_FFFFFF_H

