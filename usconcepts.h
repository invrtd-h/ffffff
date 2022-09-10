#ifndef UNDERSCORE_CPP_USCONCEPTS_H
#define UNDERSCORE_CPP_USCONCEPTS_H

namespace usconts {
    template<typename T>
    concept Printable = requires (T t) {
        std::cout << t;
    };
    
    template<class T>
    concept DefaultConstructible = requires {
        T();
    };
}

#endif //UNDERSCORE_CPP_USCONCEPTS_H
