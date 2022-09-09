#include <iostream>

#include "underscore.h"

int main() {
    __.hello2();
    __.hello3();
    
    std::vector<int> v = {1, 2, 3, 4, 5};
    __.apply(v, [](int n) {return n * 2;});
    
    for (int n : v) std::cout << n << ' ';
    
    auto v2 = __.map(v, [](int n) -> long long {return n * n;});
    
    for (auto &n : v2) std::cout << n << ' ';
}
