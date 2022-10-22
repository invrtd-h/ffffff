/**
* @author Hyegeun Song (Github : invrtd-h)
*/

#ifndef UNDERSCORE_CPP_PACKAGE_HPP
#define UNDERSCORE_CPP_PACKAGE_HPP

#include <type_traits>
#include <functional>
#include <algorithm>
#include <iostream>
#include <concepts>
#include <sstream>
#include <utility>
#include <memory>
#include <ranges>
#include <tuple>

#include "tmf.hpp"
#include "basic_ops.hpp"
#include "interfaces.hpp"

namespace fff {
   template<typename ...Ts>
   struct MultiReturn : public std::tuple<Ts...> {
       using std::tuple<Ts...>::tuple;

       constexpr auto to_tuple() const noexcept -> std::tuple<Ts...> {
           return static_cast<std::tuple<Ts...>>(*this);
       }

       template<std::invocable<Ts...> F>
       constexpr auto operator>>(F &&f) const
           noexcept(noexcept(std::apply(std::forward<F>(f), this->to_tuple())))
               -> std::invoke_result_t<F, Ts...>
       {
           return std::apply(std::forward<F>(f), this->to_tuple());
       }
   };

   struct MultiReturnFactory {
       template<typename ...Ts>
       constexpr auto operator()(Ts &&...ts) const noexcept
           -> MultiReturn<std::decay_t<Ts>...>
       {
           return MultiReturn<std::decay_t<Ts>...>{std::forward<Ts>(ts)...};
       }
   };

   template<typename T>
   concept mr = made_by<T, MultiReturn>;

   template<typename T>
   concept not_mr = not mr<T>;

   struct DeclMr {
       template<typename F, typename ...Ts>
       constexpr static auto declmr(F &&f, const MultiReturn<Ts...> &) noexcept
           -> std::invoke_result_t<F, Ts...>;
   };

   template<typename F, mr T>
   using apply_as_mr_result_t =
       decltype(DeclMr::declmr(std::declval<F>(), std::declval<T>()));

   constexpr inline MultiReturnFactory multi_return;
}

/*
* fff::AsSingle impl
*/
namespace fff {
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

/**
* fff::(null_t object class) impl
*/

namespace fff {

   template<typename T>
   using Null_or_t = std::conditional_t<std::is_void_v<T>, null_t, T>;

   template<typename F, typename ...Args>
   struct NullLifted_TD_Impl {
       using type = Null_or_t<std::invoke_result_t<F, Args...>>;
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
               -> Null_or_t<std::invoke_result_t<F, Args...>>
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

/**
 * fff::static_bind impl
 */

namespace fff {

    template<class_as_value ...ValueHolders>
    struct static_l_bind_TD_impl {
        template<typename F, typename ...Args>
        struct inner {
            using type = std::invoke_result_t<F, const typename ValueHolders::type &..., Args...>;
        };
    };

    template<typename F, class_as_value ...ValueHolders>
    class static_l_bind_f
        : public callable_i<F, static_l_bind_f<F, ValueHolders...>,
                            static_l_bind_TD_impl<ValueHolders...>::template inner> {

        template<auto ...vp>
        friend class static_l_bind_factory;

        friend callable_i<F, static_l_bind_f<F, ValueHolders...>,
                            static_l_bind_TD_impl<ValueHolders...>::template inner>;

        [[no_unique_address]] F f;

        constexpr explicit static_l_bind_f(const F &f) noexcept : f(f) {}
        constexpr explicit static_l_bind_f(F &&f) noexcept : f(std::move(f)) {}

        template<similar<static_l_bind_f> Self, typename ...Args>
            requires std::invocable<F, const typename ValueHolders::type &..., Args...>
        constexpr static auto call_impl(Self &&self, Args &&...args)
            noexcept(std::is_nothrow_invocable_v<F, const typename ValueHolders::type &..., Args...>)
                -> std::invoke_result_t<F, const typename ValueHolders::type &..., Args...>
        {
            return std::invoke(std::forward<Self>(self).f_fwd(),
                               ValueHolders::value...,
                               std::forward<Args>(args)...);
        }

        constexpr       F &  f_fwd()       &  noexcept {return f;}
        constexpr const F &  f_fwd() const &  noexcept {return f;}
        constexpr       F && f_fwd()       && noexcept {return std::move(f);}
        constexpr const F && f_fwd() const && noexcept {return std::move(f);}

    public:
        using function_type = F;
    };

