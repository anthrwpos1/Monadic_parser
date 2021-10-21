#ifndef PARSING_H
#define PARSING_H

#include <sstream>
#include <vector>
#include <functional>
#include "bind_fst.h"
#include "data_stream.h"
#include "Maybe.h"

/*
 * The monadic parser
 * functional programming inside!
 */

/*
 * Polymorphic state-value pair
 * uses std::unique_ptr ability to safely contain no value
 * todo: remove with c++20 new features
 */
template<class T> using parser_out = std::pair<std::unique_ptr<T>, parsing_state>;

//Parser is a function from 'state' to the pair 'new state - value'
template<class T> using parser_type = std::function<parser_out<T>(parsing_state)>;

template<class T>
Maybe<T> run_parser(const parser_type<T> &parser, const char *input)
{
    auto result = parser(parsing_state(input, parsing_fault_data(true)));
    auto state = result.second;
    if (state.is_valid()) return Maybe<T>::Right(std::move(*result.first));
    std::stringstream s;
    s << "parsing fail:\n" << state;
    return Maybe<T>::Left(s.str());
}

//functor

/*
 * 1: (a -> b) -> parser a -> parser b
 */
template<class T, class Arg>
parser_type<T> operator/(const function<T(Arg)> &func, const parser_type<Arg> &val)
{
    return function([&](parsing_state ps)
                    {
                        parser_out<Arg> left = val(ps);
                        std::unique_ptr<Arg> x = std::move(left.first);
                        parsing_state new_ps = left.second;

                        std::unique_ptr<T> y;
                        if (new_ps.is_valid()) y = std::make_unique<T>(func(*x));
                        return parser_out<T>(std::move(y), parsing_state::join(ps, new_ps));
                    });
}

/*
 * 2: (a -> b -> c) -> parser a -> parser (b -> c)
 */
template<class T, class Arg1, class Arg2, class... Args>
parser_type<function<T(Arg2, Args...)>>
operator/(const function<T(Arg1, Arg2, Args...)> &func, const parser_type<Arg1> &val)
{
    return function([&](parsing_state ps)
                    {
                        parser_out<Arg1> left = val(ps);
                        std::unique_ptr<Arg1> x = std::move(left.first);
                        parsing_state new_ps = left.second;

                        std::unique_ptr<function<T(Arg2, Args...)>> y;
                        if (new_ps.is_valid()) y = std::make_unique<function<T(Arg2, Args...)>>(bind_fst(func, *x));
                        return parser_out<function<T(Arg2, Args...)>>(std::move(y), parsing_state::join(ps, new_ps));
                    });
}

//applicative

/*
 * 3:  parser (a -> b) -> parser a -> parser b
 */
template<class T, class Arg>
parser_type<T> operator*(const parser_type<function<T(Arg)>> &func, const parser_type<Arg> &val)
{
    return function([&](parsing_state ps)
                    {
                        parser_out<function<T(Arg)>> left = func(ps);
                        std::unique_ptr<function<T(Arg)>> f = std::move(left.first);
                        parsing_state ps1 = left.second;

                        parser_out<Arg> right = val(ps1);
                        std::unique_ptr<Arg> x = std::move(right.first);
                        parsing_state ps2 = right.second;
                        parsing_state new_ps = parsing_state::both(ps1, ps2);

                        std::unique_ptr<T> y;
                        if (new_ps.is_valid()) y = std::make_unique<T>((*f)(*x));
                        return parser_out<T>(std::move(y), parsing_state::join(ps, new_ps));
                    });
}

/*
 * 4: parser (a -> b -> c) -> parser a -> parser (b -> c)
 */
template<class T, class Arg1, class Arg2, class... Args>
parser_type<function<T(Arg2, Args...)>>
operator*(const parser_type<function<T(Arg1, Arg2, Args...)>> &func, const parser_type<Arg1> &val)
{
    return function([&](parsing_state ps)
                    {
                        parser_out<function<T(Arg1, Arg2, Args...)>> left = func(ps);
                        std::unique_ptr<function<T(Arg1, Arg2, Args...)>> f = std::move(left.first);
                        parsing_state ps1 = left.second;

                        parser_out<Arg1> right = val(ps1);
                        std::unique_ptr<Arg1> x = std::move(right.first);
                        parsing_state ps2 = right.second;

                        parsing_state new_ps = parsing_state::both(ps1, ps2);
                        std::unique_ptr<function<T(Arg2, Args...)>> y;
                        if (new_ps.is_valid()) y = std::make_unique<function<T(Arg2, Args...)>>(bind_fst(*f, *x));
                        return parser_out<function<T(Arg2, Args...)>>(std::move(y), parsing_state::join(ps, new_ps));
                    });
}

