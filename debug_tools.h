#ifndef UNDERSCORE_CPP_DEBUG_TOOLS_H
#define UNDERSCORE_CPP_DEBUG_TOOLS_H

class ForwardingTester {
public:
    ForwardingTester() {
        std::cout << "기본 생성자 호출!\n";
    }
    ForwardingTester(const ForwardingTester &ft) {
        std::cout << "복사 생성자 호출!\n";
    }
    ForwardingTester(ForwardingTester &&ft) noexcept {
        std::cout << "이동 생성자 호출!\n";
    }
    ForwardingTester &operator=(const ForwardingTester &ft) {
        std::cout << "복사 대입 생성자 호출!\n";
        return *this;
    }
    ForwardingTester &operator=(ForwardingTester &&ft) noexcept {
        std::cout << "이동 대입 생성자 호출!\n";
        return *this;
    }
    ~ForwardingTester() {
        std::cout << "소멸자 호출!\n";
    }
    
    ForwardingTester &catch_instance(const ForwardingTester &ft) {
        std::cout << "좌측값 레퍼런스 캐치\n";
        return *this;
    }
    ForwardingTester &catch_instance(ForwardingTester &&ft) {
        std::cout << "우측값 레퍼런스 캐치\n";
        return *this;
    }
};

template<typename T>
class TD;

template<class Cont>
void print(Cont &&cont) {
    for (auto &&c : cont) {
        std::cout << c << ' ';
    } std::cout << '\n';
}

template<class Cont>
void print(const Cont &&cont) {
    for (auto &&c : cont) {
        std::cout << c << ' ';
    } std::cout << '\n';
}

template<template<class> class C>
void f220921() {
    std::cout << std::boolalpha;
    
    C<int> cont{1, 1, 2, 3, 5, 8, 13, 21, 34, 55};
    const C<int> ccont = cont;
    
    print(cont);
    print(ccont);
    
    print(fff::filter(ccont, [](const auto &s) {
        return s % 2;
    }));
    
    print(fff::reject(ccont, [](const auto &s) {
        return s % 2;
    }));
    
    std::cout << fff::every(ccont, [](auto &s) {
        return s % 2;
    }) << '\n';
}

void once_test() {
    auto once = fff::once([]() noexcept {
        std::cout << "Hello\n";
    });
    
    once(); once();
    std::cout << "The sizeof once is " << sizeof(once) << '\n';
}

void concat_test() {
    auto print_str = [](const std::string &s) {std::cout << s << '\n';};
    
    auto overloaded = fff::overload(
            [](int n) { std::cout << n << '\n'; },
            [](double n) { std::cout << n << '\n'; },
            print_str
    );
    
    overloaded(1);
    overloaded(4.9);
    overloaded("String");
    
    int r = 3;
    
    auto f = fff::concaten(
            [r](int n) {std::cout << n * 2 + r << '\n';},
            [r](double n) {std::cout << n * 2 + r << '\n';},
            print_str
    );
    
    f(1);
    f(4.9);
    f("My New String");
    
    std::cout << "The sizeof f is " << sizeof(f) << '\n';
    
}

template<int N>
auto add = [](int n) -> int {return n + N;};

template<int N>
auto multiply = [](int n) -> int {return n * N;};

template<int N>
auto add_r = [](int &n) -> void {n += N;};

template<int N>
auto multiply_r = [](int &n) -> void {n *= N;};

auto drop_negative = [](int n) {
    return n >= 0 ? fff::maybe(n) : fff::maybe.make_nullopt<int>();
};

auto pass = [](const int &n) {
    std::cout << "passed " << n << '\n';
    return n;
};

void maybe_test() {
    auto may = fff::maybe(1);
    auto may_copy = may >> multiply<3> >> add<6>;
    
    std::cout << may.value() << ' ' << may_copy.value() << '\n';
    
    auto may3 = may >> multiply<60> >> drop_negative >> pass;
    static_assert(std::is_same_v<decltype(may3), decltype(may)>);
    std::cout << typeid(may).name() << ' ' << typeid(may3).name() << '\n';
    
    auto may_not = fff::maybe.make_nullopt<int>();
    may_not << multiply_r<41771>;
    
    std::cout << may_not.has_value() << '\n';
    
    
}

auto foo_factory(int a, int b, int c) {
    return [a, b, c](int x, int y) {
        return a + b + c + x + y;
    };
}

void fly_test() {
    static_assert(sizeof(std::function<int(int)>) == 32);
    
    fff::Package f;
    
    auto g = f.fly(foo_factory(1, 2, 3));
    auto h{g};
    std::cout << g(4, 5) << ' ' << h(6, 7) << '\n';
    
    static_assert(sizeof(g) == 8);
}

#endif //UNDERSCORE_CPP_DEBUG_TOOLS_H
