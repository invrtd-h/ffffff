#include "tu_1.h"

#include "ffffff/debug_tools.h"
#include "ffffff/overload.hpp"
#include "ffffff/utils.hpp"
#include "ffffff/reducible.hpp"
#include <iostream>

void test1() {
    std::cout << std::boolalpha;
    std::cout << "\nTest 1 entered...\n";

    auto f = fff::once([](){std::cout << "F!\n"; return 1;});
    f();
    f();
    std::cout << fff::once([](){std::cout << "F!\n"; return 1;})() << '\n';

    auto g = fff::overload(ForwardingTester(), [](){std::cout << "G!\n"; return 1;});
    auto gg = fff::once(g);
    gg();
    fff::once(g)();

    auto h = fff::reducible([](int a, int b) {return a + b;});
    std::cout << h(1, 2, 3, 4, 5, 6) << '\n';

    std::cout << fff::l_call_r_invoke(h, std::make_tuple(1, 2, 3), 4, 5, 16) << '\n';


}

