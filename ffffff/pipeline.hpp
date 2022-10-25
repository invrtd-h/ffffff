#ifndef UNDERSCORE_CPP_PIPELINE_HPP
#define UNDERSCORE_CPP_PIPELINE_HPP

#include <functional>

#include "multiargs.hpp"

namespace fff {

    template<class F, class ...Fp>
    struct Pipeline;

    template<class F>
    class Pipeline<F> {
        friend class PipelineFactory;

        [[no_unique_address]] F f;

    public:
        constexpr explicit Pipeline(const F &f) noexcept : f(f) {}
        constexpr explicit Pipeline(F &&f) noexcept : f(std::move(f)) {}

        template<class ...Args>
            requires std::invocable<F, Args...>
        constexpr auto operator()(Args &&...args) const &
            noexcept(noexcept(std::invoke(f, std::forward<Args>(args)...)))
                -> std::invoke_result_t<F, Args...>
        {
            return std::invoke(f, std::forward<Args>(args)...);
        }

        template<class ...Args>
            requires std::invocable<F, Args...>
        constexpr auto operator()(Args &&...args) &&
            noexcept(noexcept(std::invoke(std::move(f), std::forward<Args>(args)...)))
                -> std::invoke_result_t<F, Args...>
        {
            return std::invoke(std::move(f), std::forward<Args>(args)...);
        }

        template<class ...Args>
            requires std::invocable<F, Args...>
        constexpr auto operator()(Args &&...args) const &&
            noexcept(noexcept(std::invoke(std::move(f), std::forward<Args>(args)...)))
                -> std::invoke_result_t<F, Args...>
        {
            return std::invoke(std::move(f), std::forward<Args>(args)...);
        }

        template<class G>
        constexpr auto operator|(G &&g) const & noexcept -> Pipeline<F, std::decay_t<G>> {
            return Pipeline<F, std::decay_t<G>>{f, std::forward<G>(g)};
        }

        template<class G>
        constexpr auto operator|(G &&g) && noexcept -> Pipeline<F, std::decay_t<G>> {
            return Pipeline<F, std::decay_t<G>>{std::move(f), std::forward<G>(g)};
        }

        template<class G>
        constexpr auto operator|(G &&g) const && noexcept -> Pipeline<F, std::decay_t<G>> {
            return Pipeline<F, std::decay_t<G>>{std::move(f), std::forward<G>(g)};
        }
    };

    template<class F1, class ...Fp>
    class Pipeline {
        friend class PipelineFactory;

        [[no_unique_address]] F1 f1;
        [[no_unique_address]] Pipeline<Fp...> f2;

    public:
        template<class U1, class U2>
        constexpr Pipeline(U1 &&f1, U2 &&f2) noexcept
            : f1(std::forward<U1>(f1)), f2(std::forward<U2>(f2)) {}

        template<class ...Args>
            requires not_mr<std::invoke_result_t<F1, Args...>>
        constexpr auto operator()(Args &&...args) const &
            noexcept(noexcept(std::invoke(f2, std::invoke(f1, std::forward<Args>(args)...))))
                -> std::invoke_result_t<Pipeline<Fp...>, std::invoke_result_t<F1, Args...>>
        {
            return std::invoke(f2, std::invoke(f1, std::forward<Args>(args)...));
        }

        template<class ...Args>
            requires not_mr<std::invoke_result_t<F1, Args...>>
        constexpr auto operator()(Args &&...args) &&
            noexcept(noexcept(std::invoke(std::move(f2), std::invoke(std::move(f1), std::forward<Args>(args)...))))
                -> std::invoke_result_t<Pipeline<Fp...>, std::invoke_result_t<F1, Args...>>
        {
            return std::invoke(std::move(f2), std::invoke(std::move(f1), std::forward<Args>(args)...));
        }

        template<class ...Args>
            requires not_mr<std::invoke_result_t<F1, Args...>>
        constexpr auto operator()(Args &&...args) const &&
            noexcept(noexcept(std::invoke(std::move(f2), std::invoke(std::move(f1), std::forward<Args>(args)...))))
                -> std::invoke_result_t<Pipeline<Fp...>, std::invoke_result_t<F1, Args...>>
        {
            return std::invoke(std::move(f2), std::invoke(std::move(f1), std::forward<Args>(args)...));
        }

