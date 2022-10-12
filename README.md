# FFFFFF.cpp Project

JS의 underscore 라이브러리를 C++로 그대로 구현하는 프로젝트입니다.

왜 이름이 FFFFFF일까요? #ffffff는 흰색이기 때문입니다. 사실 별 의미 없습니다.

### underscore.js 라이브러리

함수형 프로그래밍을 지원하는 JS의 라이브러리입니다. 

## FFFFFF.cpp features

### Namespace fff::tmf

리턴 타입 추론에 쓰이는 템플릿 메타 함수들과 C++20 Concepts를 지원합니다.

### Small Function Objects

* IdentityAt
* CopyAt
* Noop
* AlwaysConstant

이름만 봐도 무엇을 하는 함수인지 유추할 수 있습니다!

### Functions

* _.each()
* _.map()
* _.filter(), _.reject()
* _.some(), _.every(), _.none()

### Higher Order Functions

* _.once()
* _.count()

#### _.overload()

 서로 다른 인자를 가진 여러 개의 함수를 묶어줍니다!

```
int main() {
    auto print_str 
        = [](const std::string &s) {std::cout << s << '\n';};
    
    auto overloaded = fff::overload(
            [](int n) { std::cout << n << '\n'; },
            [](double n) { std::cout << n << '\n'; },
            print_str
    );
    
    overloaded(1); // 1
    overloaded(4.9); // 4.9
    overloaded("String"); // String
}
```

#### _.parallel()

 _.overload()와 비슷하지만, 함수의 순서가 보다 중요해집니다. 보다 앞에 있는 함수가 실행 가능하다면 뒤에 있는 함수가 적합하더라도 앞에 있는 함수를 실행시킵니다!

```
int main() {
    auto print_str 
        = [](const std::string &s) {std::cout << s << '\n';};
    
    int r = 3;
    
    auto f = fff::parallel(
            [r](int n) {std::cout << n * 2 + r << '\n';},
            print_str,
            [r](double n) {std::cout << n * 2 + r << '\n';}
    );
    
    f(1); // 5
    f(4.9); // 11 (NOT 12.8!!)
    f("String"); // String
}
```

#### _.pipeline()

 합성함수를 파이프라인 형식으로 지원합니다!

```
template<int N>
constexpr auto add = [](int n) {return n + N;};

template<int N>
constexpr auto multiply = [](int n) {return n * N;};

int main() {
    auto g = fff::pipeline >> add<3> >> multiply<5>;
    std::cout << g(3) << '\n'; // 30 ( =(3+3)*5 )
}
```

### Monads

#### _.go(), _.stop

 아무 기능도 없습니다! 하지만 파이프라인을 제공합니다. 함수에 접근할 때 무조건 operator()을 써야 한다는 생각을 버리세요! operator>>는 여러 번의 함수를 한꺼번에 적용해야 할 때, 보다 직관적인 표기법을 만들어줍니다.

```
template<int N>
constexpr auto multiply = [](int n) {return n * N;};

int main() {
    int got = fff::go(1) 
            >> multiply<3> 
            >> multiply<3>
            >> multiply<3> 
            >> multiply<3> 
            >> fff::stop; // 81
}
```

#### _.maybe()

STL의 std::optional을 상속받았습니다. 값이 있을 수도 있고 없을 수도 있습니다.

### Other utilities

#### fff::AsSingle\<T\>

싱글턴을 만들어 줍니다.