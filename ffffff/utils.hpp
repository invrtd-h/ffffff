#ifndef UNDERSCORE_CPP_UTILS_HPP
#define UNDERSCORE_CPP_UTILS_HPP

#include <type_traits>
#include <functional>
#include <iostream>
#include <memory>

#include "interfaces.hpp"
#include "tmf.hpp"

namespace fff {

    /**
    * A null type that works as a "null_t Object".
    * Use CRTP pattern to inherit this type and make any "null_t Object" you want.
    */
    template<typename T = void>
    struct null_i {
        const static bool nullity = true;
    };

    using null_t = null_i<>;

    template<typename T>
    using null_or_t = std::conditional_t<std::is_void_v<T>, null_t, T>;

    template<typename F, typename ...Args>
    struct NullLifted_TD_Impl {
        using type = null_or_t<std::invoke_result_t<F, Args...>>;
    };

    template<class F>
    class NullLifted : public callable_i<F, NullLifted<F>, NullLifted_TD_Impl> {
        friend callable_i<F, NullLifted, NullLifted_TD_Impl>;
        friend class NullLiftFactory;

        [[no_unique_address]] F f;

        constexpr explicit NullLifted(const F &f) noexcept : f(f) {}
        constexpr explicit NullLifted(F &&f) noexcept : f(std::move(f)) {}

        template<similar<NullLifted> Self, typename ...Args>
        constexpr static auto call_impl(Self &&self, Args &&...args)
            noexcept(noexcept(std::invoke(std::forward<Self>(self).f_fwd(), std::forward<Args>(args)...)))
                -> null_or_t<std::invoke_result_t<F, Args...>>
        {
            if constexpr (std::is_void_v<std::invoke_result_t<F, Args...>>) {
                std::invoke(std::forward<Self>(self).f_fwd(), std::forward<Args>(args)...);
                return null_t{};
            } else {
                return std::invoke(std::forward<Self>(self).f_fwd(), std::forward<Args>(args)...);
            }
        }

        constexpr       F &  f_fwd()       &  noexcept {return f;}
        constexpr const F &  f_fwd() const &  noexcept {return f;}
        constexpr       F && f_fwd()       && noexcept {return std::move(f);}
        constexpr const F && f_fwd() const && noexcept {return std::move(f);}
    };

    struct NullLiftFactory {
        template<typename F>
        constexpr auto operator()(F &&f) const noexcept -> NullLifted<F> {
            return NullLifted<F>{std::forward<F>(f)};
        }
    };

    constexpr inline NullLiftFactory null_lift;

}


/*
* fff::Once reducible_TD
*/
namespace fff {

    template<class F>
    class Once;

    template<class F>
        requires (not std::is_void_v<std::invoke_result_t<F>>)
    class Once<F> {
        friend class OnceFactory;

        [[no_unique_address]] F f;
        mutable std::invoke_result_t<F> memo;
        mutable bool flag;

        constexpr explicit Once(const F &f) noexcept : f(f), flag(false) {}
        constexpr explicit Once(F &&f) noexcept : f(std::move(f)), flag(false) {}

    public:
        constexpr auto operator()() const
            noexcept(noexcept(std::invoke(f)))
                -> std::invoke_result_t<F>
        {
            if (flag) {
                return memo;
            }
            flag = true;
            return memo = std::invoke(f);
        }
    };

    template<class F>
        requires std::is_void_v<std::invoke_result_t<F>>
    class Once<F> {
        friend class OnceFactory;

        [[no_unique_address]] F f;
        mutable bool flag;

        constexpr explicit Once(const F &f) noexcept : f(f), flag(false) {}
        constexpr explicit Once(F &&f) noexcept : f(std::move(f)), flag(false) {}

    public:
        constexpr void operator()() const
            noexcept(noexcept(std::invoke(f)))
        {
            if (flag) {
                return;
            }
            flag = true;
            std::invoke(f);
        }
    };

    struct OnceFactory {
        template<std::invocable F>
        constexpr auto operator()(F &&f) noexcept
            -> Once<std::decay_t<F>>
        {
            return Once<std::decay_t<F>>{std::forward<F>(f)};
        }
    };

    template<typename F>
    class once_f;

    template<nonvoid_invocable F>
    class once_f<F> : public callable_i<F, once_f<F>, std::invoke_result> {
        friend callable_i<F, once_f<F>, std::invoke_result>;
        friend class once_factory;

