#include "M_XML.h"

std::ostream& operator<<(std::ostream& o, const XMLTag& tag)
{
    o << tag.name << "=\"" << tag.value << "\"";
    return o;
}

std::ostream& operator<<(std::ostream& o, const XML& s)
{
    s.listtagsshow(o << s.spaces << "<" << s.blockname, s.tags);
    if (s.subblocks.empty()) o << " />\n";
    else
    {
        o << ">" << std::endl;
        for (const auto& subblock: s.subblocks)
        {
            o << subblock;
        }
        o << s.spaces << "</" << s.blockname << ">" << std::endl;
    }
    return o;
}

std::ostream& XML::listtagsshow(std::ostream& o, const std::vector<XMLTag>& tags) const
{
    for (const auto& tag: tags)
    {
        o << " " << tag;
    }
    return o;
}

parser_out<char> p_space_or_tab(parsing_state ps)
{
    return (p_char(' ') || p_char('\t'))(ps);
}

parser_out<char> p_alphanum(parsing_state ps)
{
    return (p_char([](char c) { return isalnum(c); }, "letter or digit"))(ps);
}

parser_out<char> p_specsymbol(parsing_state ps)
{
    return (p_string("&apos") >> pure('"'))(ps);
}

std::string unspecsymbol(const std::string& in)
{
    std::string out;
    for (const auto c: in)
    {
        if (c == '"') out += "&apos";
        else out.push_back(c);
    }
    return out;
}

parser_out<std::string> p_whiteSpaces(parsing_state ps)
{
    return many(function(p_space_or_tab))(ps);
}

parser_out<XMLTag> p_XMLTag(parsing_state ps)
{
    parser_type<std::string> tagName = many1(function(p_alphanum));
    parser_type<std::string> tagValue =
            function(p_whiteSpaces) >> p_char('=') >> function(p_whiteSpaces) >> p_char('"') >> p_until(p_char('"'));
    auto out = function(
            [](std::string name, std::string value, std::string spacesafter) { return XMLTag {name, value}; });
    return (out/tagName*tagValue*function(p_whiteSpaces))(ps);
}