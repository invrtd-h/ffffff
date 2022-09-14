#include <iostream>
#include <vector>

#include "underscore.h"

int main() {
    std::vector<int> v{1, 2, 3, 4, 5};
    __.each(v, [](int &n) {n *= 2;});
    
    for (int n : v) std::cout << n << ' ';
    
    auto v2 = __.map(v, [](int n) -> long long {return n * n;});
    
    for (auto &n : v2) std::cout << n << ' ';
    
    std::array arr{1, 3, 5, 7, 9, 11};
    auto arr2 = __.map(arr, [](int n) {return n + 2;});
    
    for (auto &n : arr2) std::cout << n << ' ';
    
    std::cout << '\n' << '\n';
    
    std::array arr3{1, 1, 2, 3, 5, 8};
    __.bloop_each(arr3, [](int n) {return n * 3;});
    __.each(arr3, [](int n) {std::cout << n << ' ';});
    
    auto arr4 = __.bloop_map(arr3, [](int n) {return n / 5;});
    __.bloop_each(arr4, [](int n) {std::cout << n << ' '; return n;});
    __.bloop_each(arr3, [](int n) {std::cout << n << ' '; return n;});
}
