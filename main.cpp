#include <iostream>
#include <vector>

#include "ffffff.h"
#include "debug_tools.h"

int foo(int n) noexcept {
    std::vector<int> vi(1);
    vi[0] = 3;
    
    return n;
}

struct Bar {

};

struct Foo {
    Bar bar1, bar2, bar3;
    bool operator()() const noexcept {
        return true;
    }
};

int main() {
    f220921<std::vector>();
    once_test();
    concat_test();
    
    auto count = fff::count_factory([](int n) {return n * n;});
    
    std::cout << count(10) << ' ' << count(20) << ' ' << count(30) << ' ' << count.get_count() << '\n';
    
    auto may = fff::maybe_factory(1);
    std::cout << (may >> [](int n) {return 3 * n;} >> [](int n) {return 3 * n;}).get().value() << ' '
        << may.get().value() << ' '
        << (may << [](int &n) {return n *= 69;}).get().value() << ' '
        << may.get().value() << '\n';
}
