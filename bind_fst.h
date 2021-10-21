#ifndef BIND_FST_H
#define BIND_FST_H

template<class Arg>
void bind_fst(const std::function<void(Arg)>& f, const Arg& x)
{
    f(x);
}

template<class T, class Arg>
T bind_fst(const std::function<T(Arg)>& f, const Arg& x)
{
    return f(x);
}

template<class T, class Arg, class... Args>
std::function<T(Args...)> bind_fst(const std::function<T(Arg, Args...)>& func, const Arg& var)
{
    return std::function<T(Args...)>([func, var](Args... args)
                                     { return func(var, args...); });
}

#endif /***BIND_FST_H***/