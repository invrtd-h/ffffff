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
    using namespace fff;
    
    Package f;
    
    auto g = f.pipeline(
        add<2>,
        add<4>,
        multiply<3>,
        add<4>
    );
    
    std::cout << g(5) << '\n';
    
    auto h = g >> multiply<3> >> add<7>;
    std::cout << h(5) << '\n';
}
