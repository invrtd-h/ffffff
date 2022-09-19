#include <iostream>
#include <vector>

#include "underscore.h"
#include "debug_tools.h"

void f_220916_01() {
    std::cout << std::boolalpha <<
            us::tmf::IsStdGMap<std::set<int>> << '\n';
}

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
    
    std::cout << '\n' << '\n';
    
    f_220916_01();
    
    us::lab::Foo()(std::vector<unsigned int>{1, 2, 3, 4, 5, 6});
    
    std::list<long long> dq{1, 10, 100, 1000, 10000};
    auto dqdq = __.bloop_map(dq, [](long long n) -> std::string {
        return std::string("s") + std::to_string(n);
    });
    
    for (auto &s : dqdq) {
        std::cout << s << ' ';
    } std::cout << '\n';
}