    template<auto ...vp>
    struct static_l_bind_factory {
        template<class F>
        constexpr auto operator()(F &&f) const noexcept
            -> static_l_bind_f<std::decay_t<F>, value_holder<vp>...>
        {
            return static_l_bind_f<std::decay_t<F>, value_holder<vp>...>{std::forward<F>(f)};
        }
    };

    template<auto ...vp>
    constexpr inline static_l_bind_factory<vp...> static_l_bind;



    template<class_as_value ...ValueHolders>
    struct static_r_bind_TD_impl {
        template<typename F, typename ...Args>
        struct inner {
            using type = std::invoke_result_t<F, Args..., const typename ValueHolders::type &...>;
        };
    };

    template<typename F, class_as_value ...ValueHolders>
    class static_r_bind_f
        : public callable_i<F, static_r_bind_f<F, ValueHolders...>,
                            static_r_bind_TD_impl<ValueHolders...>::template inner> {

        template<auto ...vp>
        friend class static_r_bind_factory;

        friend callable_i<F, static_r_bind_f<F, ValueHolders...>,
                          static_r_bind_TD_impl<ValueHolders...>::template inner>;

        [[no_unique_address]] F f;

        constexpr explicit static_r_bind_f(const F &f) noexcept : f(f) {}
        constexpr explicit static_r_bind_f(F &&f) noexcept : f(std::move(f)) {}

        template<similar<static_r_bind_f> Self, typename ...Args>
            requires std::invocable<F, Args..., const typename ValueHolders::type &...>
        constexpr static auto call_impl(Self &&self, Args &&...args)
            noexcept(std::is_nothrow_invocable_v<F, Args..., const typename ValueHolders::type &...>)
                -> std::invoke_result_t<F, Args..., const typename ValueHolders::type &...>
        {
            return std::invoke(std::forward<Self>(self).f_fwd(),
                               std::forward<Args>(args)...,
                               ValueHolders::value...);
        }

        constexpr       F &  f_fwd()       &  noexcept {return f;}
        constexpr const F &  f_fwd() const &  noexcept {return f;}
        constexpr       F && f_fwd()       && noexcept {return std::move(f);}
        constexpr const F && f_fwd() const && noexcept {return std::move(f);}

    public:
        using function_type = F;
    };

    template<auto ...vp>
    struct static_r_bind_factory {
        template<class F>
        constexpr auto operator()(F &&f) const noexcept
            -> static_r_bind_f<std::decay_t<F>, value_holder<vp>...>
        {
            return static_r_bind_f<std::decay_t<F>, value_holder<vp>...>{std::forward<F>(f)};
        }
    };

    template<auto ...vp>
    constexpr inline static_r_bind_factory<vp...> static_r_bind;
}

/*
* fff::Once impl
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
}

/* fff::Count impl */
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
* fff::Log impl
*/
namespace fff {

}

/*
* fff::Fly impl
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

/**
* A simple practice for CRTP pattern (If you are not interested, then just ignore...)
*/
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

   struct MyClass : ObjectCounter<MyClass> {};

   inline MyClass m, m2, m3;
}

/**
* fff::NoThrow impl
* @todo nothrow implement
*/
namespace fff {

   template<typename F>
   class NoThrow {
       [[no_unique_address]] F f;



   public:

   };
}

/**
* fff::Log impl
*/

namespace fff {
   template<typename T>
   class Log {
       T data;
       std::stringstream log;

   public:
       constexpr explicit Log(const T &data) noexcept : data(data) {}
       constexpr explicit Log(T &&data) noexcept : data(std::move(data)) {}

       template<std::invocable<T> F>
       constexpr auto operator>>(F &&f) const
           noexcept
           -> Log<std::invoke_result_t<F, T>>
       {
           // @todo log implement
       }

       constexpr std::string emit() noexcept {
           std::string temp;
           log >> temp;

           return temp;
       }
   };
}

/*
* fff::Pipeline impl
*/
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


namespace fff {

   template<typename T, template<class, class...> class C>
   class FlatLift_i {
   public:
       using Derived = C<T>;
       using value_type = T;

       template<std::invocable<T> F>
           requires made_by<std::invoke_result_t<F, T>, C>
           constexpr auto flat_lift(F &&f)
               noexcept(noexcept(static_cast<Derived*>(this)->flatlift_impl(std::forward<F>(f))))
                   -> std::invoke_result_t<F, T>
       {
           return static_cast<Derived*>(this)->flatlift_impl(std::forward<F>(f));
       }

