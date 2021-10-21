#include "Parsing.h"

parser_type<char> p_char(const char& t)
{
    return {[&](parsing_state ps)
            {
                parsing_state preserve = ps;
                data_stream &ds = ps._ds;
                if (!ds) return fail<char>("no parsing data")(ps);
                if (ps.is_EOF()) return fail<char>("end of input")(ps);
                if (!ps.is_valid()) return fail<char>("invalid state")(ps);
                std::string msg = "expected ";
                msg.push_back(t);
                if (++ds != t) return fail<char>(msg)(preserve);
                return parser_out<char>(std::make_unique<char>(t), ps);
            }};
}

parser_type<char> p_char(const function<bool(char)>& predicate, const std::string& description)
{
    return {[&](parsing_state ps)
            {
                parsing_state preserve = ps;
                data_stream &ds = ps._ds;
                if (!ds) return fail<char>("no parsing data")(ps);
                if (ps.is_EOF()) return fail<char>("End Of Input")(ps);
                if (!ps.is_valid()) return fail<char>("invalid state")(ps);
                std::string msg = "expected " + description;
                char c = ++ds;
                if (!predicate(c)) return fail<char>(msg)(preserve);
                return parser_out<char>(std::make_unique<char>(c), ps);
            }};
}

parser_type<std::string> many(const parser_type<char>& subparser)
{
    return {[&](parsing_state ps)
            {
                std::string out;
                parser_out<char> c = subparser(ps);
                while (c.second.is_valid())
                {
                    out.push_back(*c.first);
                    ps = c.second;
                    c = subparser(ps);
                }
                return parser_out<std::string>(std::make_unique<std::string>(out), ps);
            }};
}

parser_type<std::string> many1(const parser_type<char>& subparser)
{
    return {[&](parsing_state ps)
            {
                std::string out;
                parser_out<char> c = subparser(ps);
                if (!c.second.is_valid())
                {
                    ps.fault(c.second._pfd._what, false);
                    return parser_out<std::string>(std::unique_ptr<std::string>(), ps);
                }
                while (c.second.is_valid())
                {
                    out.push_back(*c.first);
                    ps = c.second;
                    c = subparser(ps);
                }
                return parser_out<std::string>(std::make_unique<std::string>(out), ps);
            }};
}

parser_type<std::string> p_string(const std::string& s)
{
    return {[&](parsing_state ps)
            {
                std::stringstream ss;
                for (char c: s)
                {
                    parser_out<char> try_c = p_char(c)(ps);
                    if (!try_c.second.is_valid())
                    {
                        ss << "expected " << c << " in string " << s;
                        return fail<std::string>(ss.str())(ps);
                    }
                    ps = try_c.second;
                }
                return parser_out<std::string>(std::make_unique<std::string>(s), ps);
            }};
}

//parse any char. fails on End-Of-Input
parser_type<char> any_char{[](parsing_state ps)
                           {
                               data_stream &ds = ps._ds;
                               if (!ds) return fail<char>("no parsing data")(ps);
                               if (ps.is_EOF()) return fail<char>("End Of Input")(ps);
                               if (!ps.is_valid()) return fail<char>("invalid state")(ps);
                               char c = ++ds;
                               return parser_out<char>(std::make_unique<char>(c), ps);
                           }};


//test if parsing state is valid, also restores it to be valid if it not.
parser_type<bool> is_valid{[](parsing_state ps)
                           {
                               ps._pfd._valid = true;
                               ps._pfd._what = "";
                               ps._pfd._fault_point = data_stream();
                               return parser_out<bool>(std::make_unique<bool>(ps.is_valid()), ps);
                           }};