        [[no_unique_address]]   F                                   f;
        [[no_unique_address]]   mutable std::invoke_result_t<F>     memo;
                                mutable bool                        flag;

        constexpr explicit once_f(const F &f) noexcept
            : f(f),             flag(false) {}
        constexpr explicit once_f(F &&f) noexcept
            : f(std::move(f)),  flag(false) {}

        template<similar<once_f<F>> Self>
        constexpr static auto call_impl(Self &&self)
            noexcept(std::is_nothrow_invocable_v<F>)
                -> std::invoke_result_t<F>
        {
            if constexpr (debug) {
                std::cout << (not std::is_lvalue_reference_v<Self> ? "Rvalue called!\n" : "Lvalue called!\n");
                std::cout << (not std::is_lvalue_reference_v<decltype((std::forward<Self>(self).f))>
                    ? "Rvalue caught!\n" : "Lvalue caught!\n");
            }
            if (self.flag) {
                return self.memo;
            }
            self.flag = true;
            return self.memo = std::invoke(std::forward<Self>(self).f);
        }

        constexpr static bool debug = true;
    };

    struct once_factory {
        template<std::invocable F>
        constexpr auto operator()(F &&f) const noexcept
            -> once_f<std::decay_t<F>>
        {
            return once_f<std::decay_t<F>>{std::forward<F>(f)};
        }
    };

    constexpr inline once_factory once;
}

/* fff::Count reducible_TD */
namespace fff {

    template<class F>
    class Count {
        friend class CountFactory;

        [[no_unique_address]] F f;
        mutable int cnt;

        constexpr explicit Count(const F &f) noexcept : f(f), cnt(0) {}
        constexpr explicit Count(F &&f) noexcept : f(std::move(f)), cnt(0) {}

    public:
        template<class ...Args>
            requires std::invocable<F, Args...>
        constexpr auto operator()(Args ...args) const
            noexcept(noexcept(std::invoke(f, std::forward<Args>(args)...)))
                -> std::invoke_result_t<F, Args...>
        {
            ++cnt;
            return std::invoke(f, std::forward<Args>(args)...);
        }
        constexpr int get_count() const noexcept {
            return cnt;
        }
    };

    struct CountFactory {
        template<class F>
        constexpr auto operator()(F &&f) const noexcept
            -> Count<F>
        {
            return Count<F>(std::forward<F>(f));
        }
    };
}

/*
* fff::Fly reducible_TD
*/
namespace fff {
    template<class F>
    class Fly {
        std::unique_ptr<F> p;

    public:
        explicit Fly(const F &f) : p(new F(f)) {}
        explicit Fly(F &&f) : p(new F(std::move(f))) {}

        Fly(const Fly &fly) : p(new F(*fly.p)) {}
        Fly(Fly &&fly) noexcept = default;
        Fly &operator=(const Fly &fly) {
            p = std::make_unique<F>(*fly.p);
            return *this;
        }
        Fly &operator=(Fly &&) noexcept = default;

        template<class ...Args>
            requires std::invocable<F, Args...>
        auto operator()(Args &&...args) const
            noexcept(noexcept(std::invoke(*p, std::forward<Args>(args)...)))
                -> std::invoke_result_t<F, Args...>
        {
            return std::invoke(*p, std::forward<Args>(args)...);
        }

    };

    struct FlyFactory {
        template<class F>
        constexpr auto operator()(F &&f) const noexcept -> Fly<std::decay_t<F>> {
            return Fly<std::decay_t<F>>(std::forward<F>(f));
        }
    };
}

namespace fff {

    template<typename T>
    class ObjectCounter {
        inline static int created_ = 0, alive_ = 0;

    public:
        constexpr ObjectCounter() noexcept {
            ++created_; ++alive_;
        }
        constexpr ~ObjectCounter() noexcept {
            --alive_;
        }

        [[nodiscard]] static int created() noexcept {
            return created_;
        }
        [[nodiscard]] static int alive() noexcept {
            return alive_;
        }

        constexpr explicit operator bool() const noexcept {
            return true;
        }
    };

    /**
    * A type constructor that makes a singleton.
    * @tparam T The type of singleton\n
    * should hide constructors and should delete copy/move procedures\n
    * should declare AsSingle as a friend
    */
    template<typename T>
    struct AsSingle {
        T &get() noexcept(noexcept(new T())) {
            static T *data = new T();
            return *data;
        }
    };
}

#endif//UNDERSCORE_CPP_UTILS_HPP
