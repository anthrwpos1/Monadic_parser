#ifndef M_GENERAL_H
#define M_GENERAL_H

#include <string>
#include "Parsing.h"
#include "fstream"

namespace M_General
{
    void filemap(const std::string &infile, const std::string &outfile, const parser_type<std::string> &parser)
    {
        std::fstream instream(infile, std::fstream::in);
        if (!instream) return;
        std::fstream  outstream(outfile, std::fstream::out);
        if (!outstream) return;
        instream.seekg(0, std::istream::end);
        size_t length = instream.tellg();
        instream.seekg(0, std::istream::beg);
        std::unique_ptr<char[]> filedump(new char[length + 1]);
        instream.read(filedump.get(),length);
        filedump[length] = 0;
        auto out = run_parser(parser, filedump.get());
        std::string outstring;
        if (out) outstring = *out.get();
        else outstring = out.getMessage();
        outstream.write(outstring.c_str(), outstring.length());
    }
}

#endif /***M_GENERAL_H***/