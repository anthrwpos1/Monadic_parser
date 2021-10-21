#ifndef M_XML_H
#define M_XML_H

#include <iostream>
#include <vector>
#include "Parsing.h"

struct XMLTag
{
    std::string name, value;

    friend std::ostream &operator<<(std::ostream &o, const XMLTag &tag);
};

struct XML
{
    std::string spaces, blockname;
    std::vector<XMLTag> tags;
    std::vector<XML> subblocks;

    friend std::ostream &operator<<(std::ostream &o, const XML &s);

    std::ostream &listtagsshow(std::ostream &o, const std::vector<XMLTag> &tags) const;
};

parser_out<char> p_space_or_tab(parsing_state ps);

parser_out<char> p_alphanum(parsing_state ps);

parser_out<char> p_specsymbol(parsing_state ps);

std::string unspecsymbol(const std::string &in);

parser_out<std::string> p_whiteSpaces(parsing_state ps);

parser_out<XMLTag> p_XMLTag (parsing_state ps);

#endif /***M_XML_H***/