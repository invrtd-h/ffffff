#include <iostream>
#include <vector>

#include "ffffff.h"
#include "debug_tools.h"

template<int N>
auto add = [](int n) -> int {return n + N;};

template<int N>
auto multiply = [](int n) -> int {return n * N;};

template<int N>
auto add_r = [](int &n) -> void {n += N;};

template<int N>
auto multiply_r = [](int &n) -> void {n *= N;};

int main() {
    f220921<std::vector>();
    once_test();
    concat_test();
    
    auto count = fff::count_factory([](int n) {return n * n;});
    
    std::cout << count(10) << ' ' << count(20) << ' ' << count(30) << ' ' << count.get_count() << '\n';
    
    auto may = fff::maybe_factory(1);
    auto may_copy = may >> multiply<3> >> add<6>;
    
    std::cout << may.value() << ' ' << may_copy.value() << '\n';
    
    may << multiply_r<41771> << add_r<7110>;
    
    std::cout << may.value() << '\n';
    
    auto may_not = fff::maybe_factory.make<int>();
    may_not << multiply_r<41771>;
    
    std::cout << may_not.has_value() << '\n';
}