       template<std::invocable<T> F>
           requires made_by<std::invoke_result_t<F, T>, C>
           constexpr auto flat_lift(F &&f) const
           noexcept(noexcept(static_cast<Derived*>(this)->flatlift_impl(std::forward<F>(f))))
               -> std::invoke_result_t<F, T>
       {
           return static_cast<Derived*>(this)->flatlift_impl(std::forward<F>(f));
       }
   };

   template<typename T>
   struct Test : Lift_i<T, Test> {
       T data;

       constexpr explicit Test(T data) : data(data) {}

       template<similar<Test> Self, std::invocable<T> F>
       constexpr static auto lift_impl(Self &&self, F &&f)
           -> Test<std::invoke_result_t<F, T>>
       {
           return Test<std::invoke_result_t<F, T>>(std::invoke(std::forward<F>(f), std::forward<Self>(self).data_fwd()));
       }

       constexpr       T &  data_fwd()       &  noexcept {return data;}
       constexpr const T &  data_fwd() const &  noexcept {return data;}
       constexpr       T && data_fwd()       && noexcept {return std::move(data);}
       constexpr const T && data_fwd() const && noexcept {return std::move(data);}
   };
}


/*
* fff::go(), fff::stop impl
*/
namespace fff {
   struct Stop {};

   template<typename T>
   class On {
       static_assert(not std::is_void_v<T>, "The template type must not be void");
       static_assert(not_made_by<T, On>, "Nested type like On<On<T>> is not allowed");
       static_assert(not std::is_reference_v<T>, "Type T must not be a reference");

       T data;

   public:
       constexpr explicit On(const T &t) noexcept(noexcept(T(t)))
           : data(t) {}
       constexpr explicit On(T &&t) noexcept(noexcept(T(std::move(t))))
           : data(std::move(t)) {}

       constexpr explicit(false) operator T() const & noexcept {
           return data;
       }
       constexpr explicit(false) operator T() && noexcept {
           return std::move(data);
       }
       constexpr explicit(false) operator T() const && noexcept {
           return std::move(data);
       }

       constexpr T &&operator>>(Stop) noexcept {
           return std::move(data);
       }
       constexpr T &&operator>>(Stop) const noexcept {
           return std::move(data);
       }

       /**
        * Lift : (T -> U) -> (M<T> -> M<U>)
        * @tparam F function object type
        * @param f function object
        * @return any On\<U> object which holds the value f(t)
        */
       template<std::invocable<T> F>
       constexpr auto operator>>(F &&f) &
           noexcept(noexcept(std::invoke(std::forward<F>(f), data)))
               -> On<std::invoke_result_t<F, T>>
       {
           return On<std::invoke_result_t<F, T>>(std::invoke(std::forward<F>(f), data));
       }

       template<std::invocable<T> F>
       constexpr auto operator>>(F &&f) const &
           noexcept(noexcept(std::invoke(std::forward<F>(f), data)))
               -> On<std::invoke_result_t<F, T>>
       {
           return On<std::invoke_result_t<F, T>>(std::invoke(std::forward<F>(f), data));
       }

       template<std::invocable<T> F>
       constexpr auto operator>>(F &&f) &&
           noexcept(noexcept(std::invoke(std::forward<F>(f), std::move(data))))
               -> On<std::invoke_result_t<F, T>>
       {
           return On<std::invoke_result_t<F, T>>(std::invoke(std::forward<F>(f), std::move(data)));
       }

       template<std::invocable<T> F>
       constexpr auto operator>>(F &&f) const &&
           noexcept(noexcept(std::invoke(std::forward<F>(f), std::move(data))))
               -> On<std::invoke_result_t<F, T>>
       {
           return On<std::invoke_result_t<F, T>>(std::invoke(std::forward<F>(f), std::move(data)));
       }

       constexpr T &operator*() & noexcept {
           return data;
       }
       constexpr const T &operator*() const & noexcept {
           return data;
       }
       constexpr T &&operator*() && noexcept {
           return std::move(data);
       }
       constexpr const T &&operator*() const && noexcept {
           return std::move(data);
       }
       constexpr T *operator->() noexcept {
           return &data;
       }
       constexpr const T *operator->() const noexcept {
           return &data;
       }

