#ifndef UNDERSCORE_CPP_US_TMF_H
#define UNDERSCORE_CPP_US_TMF_H

namespace fff::impl::tmf {
    
    template<typename T>
    concept Printable = requires (T t) {std::cout << t;};
    
    template<class T>
    concept DefaultConstructible = requires {T();};
    
    template<template<class> class C, typename Cont>
    concept IsInstantized = std::is_same_v<C<typename Cont::value_type>, Cont>;
    
    template<class Cont>
    concept IsStdArray = std::is_same_v<Cont, std::array<typename Cont::value_type, Cont().size()>>;
    
    template<class Cont>
    concept IsStdMap = std::is_same_v<Cont, std::map<typename Cont::key_type, typename Cont::mapped_type>>;
    
    template<class Cont>
    concept IsStdMultimap = std::is_same_v<Cont, std::multimap<typename Cont::key_type, typename Cont::mapped_type>>;
    
    template<class Cont>
    concept IsStdUnorderedMap = std::is_same_v<Cont, std::unordered_map<typename Cont::key_type, typename Cont::mapped_type>>;
    
    template<class Cont>
    concept IsStdUnorderedMultimap = std::is_same_v<Cont, std::unordered_multimap<typename Cont::key_type, typename Cont::mapped_type>>;
    
    /**
     * A concept that determines whether the given type is sequential container in the STL library.
     * @tparam Cont any container (or any type!)
     * @return whether the given container is vector, deque or array
     */
    template<class Cont>
    concept IsStdSeqCont = IsInstantized<std::vector, Cont> or IsStdArray<Cont> or IsInstantized<std::deque, Cont>;
    
    template<class Cont>
    concept IsStdLinkedList = IsInstantized<std::list, Cont> or IsInstantized<std::forward_list, Cont>;
    
    template<class Cont>
    concept IsStdGSet =    IsInstantized<std::set, Cont>
                           or IsInstantized<std::multiset, Cont>
                           or IsInstantized<std::unordered_set, Cont>
                           or IsInstantized<std::unordered_multiset, Cont>;
    
    template<class Cont>
    concept IsStdGMap = IsStdMap<Cont> or IsStdMultimap<Cont> or IsStdUnorderedMap<Cont> or IsStdUnorderedMultimap<Cont>;
    
    template<class Cont>
    concept IsStdContSizable = IsInstantized<std::vector, Cont>
                               or IsInstantized<std::deque, Cont>
                               or IsInstantized<std::list, Cont>
                               or IsInstantized<std::forward_list, Cont>;
    
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
