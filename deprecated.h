#ifndef UNDERSCORE_CPP_DEPRECATED_H
#define UNDERSCORE_CPP_DEPRECATED_H

namespace impl::deprecated {
    template<typename T>
    class Maybe {
        std::optional<T> data;
    
    public:
        using value_type = T;
        
        constexpr Maybe() : data(std::nullopt) {}
        constexpr Maybe(std::nullopt_t nullopt) : data(nullopt) {}
        constexpr Maybe(const T &t) : data(t) {}
        constexpr Maybe(T &&t) : data(std::move(t)) {}
        
        [[nodiscard]] constexpr bool has_value() const noexcept {
            return data.has_value();
        }
        
        constexpr std::optional<T> &get() noexcept {
            return data;
        }
        
        template<class F>
        requires std::invocable<F, T>
        constexpr Maybe<T> operator>>(F &&f) const
        noexcept(noexcept(f(data.value())))
        {
            if (data.has_value()) {
                return f(data.value());
            } else {
                return std::nullopt;
            }
        }
        
        template<class F>
        requires std::invocable<F, T &>
        constexpr Maybe<T> &operator<<(F &&f)
        noexcept(noexcept(f(data.value())))
        {
            if (data.has_value()) {
                f(data.value());
            }
            return *this;
        }
    };
    
    struct MaybeFactory {
        template<typename T>
        constexpr auto operator()(T &&t) const noexcept {
            return Maybe<T>(std::forward<T>(t));
        }
        
        template<typename T>
        constexpr auto make() const noexcept {
            return Maybe<T>();
        }
    };
}



#endif //UNDERSCORE_CPP_DEPRECATED_H
