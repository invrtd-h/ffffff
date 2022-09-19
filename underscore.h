#ifndef UNDERSCORE_CPP_UNDERSCORE_H
#define UNDERSCORE_CPP_UNDERSCORE_H

#include <type_traits>
#include <functional>
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

/**
 * Template Meta Functions && Concepts.
 */
namespace us::tmf {
    
    template<typename T>
    concept Printable = requires (T t) {std::cout << t;};
    
    template<class T>
    concept DefaultConstructible = requires {T();};
    
    template<template<class> class C, typename Cont>
    concept IsInstantized = std::is_same_v<C<typename Cont::value_type>, Cont>;
    
    template<class Cont>
    concept IsStdArray = std::is_same_v<Cont, std::array<typename Cont::value_type, Cont().size()>>;
    
    template<class Cont>
    concept IsStdMap = std::is_same_v<Cont, std::map<typename Cont::key_type, typename Cont::mapped_type>>;
    
    template<class Cont>
    concept IsStdMultimap = std::is_same_v<Cont, std::multimap<typename Cont::key_type, typename Cont::mapped_type>>;
    
    template<class Cont>
    concept IsStdUnorderedMap = std::is_same_v<Cont, std::unordered_map<typename Cont::key_type, typename Cont::mapped_type>>;
    
    template<class Cont>
    concept IsStdUnorderedMultimap = std::is_same_v<Cont, std::unordered_multimap<typename Cont::key_type, typename Cont::mapped_type>>;
    
    /**
     * A concept that determines whether the given type is sequential container in the STL library.
     * @tparam Cont any container (or any type!)
     * @return whether the given container is vector, deque or array
     */
    template<class Cont>
    concept IsStdSeqCont = IsInstantized<std::vector, Cont> or IsStdArray<Cont> or IsInstantized<std::deque, Cont>;
    
    template<class Cont>
    concept IsStdLinkedList = IsInstantized<std::list, Cont> or IsInstantized<std::forward_list, Cont>;
    
    template<class Cont>
    concept IsStdGSet =    IsInstantized<std::set, Cont>
                        or IsInstantized<std::multiset, Cont>
                        or IsInstantized<std::unordered_set, Cont>
                        or IsInstantized<std::unordered_multiset, Cont>;
    
    template<class Cont>
    concept IsStdGMap = IsStdMap<Cont> or IsStdMultimap<Cont> or IsStdUnorderedMap<Cont> or IsStdUnorderedMultimap<Cont>;
    
    template<class Cont>
    concept IsStdContSizable = IsInstantized<std::vector, Cont>
                            or IsInstantized<std::deque, Cont>
                            or IsInstantized<std::list, Cont>
                            or IsInstantized<std::forward_list, Cont>;
    
    template<template<class> class C>
    concept BackPushable = requires (C<int> cont) {
        cont.push_back(0);
    };
}

namespace us::lab {

}

namespace us {
    
    struct Each {
        template<class Cont, class FuncObj>
        requires std::ranges::range<Cont>
                 and std::invocable<FuncObj, typename Cont::value_type &>
        void operator()(Cont &cont, const FuncObj &func) const {
            for (auto &val : cont) {
                func(val);
            }
        }
    };
    
    struct Map {
        template<class Cont, class FuncObj>
        requires std::ranges::range<Cont>
                 and std::invocable<FuncObj, typename Cont::value_type &>
        inline auto operator()(const Cont &cont, const FuncObj &func) const;
    };
    
    struct NewDataPolicy {
        static constexpr bool is_new_data_policy = true;
    };
    
    struct ExecutionPolicy {
        static constexpr bool is_execution_policy = true;
    };
    
    /**
     * Making Result-Container function obj.
     * @param cont any std::(container) with type T
     * @param func any function obj with 1 param, say, T -> U
     * @return any std::(container) with type U
     *
     * @todo generate a policy that copying user-defined container
     */
    
    struct PreAllocCont : public NewDataPolicy {
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
    
    template<size_t SZ>
    struct IdentityAt : public NewDataPolicy {
        template<class T, typename ...Args>
        constexpr auto &&operator()(T &&t, Args &&...args) const noexcept {
            return IdentityAt<SZ - 1>()(std::forward<Args>(args)...);
        }
    };
    
    template<>
    struct IdentityAt<0> : public NewDataPolicy {
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
        constexpr void operator()() const noexcept {
            // do nothing
        }
        
        template<class ...Args>
        constexpr void operator()(Args &&...args) const noexcept {
            // do nothing
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
            template<template<class> class C, typename T>
            requires tmf::BackPushable<C>
            constexpr void operator()(C<T> &res_cont, const T &val) const noexcept {
                res_cont.push_back(val);
            }
        };
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
    using BloopMap = Bloop<PreAllocCont, MapExecution>;
    using Filter = Bloop<NewCont, PushExecution>;
    
    /*
     * Struct def part end;
     */
    
    /*
     * Implementation def part start;
     */
    
    template<class Cont, class FuncObj>
    requires std::ranges::range<Cont>
             and std::invocable<FuncObj, typename Cont::value_type &>
    inline auto Map::operator()(const Cont &cont, const FuncObj &func) const {
        auto ret = PreAllocCont()(cont, func);
        
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
}

/**
 * The laboratory : New features are tested in this space.
 */
namespace us::lab {
    /**
     * get class Cont == C<T>, typename T
     * return C<U>
     */
    
    template<typename T, class Cont, typename U>
    using replace =
            std::conditional_t<
                    std::is_same_v<Cont, std::vector<T>>, std::vector<U>,
                    std::conditional_t<
                            std::is_same_v<Cont, std::array<T, Cont().size()>>, std::array<U, Cont().size()>,
                            std::conditional_t<
                                    std::is_same_v<Cont, std::deque<T>>, std::deque<U>,
                                    std::conditional_t<
                                            std::is_same_v<Cont, std::list<T>>, std::list<U>,
                                            std::conditional_t<
                                                    std::is_same_v<Cont, std::forward_list<T>>, std::list<U>,
                                                    std::conditional_t<
                                                            std::is_same_v<Cont, std::set<T>>, std::set<U>,
                                                            std::conditional_t<
                                                                    std::is_same_v<Cont, std::multiset<T>>, std::multiset<U>,
                                                                    
                                                                    void>>>>>>>;
    
    
    struct Foo {
        template<template<class> class C, typename T>
        constexpr auto operator()(const C<T> &cont) {
            std::cout << typeid(C<T>).name() << ' ' << typeid(T).name() << '\n';
        }
    };
}

#include "deprecated.h"

class underscore {
public:
    us::Each             each;
    us::Map              map;
    
    us::BloopEach        bloop_each;
    us::BloopMap         bloop_map;
    
    us::Filter           filter;
};

inline static underscore __;

#endif //UNDERSCORE_CPP_UNDERSCORE_H

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