#ifndef UNDERSCORE_CPP_UNDERSCOREIMPL_H
#define UNDERSCORE_CPP_UNDERSCOREIMPL_H

#include <type_traits>
#include <functional>
#include <iostream>

#include "usconcepts.h"

namespace u_u {
    struct Each {
        template<class Cont, class FuncObj>
        requires std::ranges::range<Cont>
                 and std::invocable<FuncObj, typename Cont::value_type &>
        void operator()(Cont &cont, const FuncObj &func) const;
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
    
    /**
     * No-operation function obj.
     * @return no_return
     */
    struct Noop {
        void operator()() const noexcept {
            // do nothing
        }
    };
    
    template<class Cont, class FuncObj>
    requires std::ranges::range<Cont>
             and std::invocable<FuncObj, typename Cont::value_type &>
    void Each::operator()(Cont &cont, const FuncObj &func) const {
        for (auto &val : cont) {
            func(val);
        }
    }
}

class underscore {
public:
    u_u::Each each{};
    u_u::Identity identity{};
    u_u::Noop noop{};
    
    struct {
        template<typename T, class FuncObj>
        requires std::invocable<FuncObj, T>
                and usconts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::vector<T> &vec, const FuncObj &func) const {
            std::vector<typename std::invoke_result<FuncObj, T>::type> ret(vec.size());
            for (size_t i = 0; i < vec.size(); ++i) {
                ret[i] = func(vec[i]);
            }
        
            return ret;
        }
    
        template<typename T, class FuncObj, size_t N>
        requires std::invocable<FuncObj, T>
                and usconts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::array<T, N> &vec, const FuncObj &func) const {
            std::array<typename std::invoke_result<FuncObj, T>::type, N> ret;
            for (size_t i = 0; i < vec.size(); ++i) {
                ret[i] = func(vec[i]);
            }
        
            return ret;
        }
    } map;
    
    struct {
        template<class NewDataGeneratePolicy, class FunctionBody>
        auto operator()(const NewDataGeneratePolicy &new_data_generate_policy,
                        const FunctionBody &function_body) const
        {
        
        }
        
    } bloop;
    
    struct {
        template<typename T, class FuncObj>
        requires std::invocable<FuncObj, T>
                 and usconts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::vector<T> &vec, const FuncObj &func) const {
            return std::vector<typename std::invoke_result<FuncObj, T>::type>(vec.size());
        }
    
        template<typename T, class FuncObj, size_t N>
        requires std::invocable<FuncObj, T>
                 and usconts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::array<T, N> &vec, const FuncObj &func) const {
            return std::array<typename std::invoke_result<FuncObj, T>::type, N>();
        }
    } make_result_container;
};

#endif //UNDERSCORE_CPP_UNDERSCOREIMPL_H
