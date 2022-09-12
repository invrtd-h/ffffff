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
    struct MakeResultContainer {
        template<typename T, class FuncObj>
        requires std::invocable<FuncObj, T>
                 and usconts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::vector<T> &cont, const FuncObj &func) const;
        
        template<typename T, class FuncObj, size_t N>
        requires std::invocable<FuncObj, T>
                 and usconts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::array<T, N> &cont, const FuncObj &func) const;
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
    
    /*
     * Struct def part end;
     */
    
    /*
     * Implementation part start;
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
        auto ret = MakeResultContainer()(cont, func);
        for (size_t i = 0; i < cont.size(); ++i) {
            ret[i] = func(cont[i]);
        }
        
        return ret;
    }
    
    template<typename T, class FuncObj>
    requires std::invocable<FuncObj, T>
             and usconts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
    auto MakeResultContainer::operator()(const std::vector<T> &cont, const FuncObj &func) const {
        return std::vector<typename std::invoke_result<FuncObj, T>::type>(cont.size());
    }
    
    template<typename T, class FuncObj, size_t N>
    requires std::invocable<FuncObj, T>
             and usconts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
    auto MakeResultContainer::operator()(const std::array<T, N> &cont, const FuncObj &func) const {
        return std::array<typename std::invoke_result<FuncObj, T>::type, N>();
    }
}

class underscore {
public:
    u_u::Each each{};
    u_u::Map map{};
    u_u::Identity identity{};
    u_u::Noop noop{};
    
    u_u::MakeResultContainer make_result_container;
    
    struct {
        template<class NewDataGeneratePolicy, class FunctionBody>
        auto operator()(const NewDataGeneratePolicy &new_data_generate_policy,
                        const FunctionBody &function_body) const
        {
        
        }
        
    } bloop;
};

#endif //UNDERSCORE_CPP_UNDERSCOREIMPL_H
