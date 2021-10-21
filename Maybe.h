#ifndef MAYBE_H
#define MAYBE_H

#include "bind_fst.h"
#include <memory>

using std::function;

template<class T>
class Maybe
{
    std::unique_ptr<T> _useful;

    explicit Maybe(std::unique_ptr<T>&& obj) : _useful(move(obj)), _valid(true) { }

    explicit Maybe(const std::string& s) : _valid(false), _message(s) { }

    bool _valid;
    std::string _message;
public:
    Maybe() : _valid(false), _message("null object") { }

    static Maybe<T> Right(T&& var) { return Maybe<T>(std::make_unique<T>(std::move(var))); }

    static Maybe<T> Left(const std::string& s) { return Maybe<T>(s); }

    T* get() const { return _useful.get(); }

    explicit operator bool() const { return _valid; }

    [[nodiscard]] const std::string& getMessage() const { return _message; }
};

//functor
//1 (a -> ()) -> Maybe a -> ()
template<class Arg>
void operator/(const function<void(Arg)>& func, const Maybe<Arg>& var)
{
    if (var) bind_fst(func, *var.get());
}

//2 (a -> b) -> Maybe a -> Maybe b
template<class T, class Arg>
Maybe<T> operator/(const function<T(Arg)>& func, const Maybe<Arg>& var)
{
    if (var) return Maybe<T>::Right(bind_fst(func, *var.get()));
    return Maybe<T>::Left(var.getMessage());
}

//3 (a -> b -> c) -> Maybe a -> Maybe (b -> c)
template<class T, class Arg1, class Arg2, class... Args>
Maybe<function<T(Arg2, Args...)>>
operator/(const function<T(Arg1, Arg2, Args...)>& func, const Maybe<Arg1>& var)
{
    if (var) return Maybe<function<T(Arg2, Args...)>>::Right(bind_fst(func, *var.get()));
    return Maybe<function<T(Arg2, Args...)>>::Left(var.getMessage());
}

//applicative
//4 Maybe (a -> ()) -> Maybe a -> ()
template<class Arg>
void operator*(const Maybe<function<void(Arg)>>& func, const Maybe<Arg>& val)
{
    if (func && val) bind_fst(*func.get(), *val.get());
}

//5 Maybe (a -> b) -> Maybe a -> Maybe b
template<class T, class Arg>
Maybe<T> operator*(const Maybe<function<T(Arg)>>& func, const Maybe<Arg>& val)
{
    if (!func || !val) return Maybe<T>::Left(func.getMessage()+val.getMessage());
    return Maybe<T>::Right(bind_fst(*func.get(), *val.get()));
}

//6 Maybe (a -> b -> c) -> Maybe a -> Maybe (b -> c)
template<class T, class Arg1, class Arg2, class... Args>
Maybe<function<T(Arg2, Args...)>>
operator*(const Maybe<function<T(Arg1, Arg2, Args...)>>& func, const Maybe<Arg1>& val)
{
    if (!func || !val)
        return Maybe<function<T(Arg2, Args...)>>::Left(func.getMessage()+val.getMessage());
    return Maybe<function<T(Arg2, Args...)>>::Right(bind_fst(*func.get(), *val.get()));
}

//Monad
//7 (a -> Maybe b) -> maybe a -> maybe b
template<class T, class Arg>
Maybe<T> operator>>(const function<Maybe<T>(Arg)>& func, const Maybe<Arg>& val)
{
    if (!val) return Maybe<T>::Left(val.getMessage());
    return bind_fst(func, *val.get());
}

#endif /***MAYBE_H***/