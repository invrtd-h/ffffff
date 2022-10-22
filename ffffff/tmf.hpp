#ifndef UNDERSCORE_CPP_TMF_HPP
#define UNDERSCORE_CPP_TMF_HPP

#include <type_traits>
#include <concepts>
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

    namespace liated {
        template<unsigned int N, typename T = void, typename ...U>
        struct among_impl {
            using type = typename among_impl<N - 1, U...>::type;
        };

        template<typename T, typename ...U>
        struct among_impl<0, T, U...> {
            using type = T;
        };
    }

   template<typename ...T>
    struct among {
        template<unsigned int N>
        using get = typename liated::among_impl<N, T...>::type;
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

    /**
    * A TMP struct that just holds a constexpr variable.
    * @tparam v any, constexpr literals
    * @member value_holder::type = decayed type of v
    * @member value_holder::value = v
    */
    template<auto v>
    struct value_holder {
        using type = std::decay_t<decltype(v)>;
        constexpr static type value = v;
    };

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

    /**
    * A concept determines whether the given C is a unary predicate.
    * @tparam C Any unary type constructor
    * @example unary_pred\<std::is_class> is true, since std::is_class\<T>::value is evaluated as bool
    * @example unary_pred\<std::vector> is false
    */

    template<template<class> class C>
    concept unary_pred =
        requires {
            { C<int>::value } -> std::convertible_to<bool>;
            constexpr_determination::is_consteval_var(C<int>::value);
        };



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


    template<typename T, template<class, class...> class C>
    concept made_by = std::is_same_v<std::decay_t<T>,
    change_template_t <C, std::decay_t<T>>>;

    template<typename T, template<class, class...> class C>
    concept not_made_by = not made_by<T, C>;

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

    template<typename T, template<class...> class C>
    concept satisfies = C<T>::value;

    template<typename T>
    concept dereferencible = requires (T t) {*t;};

    template<typename T>
    concept negatable = requires (T t) {!t;};

    template<typename T>
    concept flippable = requires (T t) {~t;};

    template<template<class> class C>
    concept backpushable =
        requires (C<int> cont) {
            cont.push_back(0);
        };

    template<template<class> class C>
    concept insertible =
        requires (C<int> cont) {
            cont.insert(0);
        };

    template<typename T>
    concept maybetype =
        requires {
            T::is_maybe;
        };


    template<typename T>
    class TD;
}

#endif //UNDERSCORE_CPP_TMF_HPP
