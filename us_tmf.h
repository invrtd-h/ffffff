#ifndef UNDERSCORE_CPP_US_TMF_H
#define UNDERSCORE_CPP_US_TMF_H

namespace fff::impl::tmf {
    
    template<typename T>
    concept Printable = requires (T t) {std::cout << t;};
    
    template<class T>
    concept DefaultConstructible = requires {T();};
    
    template<template<class> class C, typename Cont>
    concept IsInstantized = std::is_same_v<C<typename Cont::value_type>, Cont>;
    
    /**
     * A concept that determines whether the given type is sequential container in the STL library.
     * @tparam Cont any container (or any type!)
     * @return whether the given container is vector, deque or array
     */
    
    template<class Cont>
    concept IsMap = requires {
        typename Cont::key_type;
        typename Cont::mapped_type;
    };
    
    template<template<class> class C>
    concept BackPushable = requires (C<int> cont) {
        cont.push_back(0);
    };
    
    template<template<class> class C>
    concept Insertable = requires (C<int> cont) {
        cont.insert(0);
    };
}

#endif //UNDERSCORE_CPP_US_TMF_H
