#include <iostream>

#include "ffffff/debug_tools.h"
#include "ffffff/utils.hpp"

#include "ffffff/bind.hpp"

#include "tu_1.h"

class Bar {
    friend class fff::AsSingle<Bar>;
    
    Bar() = default;
public:
    Bar(const Bar&) = delete;
    Bar(Bar&&) = delete;
    Bar &operator=(const Bar&) = delete;
    Bar &operator=(Bar&&) = delete;
    
    void say_hello() const {
        std::cout << "Say, Hello!\n";
    }
};

class Foo {
    int a, b, c, d;
};

int main() {
    pipeop_test();

    auto plus = [](int a, int b) {return a + b;};
    auto ttt = std::bind(plus, 1, std::placeholders::_1);

    static_assert(sizeof(ttt) == 8);

    auto uuu = fff::static_l_bind<1>(plus);
    static_assert(sizeof(uuu) == 1);

    int zz = 1;
    auto vvv = std::bind(plus, zz, std::placeholders::_1);

    static_assert(sizeof(vvv) == 8);

    test1();

}
