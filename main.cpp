#include <iostream>
#include <vector>

#include "ffffff/package.hpp"
#include "ffffff/overload.hpp"
#include "ffffff/debug_tools.h"

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
    fff::Package f;

    std::cout << fff::MyClass::created() << '\n';

    fff::Test<int> test(1);
    auto test2 = test.lift([](const int &n) {return n * 2;});
    std::cout << test2.data << '\n';

    auto test4 = fff::Test<ForwardingTester>(ForwardingTester{}).lift(f.copy);

    auto k = f.null_lift([](int n) {return 2 * n;});
    std::cout << k(2) << '\n';

    auto l = f.null_lift([]() {std::cout << "Hello\n";});
    l();

}
