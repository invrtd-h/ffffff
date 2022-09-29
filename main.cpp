#include <iostream>
#include <vector>

#include "ffffff.h"
#include "debug_tools.h"

struct foo {
    int a;
    char b;
    short c;
    long long d;
};

int main() {
    f220921<std::vector>();
    once_test();
    concat_test();
    
    auto count = fff::count_factory([](int n) {return n * n;});
    
    std::cout << count(10) << ' ' << count(20) << ' ' << count(30) << ' ' << count.get_count() << '\n';
    
    auto got =
            fff::pthrow(1) >> multiply<3> >> multiply<3>
                    >> multiply<3> >> multiply<3> >> fff::pcatch;
    
    std::cout << typeid(got).name() << ' ' << got << '\n';
}