       constexpr T &&unpack() const noexcept {
           return std::move(data);
       }

       using value_type = T;
   };

   struct GoFactory {
       template<typename T>
       constexpr auto operator()(T &&t) const noexcept
           -> On<std::decay_t<T>>
       {
           return On<std::decay_t<T>>(std::forward<T>(t));
       }
   };
}

/*
* fff::Maybe impl
*/
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

namespace fff {

   /**
    * Making Result-Container function obj.
    * @param cont any std::(container) with type T
    * @param func any function obj with 1 param, say, T -> U
    * @return any std::(container) with type U
    */

   struct PreallocCont {
       template<typename T, class FuncObj, size_t N>
           requires std::invocable<FuncObj, T>
           and std::is_default_constructible_v<std::invoke_result_t<FuncObj, T>>
       constexpr auto operator()(const std::array<T, N> &cont, const FuncObj &func) const noexcept {
           return std::array<std::invoke_result_t<FuncObj, T>, N>();
       }

       template<template<class> class C, typename T, class FuncObj>
           requires std::ranges::range<C<T>>
           and std::invocable<FuncObj, T>
           and std::is_default_constructible_v<std::invoke_result_t<FuncObj, T>>
       constexpr auto operator()(const C<T> &cont, const FuncObj &func) const noexcept {
           return C<std::invoke_result_t<FuncObj, T>>(cont.size());
       }
   };

   struct NewCont {
       template<class Cont, class FuncObj>
           requires std::ranges::range<Cont>
           and std::is_default_constructible_v<Cont>
       constexpr auto operator()(const Cont &cont, const FuncObj &funcObj) const noexcept {
           return Cont();
       }
   };

   struct MapExecution {
       /**
        * @todo consider if the return value of func is void
        */
       template<class T_cont, class U_cont, class FuncObj>
           requires std::ranges::range<T_cont>
           and std::ranges::range<U_cont>
           and std::is_same_v<typename std::invoke_result<FuncObj, typename T_cont::value_type>::type,
                              typename U_cont::value_type>
           constexpr auto &operator()(U_cont &u_cont, T_cont &t_cont, const FuncObj &func) const
           noexcept(noexcept(func(t_cont[0])))
       {
           auto it_t = t_cont.begin();
           auto it_u = u_cont.begin();

           while (it_t != t_cont.end()) {
               *it_u = std::invoke(func, *it_t);
               ++it_t; ++it_u;
           }

           return u_cont;
       }
   };

   struct PushExecution {
       template<class T_cont, class FuncObj>
           requires std::ranges::range<T_cont>
           and std::convertible_to<std::invoke_result_t<FuncObj, typename T_cont::value_type>, bool>
           constexpr auto &operator()(T_cont &res_cont, T_cont &var_cont, const FuncObj &func) const
           noexcept(noexcept(func(var_cont[0])))
       {
           for (const auto &t : var_cont) {
               if (std::invoke(func, t)) {
                   PushPolicy()(res_cont, t);
               }
           }

           return res_cont;
       }

       struct PushPolicy {
           /**
            * If the container has push_back() method, apply it
            */
           template<template<class> class C, typename T>
               requires backpushable<C>
           constexpr void operator()(C<T> &res_cont, const T &val) const noexcept {
               res_cont.push_back(val);
           }

           /**
            * If the container has no push_back() method and has insert() method,
            * apply it
            */
           template<template<class> class C, typename T>
               requires (not backpushable<C> and insertible<C>)
           constexpr void operator()(C<T> &res_cont, const T &val) const noexcept {
               res_cont.insert(val);
           }
       };
   };

   struct Each {
       template<class Cont, class FuncObj>
           requires std::ranges::range<Cont>
           and std::invocable<FuncObj, typename Cont::value_type &>
       constexpr void operator()(Cont &cont, const FuncObj &func) const
           noexcept(noexcept(func(cont[0])))
       {
           std::ranges::for_each(cont, func);
       }
   };

   struct Map {
       template<class Cont, class FuncObj>
           requires std::ranges::range<Cont>
           and std::invocable<FuncObj, typename Cont::value_type &>
       constexpr auto operator()(const Cont &cont, const FuncObj &func) const
           noexcept(noexcept(func(cont[0])))
       {
           auto ret = PreallocCont()(cont, func);

           {
               auto it_t = cont.begin();
               auto it_u = ret.begin();

               while (it_t != cont.end()) {
                   *it_u = std::invoke(func, *it_t);
                   ++it_t; ++it_u;
               }
           }

           return ret;
       }
   };

