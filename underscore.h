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

#include "usconcepts.h"

namespace under {
    
    struct Each {
        template<class Cont, class FuncObj>
        requires std::ranges::range<Cont>
                 and std::invocable<FuncObj, typename Cont::value_type &>
        void operator()(Cont &cont, const FuncObj &func) const;
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
        auto operator()(const std::array<T, N> &cont, const FuncObj &func) const noexcept;
        
        template<typename T, class FuncObj>
        requires std::invocable<FuncObj, T>
                 and us_concepts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::vector<T> &cont, const FuncObj &func) const noexcept;
        
        template<typename T, class FuncObj>
        requires std::invocable<FuncObj, T>
                 and us_concepts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::deque<T> &cont, const FuncObj &func) const noexcept;
    };
    
    /**
     * Identity-return function obj.
     * @cite https://en.cppreference.com/w/cpp/utility/functional/identity
     * @param t any-variable
     * @return t
     */
    struct Identity {
        template<class T>
        constexpr T &&operator()(T &&t) const noexcept;
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
        void operator()() const noexcept;
        
        template<class T>
        void operator()(const T &t) const noexcept;
    };
    
    template<class ANewDataPolicy, class AnExecutionPolicy>
    requires ANewDataPolicy::is_new_data_policy
            and AnExecutionPolicy::is_execution_policy
    struct Bloop {
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
    void Each::operator()(Cont &cont, const FuncObj &func) const {
        for (auto &val : cont) {
            func(val);
        }
    }
    
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
    
    template<typename T, class FuncObj, size_t N>
    requires std::invocable<FuncObj, T>
             and us_concepts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
    auto under::CopyCont::operator()(const std::array<T, N> &cont, const FuncObj &func) const noexcept {
        return std::array<typename std::invoke_result<FuncObj, T>::type, N>();
    }
    
    template<typename T, class FuncObj>
    requires std::invocable<FuncObj, T>
             and us_concepts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
    auto CopyCont::operator()(const std::vector<T> &cont, const FuncObj &func) const noexcept {
        return std::vector<typename std::invoke_result<FuncObj, T>::type>(cont.size());
    }
    
    template<typename T, class FuncObj>
    requires std::invocable<FuncObj, T>
             and us_concepts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
    auto CopyCont::operator()(const std::deque<T> &cont, const FuncObj &func) const noexcept {
        return std::deque<typename std::invoke_result<FuncObj, T>::type>(cont.size());
    }
    
    template<class T>
    constexpr T &&Identity::operator()(T &&t) const noexcept {
        return std::forward<T>(t);
    }
    
    void Noop::operator()() const noexcept {
        // do nothing
    }
    
    template<class T>
    void Noop::operator()(const T &t) const noexcept {
        // do nothing
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