//Monad
//7: parser a -> (a -> parser b) -> parser b
template<class T, class Arg>
parser_type<T> operator>>(const parser_type<Arg> &val, const function<parser_type<T>(Arg)> &func)
{
    return function([&](parsing_state ps)
                    {
                        parser_out<Arg> right = val(ps);
                        std::unique_ptr<Arg> x = std::move(right.first);
                        parsing_state ps1 = right.second;
                        if (!ps1.is_valid()) return parser_out<T>(std::unique_ptr<T>(), parsing_state::join(ps, ps1));

                        parser_type<T> new_val = func(*x);
                        return new_val(ps1);
                    });
}

//8: parser a -> parser b -> parser b
template<class T, class Arg>
parser_type<T> operator>>(const parser_type<Arg> &pa, const parser_type<T> &pb)
{
    return function([&](parsing_state ps)
                    {
                        parser_out<Arg> right = pa(ps);
                        parsing_state ps1 = right.second;
                        return pb(ps1);
                    });
}

/*
 * fail parser
 * write a fault message to the parsing state
 * and makes it invalid
 */
template<class T>
parser_type<T> fail(const std::string &why)
{
    return function([&](parsing_state ps)
                    {
                        ps.fault(why, true);
                        return parser_out<T>(std::unique_ptr<T>(), ps);
                    });
}

/*
 * pure parser
 * leaves state unchanged
 * writes a value to the result of the parsing function
 */
template<class T>
parser_type<T> pure(const T &t)
{
    return function([&](parsing_state ps)
                    {
                        return parser_out<T>(std::make_unique<T>(t), ps);
                    });
}

//parse a given char
parser_type<char> p_char(const char &t);

//parse any char obeys given predicate
parser_type<char> p_char(const function<bool(char)> &predicate, const std::string &description);

//parse zero or more chars satisfied to the subparser and return std::string of them
parser_type<std::string> many(const parser_type<char> &subparser);

//parse one or more chars satisfied to the subparser and return std::string of them
parser_type<std::string> many1(const parser_type<char> &subparser);

//parse zero or more entries and return std::vector of them
template<class T>
parser_type<std::vector<T>> many(const parser_type<T> &subparser)
{
    return {[&](parsing_state ps)
            {
                std::vector<T> out;
                parser_out<T> c = subparser(ps);
                while (c.second.is_valid())
                {
                    out.push_back(*c.first);
                    ps = c.second;
                    c = subparser(ps);
                }
                return parser_out<std::vector<T>>(std::make_unique<std::vector<T>>(out), ps);
            }};
}


//parse given string of symbols, return the string on match, fails if not
parser_type<std::string> p_string(const std::string &s);

//parse characters until subparser not satisfied
template<class T>
parser_type<std::string> p_until(const parser_type<T> &subparser)
{
    return {[&](parsing_state ps)
            {
                std::string out;
                parser_out<T> test = subparser(ps);
                while (!test.second.is_valid() && !ps.is_EOF())
                {
                    out.push_back(++ps._ds);
                    test = subparser(ps);
                }
                return parser_out<std::string>(std::make_unique<std::string>(out), ps);
            }};
}

extern parser_type<char> any_char;

extern parser_type<bool> is_valid;

template<class T>
function<parser_type<T>(bool)> p_if(const parser_type<T> &trueval, const parser_type<T> &falseval)
{
    return function([&](bool b)
                    {
                        if (b) return trueval;
                        return falseval;
                    });
}

template<class T>
parser_type<T> operator||(const parser_type<T> &p1, const parser_type<T> &p2)
{
    return {[&](parsing_state ps)
            {
                parser_out<T> test1 = p1(ps);
                if (test1.second.is_valid()) return test1;
                parser_out<T> test2 = p2(ps);
                if (test2.second.is_valid()) return test2;
                std::stringstream ss;
                ss << test1.second << "\nor\n" << test2.second;
                ps.fault(ss.str(), false);
                return parser_out<T>(std::unique_ptr<T>(), ps);
            }};
}

#endif /***PARSING_H***/