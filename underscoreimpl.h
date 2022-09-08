#ifndef UNDERSCORE_CPP_UNDERSCOREIMPL_H
#define UNDERSCORE_CPP_UNDERSCOREIMPL_H

#include <functional>
#include <iostream>

class underscore {
public:
    static void hello();
    
    std::function<void()> hello3 = [this](){
        hello2();
    };
    std::function<void()> hello2 = [](){
        std::cout << "Hello2\n";
    };
    
    
};

#endif //UNDERSCORE_CPP_UNDERSCOREIMPL_H
