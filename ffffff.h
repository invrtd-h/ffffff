#ifndef UNDERSCORE_CPP_FFFFFF_H
#define UNDERSCORE_CPP_FFFFFF_H

#include <type_traits>
#include <functional>
#include <algorithm>
#include <iostream>
#include <concepts>
#include <utility>
#include <ranges>

#include <vector>
#include <array>
#include <deque>
#include <forward_list>
#include <list>

#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>

#include "us_tmf.h"

namespace us::lab {

}

namespace us::policydef {
    struct NewDataPolicy {
        static constexpr bool is_new_data_policy = true;
    };
    
    struct ExecutionPolicy {
        static constexpr bool is_execution_policy = true;
    };
}

/**
 * Small operations
 */
namespace us::smallops {
    using namespace policydef;
    
    /**
     * Identity-return function obj.
     * @cite https://en.cppreference.com/w/cpp/utility/functional/identity
     * @param t any-variable
     * @return t
     */
    struct Identity {
        template<class T>
        constexpr T &&operator()(T &&t) const noexcept {
            return std::forward<T>(t);
        }
    };
    
    /**
     * The function object that Returns the SZ-th value by perfect forwarding.
     * @example IdentityAt<2>()(1, 2, std::string("a")) \n
     * Returns the rvalue of std::string("a")
     * @tparam SZ the position to return
     * @cite https://en.cppreference.com/w/cpp/utility/functional/identity
     */
    template<size_t SZ>
    struct IdentityAt : public NewDataPolicy {
        /**
         * @tparam T Any-type
         * @tparam Args Any-parameter-pack
         * @param t Any-variable
         * @param args Any-variable-pack
         * @return std::forward<T>(t)
         */
        template<class T, typename ...Args>
        constexpr auto &&operator()(T &&t, Args &&...args) const noexcept {
            return IdentityAt<SZ - 1>()(std::forward<Args>(args)...);
        }
    };
    template<>
    struct IdentityAt<0> : public NewDataPolicy {
        /**
         * Template specification of IdentityAt<> (read upward)
         */
        template<class T, typename ...Args>
        constexpr T &&operator()(T &&t, Args &&...args) const noexcept {
            return std::forward<T>(t);
        }
    };
    
    /**
     * No-operation function obj.
     * @return no_return
     */
    struct Noop {
        template<class ...Args>
        constexpr void operator()(Args &&...args) const noexcept {
            // do nothing
        }
    };
    
    /**
     * A callable that always Returns a constant.
     * @tparam T the type of the constant to return
     */
    template<typename T>
    struct AlwaysConstant {
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
    
    using AlwaysPositive = AlwaysConstant<bool>::Returns<true>;
    using AlwaysNegative = AlwaysConstant<bool>::Returns<false>;
    
    inline auto always_positive = AlwaysPositive();
    inline auto always_negative = AlwaysNegative();
}

namespace us {
    using namespace smallops;
    
    /**
     * Making Result-Container function obj.
     * @param cont any std::(container) with type T
     * @param func any function obj with 1 param, say, T -> U
     * @return any std::(container) with type U
     */
    
    struct PreallocCont : public NewDataPolicy {
        template<typename T, class FuncObj, size_t N>
        requires std::invocable<FuncObj, T>
                 and tmf::DefaultConstructible<std::invoke_result_t<FuncObj, T>>
        constexpr auto operator()(const std::array<T, N> &cont, const FuncObj &func) const noexcept {
            return std::array<std::invoke_result_t<FuncObj, T>, N>();
        }
        
        template<template<class> class C, typename T, class FuncObj>
        requires std::ranges::range<C<T>>
                 and std::invocable<FuncObj, T>
                 and tmf::DefaultConstructible<std::invoke_result_t<FuncObj, T>>
        constexpr auto operator()(const C<T> &cont, const FuncObj &func) const noexcept {
            return C<std::invoke_result_t<FuncObj, T>>(cont.size());
        }
    };
    
    struct NewCont : public NewDataPolicy {
        template<class Cont, class FuncObj>
        requires std::ranges::range<Cont>
                and tmf::DefaultConstructible<Cont>
        constexpr auto operator()(const Cont &cont, const FuncObj &funcObj) const noexcept {
            return Cont();
        }
    };
    
    struct MapExecution : public ExecutionPolicy {
        /**
         * @todo consider if the return value of func is void
         * @tparam T_cont
         * @tparam U_cont
         * @tparam FuncObj
         * @param u_cont
         * @param t_cont
         * @param func
         * @return
         */
        template<class T_cont, class U_cont, class FuncObj>
        requires std::ranges::range<T_cont>
                and std::ranges::range<U_cont>
                and std::is_same_v<typename std::invoke_result<FuncObj, typename T_cont::value_type>::type,
                                 typename U_cont::value_type>
        constexpr auto &operator()(U_cont &u_cont, T_cont &t_cont, const FuncObj &func) const {
            auto it_t = t_cont.begin();
            auto it_u = u_cont.begin();
            
            while (it_t != t_cont.end()) {
                *it_u = func(*it_t);
                ++it_t; ++it_u;
            }
            
            return u_cont;
        }
    };
    
    struct PushExecution : public ExecutionPolicy {
        template<class T_cont, class FuncObj>
        requires std::ranges::range<T_cont>
                 and std::convertible_to<std::invoke_result_t<FuncObj, typename T_cont::value_type>, bool>
        constexpr auto &operator()(T_cont &res_cont, T_cont &var_cont, const FuncObj &func) const {
            for (const auto &t : var_cont) {
                if (func(t)) {
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
            requires tmf::BackPushable<C>
            constexpr void operator()(C<T> &res_cont, const T &val) const noexcept {
                res_cont.push_back(val);
            }
            
            /**
             * If the container has no push_back() method and has insert() method,
             * apply it
             */
            template<template<class> class C, typename T>
            requires (not tmf::BackPushable<C> and tmf::Insertable<C>)
            constexpr void operator()(C<T> &res_cont, const T &val) const noexcept {
                res_cont.insert(val);
            }
        };
    };
    
    struct Each {
        template<class Cont, class FuncObj>
        requires std::ranges::range<Cont>
                 and std::invocable<FuncObj, typename Cont::value_type &>
        constexpr void operator()(Cont &cont, const FuncObj &func) const {
            std::ranges::for_each(cont, func);
        }
    };
    
    struct Map {
        template<class Cont, class FuncObj>
        requires std::ranges::range<Cont>
                 and std::invocable<FuncObj, typename Cont::value_type &>
        inline auto operator()(const Cont &cont, const FuncObj &func) const {
            auto ret = PreallocCont()(cont, func);
    
            {
                auto it_t = cont.begin();
                auto it_u = ret.begin();
        
                while (it_t != cont.end()) {
                    *it_u = func(*it_t);
                    ++it_t; ++it_u;
                }
            }
    
            return ret;
        }
    };
    
    template<class ANewDataPolicy, class AnExecutionPolicy>
    requires ANewDataPolicy::is_new_data_policy
             and AnExecutionPolicy::is_execution_policy
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
namespace us::lab {
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

#include "deprecated.h"

class FFFFFF {
public:
    us::Each             each;
    us::Map              map;
    
    us::BloopEach        bloop_each;
    us::BloopMap         bloop_map;
    us::BloopFilter      bloop_filter;
};

inline static FFFFFF ffffff;

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