   struct Filter {
       template<class Cont, class FuncObj>
           requires std::ranges::range<Cont>
           and std::convertible_to<std::invoke_result_t<FuncObj, typename Cont::value_type &>, bool>
           constexpr auto operator()(const Cont &cont, const FuncObj &func) const
           noexcept(noexcept(func(cont[0])))
       {
           auto ret = NewCont()(cont, copy);

           for (const auto &v : cont) {
               if (std::invoke(func, v)) {
                   PushPolicy()(ret, v);
               }
           }

           return ret;
       }

       struct PushPolicy {
           /**
            * If the container has push_back() method, apply it
            */
           template<template<class> class C, typename T>
               requires backpushable<C>
           constexpr void operator()(C<T> &res_cont, const T &val) const noexcept {
               res_cont.push_back(val);
           }

           /**
            * If the container has no push_back() method and has insert() method,
            * apply it
            */
           template<template<class> class C, typename T>
               requires (not backpushable<C> and insertible<C>)
           constexpr void operator()(C<T> &res_cont, const T &val) const noexcept {
               res_cont.insert(val);
           }
       };
   };

   struct Reject {
       template<class Cont, class FuncObj>
           requires std::ranges::range<Cont>
           and std::convertible_to<std::invoke_result_t<FuncObj, typename Cont::value_type>, bool>
           constexpr auto operator()(const Cont &cont, const FuncObj &func) const
           noexcept(noexcept(func(cont[0])))
       {
           return Filter()(cont, std::not_fn(func));
       }
   };

   template<bool func_ret, bool ret>
   struct LogicMake {
       template<class Cont, class FuncObj>
           requires std::ranges::range<Cont>
           and std::convertible_to<std::invoke_result_t
                                   <FuncObj, std::remove_cv_t<typename Cont::value_type &>>, bool>
           constexpr auto operator()(const Cont &cont, const FuncObj &func) const
           noexcept(noexcept(func(cont[0]))) -> bool
       {
           for (auto &v : cont) {
               if (static_cast<bool>(std::invoke(func, v)) == func_ret) {
                   return ret;
               }
           }
           return not ret;
       }
   };

   using Some = LogicMake<true, true>;
   using Every = LogicMake<false, false>;
   using None = LogicMake<true, false>;

   constexpr inline Some some;
   constexpr inline Every every;
   constexpr inline None none;
}

namespace fff {
   inline Each                     each;
   inline Map                      map;
   inline Filter                   filter;
   inline Reject                   reject;

   template<std::size_t SZ>
   inline IdentityAt<SZ>           identity_at;
   template<std::size_t SZ>
   inline CopyAt<SZ>               copy_at;

   inline OnceFactory              once;
   inline CountFactory             count;

   inline FlyFactory               fly;

   inline MaybeFactory             maybe;
   inline Stop                     stop;
   inline GoFactory                go;

   inline PipelineFactory          pipeline;
}

namespace fff {
   struct Package {
       [[no_unique_address]] MultiReturnFactory multi_return{};

       [[no_unique_address]] Each each{};
       [[no_unique_address]] Map map{};
       [[no_unique_address]] Filter filter{};
       [[no_unique_address]] Reject reject{};

       [[no_unique_address]] Some some{};
       [[no_unique_address]] Every every{};
       [[no_unique_address]] None none{};

       [[no_unique_address]] always_true_f always_true{};
       [[no_unique_address]] always_false_f always_false{};

       [[no_unique_address]] NullLiftFactory null_lift{};

       [[no_unique_address]] IdentityAt<0> identity{};
       [[no_unique_address]] CopyAt<0> copy{};

       [[no_unique_address]] OnceFactory once{};
       [[no_unique_address]] CountFactory count{};

       [[no_unique_address]] FlyFactory fly;

       [[no_unique_address]] MaybeFactory maybe{};
       [[no_unique_address]] Stop stop{};
       [[no_unique_address]] GoFactory go{};

       [[no_unique_address]] PipelineFactory pipeline{};

       Package() = default;
   };
}

static_assert(std::is_empty_v<fff::Package>, "the Package class should be empty");

#endif//UNDERSCORE_CPP_PACKAGE_HPP
