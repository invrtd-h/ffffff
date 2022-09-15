#ifndef UNDERSCORE_CPP_UNDERSCORE_H
#define UNDERSCORE_CPP_UNDERSCORE_H

#include <type_traits>
#include <functional>
#include <iostream>
#include <concepts>
#include <utility>
#include <ranges>

#include <vector>
#include <array>
#include <deque>
#include <forward_list>
#include <list>

#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>

/**
 * The laboratory : New features are tested in this space.
 */
namespace us::lab {

}

/**
 * Template Meta Functions && Concepts.
 */
namespace us::tmf {
    template<typename T>
    concept Printable = requires (T t) {
        std::cout << t;
    };
    
    template<class T>
    concept DefaultConstructible = requires {
        T();
    };
    
    /**
     * A concept that determines whether the given type is sequential container in the STL library.
     * @tparam Cont any container (or any type!)
     * @return whether the given container is vector, deque or array
     */
    template<class Cont>
    concept IsStdSeqCont = requires {typename Cont::value_type;}
                           and (std::is_same_v<Cont, std::vector<typename Cont::value_type>>
                                or   std::is_same_v<Cont, std::deque<typename Cont::value_type>>
                                or (requires (Cont cont) {cont.size();}
                                    and  std::is_same_v<Cont, std::array<typename Cont::value_type, Cont().size()>>
                                ));
    
    template<class Cont>
    concept IsStdList = requires {typename Cont::value_type;}
                        and (std::is_same_v<Cont, std::list<typename Cont::value_type>>
                             or std::is_same_v<Cont, std::forward_list<typename Cont::value_type>>
                        );
    
    template<class Cont>
    concept IsStdSet = requires {typename Cont::value_type;}
                       and (std::is_same_v<Cont, std::set<typename Cont::value_type>>
                            or std::is_same_v<Cont, std::multiset<typename Cont::value_type>>
                            or std::is_same_v<Cont, std::unordered_set<typename Cont::value_type>>
                            or std::is_same_v<Cont, std::unordered_multiset<typename Cont::value_type>>
                       );
    
    template<class Cont>
    concept IsStdMap = (requires {typename Cont::key_type; typename Cont::mapped_type;})
    and (std::is_same_v<Cont, std::map      <typename Cont::key_type, typename Cont::mapped_type>>
    or   std::is_same_v<Cont, std::multimap <typename Cont::key_type, typename Cont::mapped_type>>);

    template<typename T, class Cont, typename U>
    using replace =
            std::conditional_t<
                std::is_same_v<Cont, std::vector<T>>, std::vector<U>,
            std::conditional_t<
                std::is_same_v<Cont, std::array<T, Cont().size()>>, std::array<U, Cont().size()>,
            std::conditional_t<
                std::is_same_v<Cont, std::deque<T>>, std::deque<U>,
            std::conditional_t<
                std::is_same_v<Cont, std::list<T>>, std::list<U>,
            std::conditional_t<
                std::is_same_v<Cont, std::forward_list<T>>, std::list<U>,
            std::conditional_t<
                std::is_same_v<Cont, std::set<T>>, std::set<U>,
            std::conditional_t<
                std::is_same_v<Cont, std::multiset<T>>, std::multiset<U>,
                
            void>>>>>>>;
}

namespace us {
    
    struct Each {
        template<class Cont, class FuncObj>
        requires std::ranges::range<Cont>
                 and std::invocable<FuncObj, typename Cont::value_type &>
        void operator()(Cont &cont, const FuncObj &func) const {
            for (auto &val : cont) {
                func(val);
            }
        }
    };
    
    struct Map {
        template<class Cont, class FuncObj>
        requires std::ranges::range<Cont>
                 and std::invocable<FuncObj, typename Cont::value_type &>
        auto operator()(const Cont &cont, const FuncObj &func) const;
    };
    
    struct NewDataPolicy {
        static const bool is_new_data_policy = true;
    };
    
    struct ExecutionPolicy {
        static const bool is_execution_policy = true;
    };
    
    /**
     * Making Result-Container function obj.
     * @param cont any std::(container) with type T
     * @param func any function obj with 1 param, say, T -> U
     * @return any std::(container) with type U
     *
     * @todo generate a policy that copying user-defined container
     */
    struct CopyCont : public NewDataPolicy {
        template<typename T, class FuncObj, size_t N>
        requires std::invocable<FuncObj, T>
                 and tmf::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::array<T, N> &cont, const FuncObj &func) const noexcept {
            return std::array<typename std::invoke_result<FuncObj, T>::type, N>();
        }
        