        template<class ...Args>
            requires mr<std::invoke_result_t<F1, Args...>>
        constexpr auto operator()(Args &&...args) const &
            noexcept(noexcept(std::apply(f2, std::invoke(f1, std::forward<Args>(args)...).to_tuple())))
                -> apply_as_mr_result_t<Pipeline<Fp...>, std::invoke_result_t<F1, Args...>>
        {
            return std::apply(f2, std::invoke(f1, std::forward<Args>(args)...).to_tuple());
        }

        template<class ...Args>
            requires mr<std::invoke_result_t<F1, Args...>>
        constexpr auto operator()(Args &&...args) &&
            noexcept(noexcept(std::apply(std::move(f2), std::invoke(std::move(f1), std::forward<Args>(args)...).to_tuple())))
                -> apply_as_mr_result_t<Pipeline<Fp...>, std::invoke_result_t<F1, Args...>>
        {
            return std::apply(std::move(f2), std::invoke(std::move(f1), std::forward<Args>(args)...).to_tuple());
        }

        template<class ...Args>
            requires mr<std::invoke_result_t<F1, Args...>>
        constexpr auto operator()(Args &&...args) const &&
            noexcept(noexcept(std::apply(std::move(f2), std::invoke(std::move(f1), std::forward<Args>(args)...).to_tuple())))
                -> apply_as_mr_result_t<Pipeline<Fp...>, std::invoke_result_t<F1, Args...>>
        {
            return std::apply(std::move(f2), std::invoke(std::move(f1), std::forward<Args>(args)...).to_tuple());
        }

        template<class G>
        constexpr auto operator|(G &&g) const & noexcept
            -> Pipeline<F1, Fp..., std::decay_t<G>>
        {
            return Pipeline<F1, Fp..., std::decay_t<G>>{f1, f2 | std::forward<G>(g)};
        }

        template<class G>
        constexpr auto operator|(G &&g) && noexcept
            -> Pipeline<F1, Fp..., std::decay_t<G>>
        {
            return Pipeline<F1, Fp..., std::decay_t<G>>{std::move(f1), std::move(f2) | std::forward<G>(g)};
        }

        template<class G>
        constexpr auto operator|(G &&g) const && noexcept
            -> Pipeline<F1, Fp..., std::decay_t<G>>
        {
            return Pipeline<F1, Fp..., std::decay_t<G>>{std::move(f1), std::move(f2) | std::forward<G>(g)};
        }
    };

    struct PipelineFactory {
        template<class F>
        constexpr auto operator()(F &&f) const noexcept
            -> Pipeline<std::decay_t<F>>
        {
            return Pipeline<std::decay_t<F>>{std::forward<F>(f)};
        }

        template<class F, class ...Fp>
        constexpr auto operator()(F &&f, Fp &&...fp) const noexcept
            -> Pipeline<std::decay_t<F>, std::decay_t<Fp>...>
        {
            return Pipeline<std::decay_t<F>, std::decay_t<Fp>...>
                {std::forward<F>(f), operator()(std::forward<Fp>(fp)...)};
        }

        template<class F>
        constexpr auto operator|(F &&f) const noexcept
            -> Pipeline<std::decay_t<F>>
        {
            return Pipeline<std::decay_t<F>>{std::forward<F>(f)};
        }
    };
}


namespace fff::pipe_op {
    template<typename T, std::invocable<T> F>
    constexpr auto operator|(T &&t, F &&f)
        noexcept(std::is_nothrow_invocable_v<F, T>)
            -> std::invoke_result_t<F, T>
    {
        return std::invoke(std::forward<F>(f), std::forward<T>(t));
    }

    template<mr T, applicable<T> F>
    constexpr auto operator|(T &&t, F &&f)
        noexcept(noexcept(std::apply(std::forward<F>(f), std::forward<T>(t).to_tuple())))
            -> apply_as_mr_result_t<F, T>
    {
        return std::apply(std::forward<F>(f), std::forward<T>(t).to_tuple());
    }
}


#endif//UNDERSCORE_CPP_PIPELINE_HPP
