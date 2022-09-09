#ifndef UNDERSCORE_CPP_UNDERSCOREIMPL_H
#define UNDERSCORE_CPP_UNDERSCOREIMPL_H

#include <functional>
#include <iostream>

namespace usconts {
    template<typename T>
    concept Printable = requires (T t) {
        std::cout << t;
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
};

#endif //UNDERSCORE_CPP_UNDERSCOREIMPL_H
