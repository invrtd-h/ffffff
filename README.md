# FFFFFF.cpp Project

JS의 underscore 라이브러리를 C++로 그대로 구현하는 프로젝트입니다.

왜 이름이 FFFFFF일까요? #ffffff는 흰색이기 때문입니다. 사실 별 의미 없습니다.

### underscore.js 라이브러리

함수형 프로그래밍을 지원하는 JS의 라이브러리입니다. 

## FFFFFF.cpp features

### Namespace impl::tmf

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

* _.once_factory()
* _.count_factory()
* _.concat_factory()

### Monads

* Maybe\<T\>
* _.maybe_factory()