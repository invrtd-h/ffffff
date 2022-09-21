#include <iostream>
#include <vector>

#include "ffffff.h"
#include "debug_tools.h"

void f_220916_01() {
    std::cout << std::boolalpha <<
            us::tmf::IsStdGMap<std::set<int>> << '\n';
}

void f_220919_filter_test() {
    std::deque<int> dq{3, 1, 4, 1, 5, 9, 2};
    auto filt = ffffff.filter(dq, [](int n) -> int {
        return n - 1;
    });
    
    for (const auto &v : filt) {
        std::cout << v << ' ';
    } std::cout << '\n';
    
    std::vector<int> vec{5, 8, 7, 7, 3, 3, 8, 5};
    auto rej = ffffff.reject(vec, [](int n) -> bool {
        return n > 5;
    });
    
    for (const auto &v : rej) {
        std::cout << v << ' ';
    } std::cout << '\n';
}

int main() {
    std::vector<int> v{1, 2, 3, 4, 5};
    ffffff.each(v, [](int &n) { n *= 2;});
    
    for (int n : v) std::cout << n << ' ';
    
    auto v2 = ffffff.map(v, [](int n) -> long long {return n * n;});
    
    for (auto &n : v2) std::cout << n << ' ';
    
    std::array arr{1, 3, 5, 7, 9, 11};
    auto arr2 = ffffff.map(arr, [](int n) {return n + 2;});
    
    for (auto &n : arr2) std::cout << n << ' ';
    
    std::cout << '\n' << '\n';
    
    std::array arr3{1, 1, 2, 3, 5, 8};
    ffffff.bloop_each(arr3, [](int n) {return n * 3;});
    ffffff.each(arr3, [](int n) {std::cout << n << ' ';});
    
    auto arr4 = ffffff.bloop_map(arr3, [](int n) {return n / 5;});
    ffffff.bloop_each(arr4, [](int n) {std::cout << n << ' '; return n;});
    ffffff.bloop_each(arr3, [](int n) {std::cout << n << ' '; return n;});
    
    std::cout << '\n' << '\n';
    
    f_220916_01();
    
    us::lab::Foo()(std::vector<unsigned int>{1, 2, 3, 4, 5, 6});
    
    std::list<long long> dq{1, 10, 100, 1000, 10000};
    auto dqdq = ffffff.bloop_map(dq, [](long long n) -> std::string {
        return std::string("s") + std::to_string(n);
    });
    
    for (auto &s : dqdq) {
        std::cout << s << ' ';
    } std::cout << '\n';
    
    f_220919_filter_test();
}
