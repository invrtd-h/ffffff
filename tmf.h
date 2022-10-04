#ifndef UNDERSCORE_CPP_TMF_H
#define UNDERSCORE_CPP_TMF_H

namespace fff::tmf {
    
    template<typename T, typename U = T>
    concept addable = requires (T l, U r) {l + r;};
    
    template<typename T>
    concept printable = requires (T t) {std::cout << t;};
    
    template<template<class> class C, typename Cont>
    concept is_instantized = std::is_same_v<C<typename Cont::value_type>, Cont>;
    
    /**
     * A concept that determines whether the given type is sequential container in the STL library.
     * @tparam Cont any container (or any type!)
     * @return whether the given container is vector, deque or array
     */
    
    template<class Cont>
    concept is_map = requires {
        typename Cont::key_type;
        typename Cont::mapped_type;
    };
    
    template<template<class> class C>
    concept backpushable = requires (C<int> cont) {
        cont.push_back(0);
    };
    
    template<template<class> class C>
    concept insertible = requires (C<int> cont) {
        cont.insert(0);
    };
    
    template<typename T>
    concept maybetype = requires {
        T::is_maybe;
    };
    
    template<typename T>
    concept inheritable = (std::is_class_v<T> and not std::is_final_v<T>);
    
    template<typename T>
    concept empty_type = std::is_empty_v<T>;
    
    template<typename T>
    concept nonempty_type = (not std::is_empty_v<T>);
    
    template<typename T>
    concept v_to_nv = (not std::is_void_v<std::invoke_result_t<T>>);
    
    template<typename T>
    concept v_to_v = std::is_void_v<std::invoke_result_t<T>>;
    
    template<typename T>
    concept hidden = (not std::move_constructible<T> and not std::copy_constructible<T>);
}

#endif //UNDERSCORE_CPP_TMF_H
