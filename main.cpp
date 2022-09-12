#include <iostream>

#include "underscore.h"

int main() {

    std::vector<int> v = {1, 2, 3, 4, 5};
    __.each(v, [](int &n) {n *= 2;});
    
    for (int n : v) std::cout << n << ' ';
    
    auto v2 = __.map(v, [](int n) -> long long {return n * n;});
    
    for (auto &n : v2) std::cout << n << ' ';
    
    std::array arr{1, 3, 5, 7, 9, 11};
    auto arr2 = __.map(arr, [](int n) {return n + 2;});
    
    for (auto &n : arr2) std::cout << n << ' ';
}
