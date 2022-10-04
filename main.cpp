#include <iostream>
#include <vector>

#include "ffffff.h"
#include "debug_tools.h"

#include "classify.h"

struct foo {
    int a;
    char b;
    short c;
    long long d;
};

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
    fff::Package f_{};
    f220921<std::vector>();
    once_test();
    concat_test();
    
    auto count = fff::count([](int n) {return n * n;});
    
    std::cout << count(10) << ' ' << count(20) << ' ' << count(30) << ' ' << count.get_count() << '\n';
    
    int got =
            fff::pipethrow(1) >> multiply<3> >> multiply<3>
                    >> multiply<3> >> multiply<3> >> fff::pipecatch;
    
    std::cout << sizeof(f_) << '\n';
}
