#ifndef UNDERSCORE_CPP_UNDERSCORE_H
#define UNDERSCORE_CPP_UNDERSCORE_H

#include <type_traits>
#include <functional>
#include <iostream>
#include <concepts>
#include <utility>
#include <ranges>

#include <forward_list>
#include <vector>
#include <array>
#include <deque>
#include <list>

#include "usconcepts.h"

namespace under {
    
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
        auto operator()(const Cont &cont, const FuncObj &func) const;
    };
    
    struct NewDataPolicy {
        static const bool is_new_data_policy = true;
    };
    
    struct ExecutionPolicy {
        static const bool is_execution_policy = true;
    };
    
    /**
     * Making Result-Container function obj.
     * @param cont any std::(container) with type T
     * @param func any function obj with 1 param, say, T -> U
     * @return any std::(container) with type U
     */
    struct CopyCont : public NewDataPolicy {
        template<typename T, class FuncObj, size_t N>
        requires std::invocable<FuncObj, T>
                 and us_concepts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::array<T, N> &cont, const FuncObj &func) const noexcept {
            return std::array<typename std::invoke_result<FuncObj, T>::type, N>();
        }
        
        template<typename T, class FuncObj>
        requires std::invocable<FuncObj, T>
                 and us_concepts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::vector<T> &cont, const FuncObj &func) const noexcept {
            return std::vector<typename std::invoke_result<FuncObj, T>::type>(cont.size());
        }
        
        template<typename T, class FuncObj>
        requires std::invocable<FuncObj, T>
                 and us_concepts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::deque<T> &cont, const FuncObj &func) const noexcept {
            return std::deque<typename std::invoke_result<FuncObj, T>::type>(cont.size());
        }
    
        template<typename T, class FuncObj>
        requires std::invocable<FuncObj, T>
                 and us_concepts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::list<T> &cont, const FuncObj &func) const noexcept {
            return std::list<typename std::invoke_result<FuncObj, T>::type>(cont.size());
        }
    
        template<typename T, class FuncObj>
        requires std::invocable<FuncObj, T>
                 and us_concepts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::forward_list<T> &cont, const FuncObj &func) const noexcept {
            return std::forward_list<typename std::invoke_result<FuncObj, T>::type>(cont.size());
        }
        
        /**
         * @todo generate a policy that copying user-defined container
         */
    };
    
    struct NewCont : public NewDataPolicy {
    
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
    struct Identity_at {
        static const bool is_new_data_policy = true;
    
        template<class T, typename ...Args>
        constexpr auto &&operator()(T &&t, Args &&...args) const noexcept {
            return Identity_at<SZ - 1>()(std::forward<Args>(args)...);
        }
    };
    
    template<>
    struct Identity_at<0> {
        static const bool is_new_data_policy = true;
        
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
        void operator()() const noexcept {
            // do nothing
        }
        
        template<class ...Args>
        void operator()(Args &&...args) const noexcept {
            // do nothing
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
        auto operator()(Cont &cont, const FuncObj &func) const {
    
            decltype(auto) ret = ANewDataPolicy()(cont, func);
    
            AnExecutionPolicy()(ret, cont, func);
    
            return ret;
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
                and std::is_same<typename std::invoke_result<FuncObj, typename T_cont::value_type>::type,
                                 typename U_cont::value_type>::value
        auto &operator()(U_cont &u_cont, T_cont &t_cont, const FuncObj &func) const {
            auto it_t = t_cont.begin();
            auto it_u = u_cont.begin();
            
            while (it_t != t_cont.end()) {
                *it_u = func(*it_t);
                ++it_t; ++it_u;
            }
            
            return u_cont;
        }
    };
    
    using BloopEach = Bloop<Identity_at<0>, MapExecution>;
    using BloopMap = Bloop<CopyCont, MapExecution>;
    
    /*
     * Struct def part end;
     */
    
    /*
     * Implementation def part start;
     */
    
    template<class Cont, class FuncObj>
    requires std::ranges::range<Cont>
             and std::invocable<FuncObj, typename Cont::value_type &>
    auto Map::operator()(const Cont &cont, const FuncObj &func) const {
        auto ret = CopyCont()(cont, func);
        
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

class underscore {
public:
    under::Each             each;
    under::Map              map;
    
    under::BloopEach        bloop_each;
    under::BloopMap         bloop_map;
};

static underscore __;

#endif //UNDERSCORE_CPP_UNDERSCORE_H
