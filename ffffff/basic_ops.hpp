#ifndef UNDERSCORE_CPP_BASIC_OPS_HPP
#define UNDERSCORE_CPP_BASIC_OPS_HPP

#include <type_traits>
#include <functional>
#include <utility>

#include "tmf.hpp"

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
   template<std::size_t SZ>
   struct IdentityAt {
       /**
        * @tparam T Any-type
        * @tparam Args Any-parameter-pack
        * @param t Any-variable
        * @param args Any-variable-pack
        * @return std::forward<T>(t)
        */
       template<class T, typename... Args>
       constexpr auto operator()(T &&, Args &&...args) const noexcept
           -> std::invoke_result_t<IdentityAt<SZ - 1>, Args...>
       {
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
       constexpr auto operator()(T &&, Args &&...args) const noexcept
       -> std::invoke_result_t<CopyAt<SZ - 1>, Args...>
       {
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
    * A null type that works as a "null_t Object".
    * Use CRTP pattern to inherit this type and make any "null_t Object" you want.
    */
   template<typename T = void>
   struct null_i {
       const static bool nullity = true;
   };

   using null_t = null_i<>;

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
       constexpr auto operator()(Args &&...) const noexcept -> null_t {
           return {};
       }
   };
}

/*
* fff::AlwaysConstant impl
*/
namespace fff {

   template<auto v>
   struct always_constant_f {
       using type = std::decay_t<decltype(v)>;
       constexpr static type value = v;

       template<class ...Args>
       constexpr type operator()(Args &&...) const noexcept {
           return v;
       }
   };

   using always_true_f = always_constant_f<true>;
   using always_false_f = always_constant_f<false>;

   constexpr inline always_true_f always_true;
   constexpr inline always_false_f always_false;

   static_assert(std::is_same_v<always_true_f::type, bool>);
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

#endif //UNDERSCORE_CPP_BASIC_OPS_HPP
