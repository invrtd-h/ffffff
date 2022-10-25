#ifndef UNDERSCORE_CPP_OVERLOAD_HPP
#define UNDERSCORE_CPP_OVERLOAD_HPP

#include <type_traits>
#include <functional>
#include <utility>

/*
* fff::Overload reducible_TD
*/
namespace fff {
    template<class ...Fp>
    struct Overload : Fp... {
        using Fp::operator()...;
    };

    struct OverloadFactory {
        template<class ...Fp>
        constexpr auto operator()(Fp &&...fp) const noexcept
            -> Overload<std::decay_t<Fp>...>
        {
            return {std::forward<Fp>(fp)...};
        }
    };
}

/*
* fff::Parallel reducible_TD
*/

namespace fff {
    template<class F, class ...Fp>
    class Parallel;

    template<class F>
    class Parallel<F> {
        friend class ParallelFactory;

        [[no_unique_address]] F f;

    public:
        constexpr explicit Parallel(const F &f) noexcept : f(f) {}
        constexpr explicit Parallel(F &&f) noexcept : f(std::move(f)) {}

        template<typename ...Args>
            requires std::invocable<F, Args...>
        constexpr auto operator()(Args &&...args) const &
            noexcept(noexcept(std::invoke(f, std::forward<Args>(args)...)))
                -> std::invoke_result_t<F, Args...>
        {
            return std::invoke(f, std::forward<Args>(args)...);
        }

        template<typename ...Args>
            requires std::invocable<F, Args...>
        constexpr auto operator()(Args &&...args) &&
            noexcept(noexcept(std::invoke(std::move(f), std::forward<Args>(args)...)))
                -> std::invoke_result_t<F, Args...>
        {
            return std::invoke(std::move(f), std::forward<Args>(args)...);
        }

        template<typename ...Args>
            requires std::invocable<F, Args...>
        constexpr auto operator()(Args &&...args) const &&
            noexcept(noexcept(std::invoke(std::move(f), std::forward<Args>(args)...)))
                -> std::invoke_result_t<F, Args...>
        {
            return std::invoke(std::move(f), std::forward<Args>(args)...);
        }

        template<typename G>
        constexpr auto make_chain(G &&g) const & noexcept -> Parallel<F, std::decay_t<G>> {
            return Parallel<F, std::decay_t<G>>{f, std::forward<G>(g)};
        }

        template<typename G>
        constexpr auto make_chain(G &&g) && noexcept -> Parallel<F, std::decay_t<G>> {
            return Parallel<F, std::decay_t<G>>{std::move(f), std::forward<G>(g)};
        }

        template<typename G>
        constexpr auto make_chain(G &&g) const && noexcept -> Parallel<F, std::decay_t<G>> {
            return Parallel<F, std::decay_t<G>>{std::move(f), std::forward<G>(g)};
        }
    };

    template<class F, class ...Fp>
    class Parallel {
        friend class ParallelFactory;

        [[no_unique_address]] F f;
        [[no_unique_address]] Parallel<Fp...> pfp;

    public:
        template<class U1, class U2>
        constexpr Parallel(U1 &&f, U2 &&pfp) noexcept
            : f(std::forward<U1>(f)), pfp(std::forward<U2>(pfp)) {}

        template<typename ...Args>
            requires std::invocable<F, Args...>
        constexpr auto operator()(Args &&...args) const &
            noexcept(noexcept(std::invoke(f, std::forward<Args>(args)...)))
                -> std::invoke_result_t<F, Args...>
        {
            return std::invoke(f, std::forward<Args>(args)...);
        }

        template<typename ...Args>
            requires std::invocable<F, Args...>
        constexpr auto operator()(Args &&...args) &&
            noexcept(noexcept(std::invoke(std::move(f), std::forward<Args>(args)...)))
                -> std::invoke_result_t<F, Args...>
        {
            return std::invoke(std::move(f), std::forward<Args>(args)...);
        }

        template<typename ...Args>
            requires std::invocable<F, Args...>
        constexpr auto operator()(Args &&...args) const &&
            noexcept(noexcept(std::invoke(std::move(f), std::forward<Args>(args)...)))
                -> std::invoke_result_t<F, Args...>
        {
            return std::invoke(std::move(f), std::forward<Args>(args)...);
        }

        template<typename ...Args>
            requires (not std::invocable<F, Args...>
                     and std::invocable<Parallel<Fp...>, Args...>)
        constexpr auto operator()(Args &&...args) const &
            noexcept(noexcept(std::invoke(pfp, std::forward<Args>(args)...)))
                -> std::invoke_result_t<Parallel<Fp...>, Args...>
        {
            return std::invoke(pfp, std::forward<Args>(args)...);
        }

        template<typename ...Args>
            requires (not std::invocable<F, Args...>
                     and std::invocable<Parallel<Fp...>, Args...>)
        constexpr auto operator()(Args &&...args) &&
            noexcept(noexcept(std::invoke(std::move(pfp), std::forward<Args>(args)...)))
                -> std::invoke_result_t<Parallel<Fp...>, Args...>
        {
            return std::invoke(std::move(pfp), std::forward<Args>(args)...);
        }

        template<typename ...Args>
            requires (not std::invocable<F, Args...>
                     and std::invocable<Parallel<Fp...>, Args...>)
        constexpr auto operator()(Args &&...args) const &&
            noexcept(noexcept(std::invoke(std::move(pfp), std::forward<Args>(args)...)))
                -> std::invoke_result_t<Parallel<Fp...>, Args...>
        {
            return std::invoke(std::move(pfp), std::forward<Args>(args)...);
        }

        template<typename G>
        constexpr auto make_chain(G &&g) const & noexcept
            -> Parallel<F, Fp..., std::decay_t<G>>
        {
            return Parallel<F, Fp..., std::decay_t<G>>{f, pfp.make_chain(std::forward<G>(g))};
        }

        template<typename G>
        constexpr auto make_chain(G &&g) && noexcept
            -> Parallel<F, Fp..., std::decay_t<G>>
        {
            return Parallel<F, Fp..., std::decay_t<G>>{std::move(f), pfp.make_chain(std::forward<G>(g))};
        }

        template<typename G>
        constexpr auto make_chain(G &&g) const && noexcept
            -> Parallel<F, Fp..., std::decay_t<G>>
        {
            return Parallel<F, Fp..., std::decay_t<G>>{std::move(f), pfp.make_chain(std::forward<G>(g))};
        }
    };

    struct ParallelFactory {
        template<class F>
        constexpr auto operator()(F &&f) const noexcept -> Parallel<std::decay_t<F>>
        {
            return Parallel<std::decay_t<F>>(std::forward<F>(f));
        }

        template<class F, class ...Fp>
        constexpr auto operator()(F &&f, Fp &&...fp) const noexcept
            -> Parallel<std::decay_t<F>, std::decay_t<Fp>...>
        {
            return Parallel<std::decay_t<F>, std::decay_t<Fp>...>
                (std::forward<F>(f), operator()(std::forward<Fp>(fp)...));
        }

        template<typename F>
        constexpr auto make_chain(F &&f) const noexcept -> Parallel<std::decay_t<F>> {
            return Parallel<F>{std::forward<F>(f)};
        }
    };

    constexpr OverloadFactory overload;
    constexpr ParallelFactory parallel;
}

#endif//UNDERSCORE_CPP_OVERLOAD_HPP