        template<typename T, class FuncObj>
        requires std::invocable<FuncObj, T>
                 and tmf::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::vector<T> &cont, const FuncObj &func) const noexcept {
            return std::vector<typename std::invoke_result<FuncObj, T>::type>(cont.size());
        }
        
        template<typename T, class FuncObj>
        requires std::invocable<FuncObj, T>
                 and tmf::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::deque<T> &cont, const FuncObj &func) const noexcept {
            return std::deque<typename std::invoke_result<FuncObj, T>::type>(cont.size());
        }
    
        template<typename T, class FuncObj>
        requires std::invocable<FuncObj, T>
                 and tmf::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::list<T> &cont, const FuncObj &func) const noexcept {
            return std::list<typename std::invoke_result<FuncObj, T>::type>(cont.size());
        }
    
        template<typename T, class FuncObj>
        requires std::invocable<FuncObj, T>
                 and tmf::DefaultConstructible<typename std::invoke_result<FuncObj, T>::type>
        auto operator()(const std::forward_list<T> &cont, const FuncObj &func) const noexcept {
            return std::forward_list<typename std::invoke_result<FuncObj, T>::type>(cont.size());
        }
    };
    
    struct NewCont : public NewDataPolicy {
        template<typename T, class FuncObj>
        requires std::invocable<FuncObj, T>
        auto operator()(const std::vector<T> &cont, const FuncObj &func) const noexcept {
            return std::vector<typename std::invoke_result<FuncObj, T>::type>();
        }
    };
    
    /**
     * Identity-return function obj.
     * @cite https://en.cppreference.com/w/cpp/utility/functional/identity
     * @param t any-variable
     * @return t
     */
    struct Identity {
        template<class T>
        constexpr T &&operator()(T &&t) const noexcept {
            return std::forward<T>(t);
        }
    };
    
    template<size_t SZ>
    struct Identity_at {
        static const bool is_new_data_policy = true;
    
        template<class T, typename ...Args>
        constexpr auto &&operator()(T &&t, Args &&...args) const noexcept {
            return Identity_at<SZ - 1>()(std::forward<Args>(args)...);
        }
    };
    
    template<>
    struct Identity_at<0> {
        static const bool is_new_data_policy = true;
        
        template<class T, typename ...Args>
        constexpr T &&operator()(T &&t, Args &&...args) const noexcept {
            return std::forward<T>(t);
        }
    };
    
    /**
     * No-operation function obj.
     * @return no_return
     */
    struct Noop {
        void operator()() const noexcept {
            // do nothing
        }
        
        template<class ...Args>
        void operator()(Args &&...args) const noexcept {
            // do nothing
        }
    };
    
    template<class ANewDataPolicy, class AnExecutionPolicy>
    requires ANewDataPolicy::is_new_data_policy
            and AnExecutionPolicy::is_execution_policy
    struct Bloop {
        using new_data_policy = ANewDataPolicy;
        using execution_policy = AnExecutionPolicy;
        
        template<class Cont, class FuncObj>
        requires std::ranges::range<Cont>
                 and std::invocable<FuncObj, typename Cont::value_type &>
        auto operator()(Cont &cont, const FuncObj &func) const {
    
            decltype(auto) ret = ANewDataPolicy()(cont, func);
    
            AnExecutionPolicy()(ret, cont, func);
    
            return ret;
        }
    };
    
    struct MapExecution : public ExecutionPolicy {
        /**
         * @todo consider if the return value of func is void
         * @tparam T_cont
         * @tparam U_cont
         * @tparam FuncObj
         * @param u_cont
         * @param t_cont
         * @param func
         * @return
         */
        template<class T_cont, class U_cont, class FuncObj>
        requires std::ranges::range<T_cont>
                and std::ranges::range<U_cont>
                and std::is_same<typename std::invoke_result<FuncObj, typename T_cont::value_type>::type,
                                 typename U_cont::value_type>::value
        auto &operator()(U_cont &u_cont, T_cont &t_cont, const FuncObj &func) const {
            auto it_t = t_cont.begin();
            auto it_u = u_cont.begin();
            
            while (it_t != t_cont.end()) {
                *it_u = func(*it_t);
                ++it_t; ++it_u;
            }
            
            return u_cont;
        }
    };
    
    using BloopEach = Bloop<Identity_at<0>, MapExecution>;
    using BloopMap = Bloop<CopyCont, MapExecution>;
    
    /*
     * Struct def part end;
     */
    
    /*
     * Implementation def part start;
     */
    
    template<class Cont, class FuncObj>
    requires std::ranges::range<Cont>
             and std::invocable<FuncObj, typename Cont::value_type &>
    auto Map::operator()(const Cont &cont, const FuncObj &func) const {
        auto ret = CopyCont()(cont, func);
        
        {
            auto it_t = cont.begin();
            auto it_u = ret.begin();
            
            while (it_t != cont.end()) {
                *it_u = func(*it_t);
                ++it_t; ++it_u;
            }
        }
        
        return ret;
    }
}

class underscore {
public:
    us::Each             each;
    us::Map              map;
    
    us::BloopEach        bloop_each;
    us::BloopMap         bloop_map;
};

static underscore __;

#endif //UNDERSCORE_CPP_UNDERSCORE_H
