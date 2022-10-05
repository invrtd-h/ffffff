#include <iostream>
#include <vector>

#include "ffffff.h"
#include "debug_tools.h"

#include "classify.h"

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

int main() {
    fff::Package f_;
    f220921<std::vector>();
    once_test();
    concat_test();
    
    auto g = f_.compose(
        add<3>, add<5>, multiply<3>
    );
    std::cout << g(5) << '\n';
}
