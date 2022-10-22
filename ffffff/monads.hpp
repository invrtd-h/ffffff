#ifndef UNDERSCORE_CPP_MONADS_HPP
#define UNDERSCORE_CPP_MONADS_HPP

#include <functional>
#include <optional>
#include "tmf.hpp"

namespace fff {
    template<typename T>
    class Maybe : public std::optional<T> {
    public:
        constexpr static bool is_maybe = true;

        using std::optional<T>::optional;

        /**
        * Lift : (T -> U) -> (M\<T> -> M\<U>)
        * @tparam F Function Object Type
        * @param f Function Object
        * @return f(x) if x is not empty, std::nullopt if empty
        */
        template<class F>
            requires (std::invocable<F, T>
                     and not std::is_void_v<std::invoke_result_t<F, T>>
                     and not maybetype<std::invoke_result_t<F, T>>)
        constexpr Maybe<std::invoke_result_t<F, T>> operator>>(F &&f) const
            noexcept(noexcept(std::invoke(std::forward<F>(f), this->value())))
        {
            if (this->has_value()) {
                return std::invoke(std::forward<F>(f), this->value());
            } else {
                return std::nullopt;
            }
        }

        /**
        * Flatlift : (T -> M\<U>) -> (M\<T> -> M\<U>)
        * @tparam F Function Object Type
        * @param f Function Object
        * @return
        */
        template<class F>
            requires (std::invocable<F, T>
                     and maybetype<std::invoke_result_t<F, T>>)
        constexpr std::invoke_result_t<F, T> operator>>(F &&f) const
            noexcept(noexcept(std::invoke(std::forward<F>(f), this->value())))
        {
            if (this->has_value()) {
                return std::invoke(std::forward<F>(f), this->value());
            } else {
                return std::nullopt;
            }
        }

        template<class F>
            requires std::invocable<F, T &>
        constexpr Maybe<T> &operator<<(F &&f)
            noexcept(noexcept(std::invoke(std::forward<F>(f), this->value())))
        {
            if (this->has_value()) {
                std::invoke(std::forward<F>(f), this->value());
            }
            return *this;
        }
    };

    struct MaybeFactory {
        template<typename T>
        constexpr auto operator()(T &&t) const noexcept
            -> Maybe<std::decay_t<T>>
        {
            return Maybe<std::decay_t<T>>(std::forward<T>(t));
        }

        template<typename T>
        constexpr auto make_nullopt() const noexcept
            -> Maybe<T>
        {
            return Maybe<T>();
        }
    };
}

#endif//UNDERSCORE_CPP_MONADS_HPP
