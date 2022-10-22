#ifndef UNDERSCORE_CPP_DEBUG_TOOLS_H
#define UNDERSCORE_CPP_DEBUG_TOOLS_H

#include "pipeline.hpp"

class ForwardingTester {
public:
    ForwardingTester() {
        std::cout << "기본 생성자 호출!" << std::endl;
    }
    ForwardingTester(const ForwardingTester &ft) {
        std::cout << "복사 생성자 호출!" << std::endl;
    }
    ForwardingTester(ForwardingTester &&ft) noexcept {
        std::cout << "이동 생성자 호출!" << std::endl;
    }
    ForwardingTester &operator=(const ForwardingTester &ft) {
        std::cout << "복사 대입 생성자 호출!" << std::endl;
        return *this;
    }
    ForwardingTester &operator=(ForwardingTester &&ft) noexcept {
        std::cout << "이동 대입 생성자 호출!" << std::endl;
        return *this;
    }
    ~ForwardingTester() {
        std::cout << "소멸자 호출!" << std::endl;
    }
    
    ForwardingTester &catch_instance(const ForwardingTester &ft) {
        std::cout << "좌측값 레퍼런스 캐치\n";
        return *this;
    }
    ForwardingTester &catch_instance(ForwardingTester &&ft) {
        std::cout << "우측값 레퍼런스 캐치\n";
        return *this;
    }

    [[nodiscard]] ForwardingTester do_any() const {
        return {};
    }

    ForwardingTester &self() & {
        return *this;
    }
    ForwardingTester self() && {
        return std::move(*this);
    }
};

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

}

template<int N>
auto add = [](int n) -> int {return n + N;};

template<int N>
auto multiply = [](int n) -> int {return n * N;};

template<int N>
auto add_r = [](int &n) -> void {n += N;};

template<int N>
auto multiply_r = [](int &n) -> void {n *= N;};

auto foo_factory(int a, int b, int c) {
    return [a, b, c](int x, int y) {
        return a + b + c + x + y;
    };
}

void pipeop_test() {
    using namespace fff::pipe_op;

    auto x = 1 | [](int n) {return 2 * n;} | [](int n) {return 3 + n;};
    std::cout << x << '\n';

    auto y = 3 | [](int n) {return fff::multi_return(n * 2, n + 3);} | std::plus<>();
    std::cout << y << '\n';
}

#endif //UNDERSCORE_CPP_DEBUG_TOOLS_H
