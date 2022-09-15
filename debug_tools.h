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


#endif //UNDERSCORE_CPP_DEBUG_TOOLS_H
