#include "tu_1.h"

#include <iostream>
#include "ffffff/utils.hpp"

void test1() {
    std::cout << std::boolalpha;
    std::cout << "\nTest 1 entered...\n";

    auto f = fff::once([](){std::cout << "F!\n"; return 1;});
    f();
    f();
    std::cout << fff::once([](){std::cout << "F!\n"; return 1;})() << '\n';

    std::cout << fff::rvalue_detector(1);
}