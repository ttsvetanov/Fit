/*=============================================================================
    Copyright (c) 2015 Paul Fultz II
    compress.h
    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef FIT_GUARD_COMPRESS_H
#define FIT_GUARD_COMPRESS_H

/// compress
/// ========
/// 
/// Description
/// -----------
/// 
/// The `compress` function adaptor uses a binary function to apply a
/// [fold](https://en.wikipedia.org/wiki/Fold_%28higher-order_function%29)
/// operation to the arguments passed to the function. Additionally, an
/// optional initial state can be provided, otherwise the first argument is
/// used as the initial state.
/// 
/// The arguments to the binary function, take first the state and then the
/// argument.
/// 
/// Synopsis
/// --------
/// 
///     template<class F, class State>
///     constexpr compress_adaptor<F, State> compress(F f, State s);
/// 
///     template<class F>
///     constexpr compress_adaptor<F> compress(F f);
/// 
/// Semantics
/// ---------
/// 
///     assert(compress(f, z)() == z);
///     assert(compress(f, z)(x, xs...) == compress(f, f(z, x))(xs...));
///     assert(compress(f)(x) == x);
///     assert(compress(f)(x, y, xs...) == compress(f)(f(x, y), xs...));
/// 
/// Requirements
/// ------------
/// 
/// State must be:
/// 
/// * CopyConstructible
/// 
/// F must be:
/// 
/// * [BinaryCallable](BinaryCallable)
/// * MoveConstructible
/// 
/// Example
/// -------
/// 
///     #include <fit.hpp>
///     #include <cassert>
/// 
///     struct max_f
///     {
///         template<class T, class U>
///         constexpr T operator()(T x, U y) const
///         {
///             return x > y ? x : y;
///         }
///     };
///     int main() {
///         assert(fit::compress(max_f())(2, 3, 4, 5) == 5);
///     }
/// 
/// References
/// ----------
/// 
/// * [Fold](https://en.wikipedia.org/wiki/Fold_(higher-order_function))
/// * [Variadic sum](<Variadic sum>)
/// 

#include <fit/detail/callable_base.hpp>
#include <fit/detail/delegate.hpp>
#include <fit/detail/compressed_pair.hpp>
#include <fit/detail/move.hpp>
#include <fit/detail/make.hpp>
#include <fit/detail/static_const_var.hpp>

namespace fit { namespace detail {

struct v_fold
{
    FIT_RETURNS_CLASS(v_fold);
    template<class F, class State, class T, class... Ts>
    constexpr FIT_SFINAE_MANUAL_RESULT(const v_fold&, id_<const F&>, result_of<const F&, id_<State>, id_<T>>, id_<Ts>...)
    operator()(const F& f, State&& state, T&& x, Ts&&... xs) const FIT_SFINAE_MANUAL_RETURNS
    (
        (*FIT_CONST_THIS)(f, f(FIT_FORWARD(State)(state), FIT_FORWARD(T)(x)), FIT_FORWARD(Ts)(xs)...)
    );

    template<class F, class State>
    constexpr State operator()(const F&, State&& state) const noexcept
    {
        return FIT_FORWARD(State)(state);
    }
};

}

template<class F, class State=void>
struct compress_adaptor
: detail::compressed_pair<detail::callable_base<F>, State>
{
    typedef detail::compressed_pair<detail::callable_base<F>, State> base_type;
    FIT_INHERIT_CONSTRUCTOR(compress_adaptor, base_type)

    template<class... Ts>
    constexpr const detail::callable_base<F>& base_function(Ts&&... xs) const noexcept
    {
        return this->first(xs...);
    }

    template<class... Ts>
    constexpr State get_state(Ts&&... xs) const noexcept
    {
        return this->second(xs...);
    }

    FIT_RETURNS_CLASS(compress_adaptor);

    template<class... Ts>
    constexpr FIT_SFINAE_RESULT(detail::v_fold, id_<const detail::callable_base<F>&>, id_<State>, id_<Ts>...)
    operator()(Ts&&... xs) const FIT_SFINAE_RETURNS
    (
        detail::v_fold()(
            FIT_MANGLE_CAST(const detail::callable_base<F>&)(FIT_CONST_THIS->base_function(xs...)), 
            FIT_MANGLE_CAST(State)(FIT_CONST_THIS->get_state(xs...)), 
            FIT_FORWARD(Ts)(xs)...
        )
    )
};


template<class F>
struct compress_adaptor<F, void>
: detail::callable_base<F>
{
    FIT_INHERIT_CONSTRUCTOR(compress_adaptor, detail::callable_base<F>)

    template<class... Ts>
    constexpr const detail::callable_base<F>& base_function(Ts&&... xs) const noexcept
    {
        return always_ref(*this)(xs...);
    }

    FIT_RETURNS_CLASS(compress_adaptor);

    template<class... Ts>
    constexpr FIT_SFINAE_RESULT(detail::v_fold, id_<const detail::callable_base<F>&>, id_<Ts>...)
    operator()(Ts&&... xs) const FIT_SFINAE_RETURNS
    (
        detail::v_fold()(
            FIT_MANGLE_CAST(const detail::callable_base<F>&)(FIT_CONST_THIS->base_function(xs...)), 
            FIT_FORWARD(Ts)(xs)...
        )
    )
};

FIT_DECLARE_STATIC_VAR(compress, detail::make<compress_adaptor>);

} // namespace fit

#endif
