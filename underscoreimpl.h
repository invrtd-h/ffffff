#ifndef UNDERSCORE_CPP_UNDERSCOREIMPL_H
#define UNDERSCORE_CPP_UNDERSCOREIMPL_H

#include <functional>
#include <iostream>

namespace usconts {
    template<typename T>
    concept Printable = requires (T t) {
        std::cout << t;
    };
    
    template<typename F, typename RET, typename T>
    concept IsFunction = requires (F f, T t) {
        f(t) -> RET;
    };
    
    template<class T>
    concept DefaultConstructible = requires {
        T();
    };
}

class underscore {
public:
    static void hello();
    
    std::function<void()> hello3 = [this](){
        hello2();
    };
    std::function<void()> hello2 = [](){
        std::cout << "Hello2\n";
    };
    
    template<class Cont, class UnaryCallable>
    requires std::ranges::range<Cont> and std::invocable<UnaryCallable, typename Cont::value_type>
    underscore &apply(Cont &cont, const UnaryCallable &callable) {
        for (auto &val : cont) {
            val = callable(val);
        }
        
        return *this;
    }
    
    struct Map {
        template<typename T, class FuncObj>
        requires std::invocable<FuncObj, T> and usconts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::vector<T> &vec, const FuncObj &func) {
            std::vector<typename std::invoke_result<FuncObj, T>::type> ret(vec.size());
            for (size_t i = 0; i < vec.size(); ++i) {
                ret[i] = func(vec[i]);
            }
        
            return ret;
        }
    
        template<typename T, class FuncObj, size_t N>
        requires std::invocable<FuncObj, T> and usconts::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::array<T, N> &vec, const FuncObj &func) {
            std::array<typename std::invoke_result<FuncObj, T>::type, N> ret;
            for (size_t i = 0; i < vec.size(); ++i) {
                ret[i] = func(vec[i]);
            }
        
            return ret;
        }
    } map;
};

#endif //UNDERSCORE_CPP_UNDERSCOREIMPL_H
