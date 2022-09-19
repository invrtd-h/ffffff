#ifndef UNDERSCORE_CPP_DEPRECATED_H
#define UNDERSCORE_CPP_DEPRECATED_H

namespace us::deprecated {
    struct PreAllocCont : public NewDataPolicy {
        template<typename T, class FuncObj, size_t N>
        requires std::invocable<FuncObj, T>
                 and tmf::DefaultConstructible<std::invoke_result_t<FuncObj, T>>
        auto operator()(const std::array<T, N> &cont, const FuncObj &func) const noexcept {
            return std::array<std::invoke_result_t<FuncObj, T>, N>();
        }
        
        template<typename T, class FuncObj>
        requires std::invocable<FuncObj, T>
                 and tmf::DefaultConstructible<std::invoke_result_t<FuncObj, T>>
        auto operator()(const std::vector<T> &cont, const FuncObj &func) const noexcept {
            return std::vector<std::invoke_result_t<FuncObj, T>>(cont.size());
        }
        
        template<typename T, class FuncObj>
        requires std::invocable<FuncObj, T>
                 and tmf::DefaultConstructible<std::invoke_result_t<FuncObj, T>>
        auto operator()(const std::deque<T> &cont, const FuncObj &func) const noexcept {
            return std::deque<std::invoke_result_t<FuncObj, T>>(cont.size());
        }
        
        template<typename T, class FuncObj>
        requires std::invocable<FuncObj, T>
                 and tmf::DefaultConstructible<std::invoke_result_t<FuncObj, T>>
        auto operator()(const std::list<T> &cont, const FuncObj &func) const noexcept {
            return std::list<std::invoke_result_t<FuncObj, T>>(cont.size());
        }
        
        template<typename T, class FuncObj>
        requires std::invocable<FuncObj, T>
                 and tmf::DefaultConstructible<std::invoke_result_t<FuncObj, T>>
        auto operator()(const std::forward_list<T> &cont, const FuncObj &func) const noexcept {
            return std::forward_list<std::invoke_result_t<FuncObj, T>>(cont.size());
        }
    };
}

#endif //UNDERSCORE_CPP_DEPRECATED_H
