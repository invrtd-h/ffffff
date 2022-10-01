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

### Functions

* _.each()
* _.map()
* _.filter(), _.reject()
* _.some(), _.every(), _.none()

### Higher Order Functions

* _.once()
* _.count()
* _.compose()
* _.concaten()
* _.overload()

### Monads

#### _.pthrow(), _.pcatch

아무 기능도 없습니다! 하지만 파이프라인을 제공합니다.

#### _.maybe()

STL의 std::optional을 상속받았습니다. 값이 있을 수도 있고 없을 수도 있습니다.

### Other utilities

#### fff::AsSingle\<T\>

싱글턴을 만들어 줍니다.