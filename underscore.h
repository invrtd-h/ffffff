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
        void operator()(Cont &cont, const FuncObj &func) const
        requires std::ranges::range<Cont>
                 and std::invocable<FuncObj, typename Cont::value_type &> {
            for (auto &val : cont) {
                func(val);
            }
        };
        
        template<class T>
        int foo(T bar) {
            return static_cast<int>(bar);
        }
    };
    
    struct Map {
        template<class Cont, class FuncObj>
        requires std::ranges::range<Cont>
                 and std::invocable<FuncObj, typename Cont::value_type &>
        auto operator()(const Cont &cont, const FuncObj &func) const;
    };
    
    /**
     * Making Result-Container function obj.
     * @param cont any std::(container) with type T
     * @param func any function obj with 1 param, say, T -> U
     * @return any std::(container) with type U
     */
    struct MakeResultCont {
        template<typename T, class FuncObj, size_t N>
        requires std::invocable<FuncObj, T>
                 and us_concepts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::array<T, N> &cont, const FuncObj &func) const;
        
        template<typename T, class FuncObj>
        requires std::invocable<FuncObj, T>
                 and us_concepts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::vector<T> &cont, const FuncObj &func) const;
        
        template<typename T, class FuncObj>
        requires std::invocable<FuncObj, T>
                 and us_concepts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::deque<T> &cont, const FuncObj &func) const;
        
        void succ() const {
        
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
        constexpr T &&operator()(T &&t) const noexcept;
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
        auto ret = MakeResultCont()(cont, func);
        
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
    auto under::MakeResultCont::operator()(const std::array<T, N> &cont, const FuncObj &func) const {
        return std::array<typename std::invoke_result<FuncObj, T>::type, N>();
    }
    
    template<typename T, class FuncObj>
    requires std::invocable<FuncObj, T>
             and us_concepts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
    auto under::MakeResultCont::operator()(const std::vector<T> &cont, const FuncObj &func) const {
        return std::vector<typename std::invoke_result<FuncObj, T>::type>(cont.size());
    }
    
    template<typename T, class FuncObj>
    requires std::invocable<FuncObj, T>
             and us_concepts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
    auto MakeResultCont::operator()(const std::deque<T> &cont, const FuncObj &func) const {
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
    under::Identity         identity;
    under::Noop             noop;
    under::MakeResultCont   make_result_cont;
};

static underscore __;

#endif //UNDERSCORE_CPP_UNDERSCORE_H
