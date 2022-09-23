#include <iostream>
#include <vector>

#include "ffffff.h"
#include "debug_tools.h"

int foo(int n) noexcept {
    std::vector<int> vi(1);
    vi[0] = 3;
    
    return n;
}

int main() {
    f220921<std::vector>();
    
    auto once = fff::once([]() noexcept {
        std::cout << "Hello\n";
        return 3;
    });
    
    once(); once();
    
    auto print_str = [](const std::string &s) {std::cout << s << '\n';};
    
    auto concated = fff::make_f_concat(
            [](int n) {std::cout << n << '\n';},
            print_str,
            [](std::pair<int, int> p) {std::cout << p.first << ' ' << p.second << '\n';}
    );
    
    concated(1);
    concated("String");
    concated(std::make_pair(41771, 7110));
}
