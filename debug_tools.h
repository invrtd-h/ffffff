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
        return 'x';
    });
    
    once(); once();
}

void concat_test() {
    auto print_str = [](const std::string &s) {std::cout << s << '\n';};
    
    auto concated = fff::concat(
            [](int n) {std::cout << n << '\n';},
            [](double n) { std::cout << n << '\n'; },
            print_str
    );
    
    concated(1);
    concated(4.9);
    concated("String");
    
    auto overloaded = fff::overload(
            [](int n) { std::cout << n << '\n'; },
            [](double n) { std::cout << n << '\n'; },
            print_str
    );
    
    overloaded(1);
    overloaded(4.9);
    overloaded("String");
    
}

template<int N>
auto add = [](int n) -> int {return n + N;};

template<int N>
auto multiply = [](int n) -> int {return n * N;};

template<int N>
auto add_r = [](int &n) -> void {n += N;};

template<int N>
auto multiply_r = [](int &n) -> void {n *= N;};

void maybe_test() {
    auto may = fff::maybe(1);
    auto may_copy = may >> multiply<3> >> add<6>;
    
    std::cout << may.value() << ' ' << may_copy.value() << '\n';
    
    may << multiply_r<41771> << add_r<7110>;
    
    std::cout << may.value() << '\n';
    
    auto may_not = fff::maybe.make<int>();
    may_not << multiply_r<41771>;
    
    std::cout << may_not.has_value() << '\n';
}

#endif //UNDERSCORE_CPP_DEBUG_TOOLS_H
