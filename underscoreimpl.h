#ifndef UNDERSCORE_CPP_UNDERSCOREIMPL_H
#define UNDERSCORE_CPP_UNDERSCOREIMPL_H

#include <type_traits>
#include <functional>
#include <iostream>

namespace usconts {
    template<typename T>
    concept Printable = requires (T t) {
        std::cout << t;
    };
    
    template<class T>
    concept DefaultConstructible = requires {
        T();
    };
}

class underscore {
public:
    struct {
        template<class Cont, class UnaryFuncObj>
        requires std::ranges::range<Cont> and std::invocable<UnaryFuncObj, typename Cont::value_type &>
        void operator()(Cont &cont, const UnaryFuncObj &func) const {
            for (auto &val : cont) {
                func(val);
            }
        }
    } each;
    
    struct {
        template<typename T, class FuncObj>
        requires std::invocable<FuncObj, T> and usconts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::vector<T> &vec, const FuncObj &func) const {
            std::vector<typename std::invoke_result<FuncObj, T>::type> ret(vec.size());
            for (size_t i = 0; i < vec.size(); ++i) {
                ret[i] = func(vec[i]);
            }
        
            return ret;
        }
    
        template<typename T, class FuncObj, size_t N>
        requires std::invocable<FuncObj, T> and usconts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::array<T, N> &vec, const FuncObj &func) const {
            std::array<typename std::invoke_result<FuncObj, T>::type, N> ret;
            for (size_t i = 0; i < vec.size(); ++i) {
                ret[i] = func(vec[i]);
            }
        
            return ret;
        }
    } map;
    
    /**
     * Identity-return function obj.
     * @cite https://en.cppreference.com/w/cpp/utility/functional/identity
     * @param t any
     * @return t
     */
    struct {
        template<class T>
        constexpr T &&operator()(T &&t) const noexcept {
            return std::forward<T>(t);
        }
    } identity;
    
    /**
     * No-operation function obj.
     * @return no_return
     */
    struct {
        void operator()() const noexcept {
            // do nothing
        }
    } noop;
};

#endif //UNDERSCORE_CPP_UNDERSCOREIMPL_H
