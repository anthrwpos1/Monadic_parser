#include <iostream>
#include <iterator>
#include <memory>
//#include "M_General.h"
#include "M_XML.h"

using namespace std;

int main()
{

    XMLTag t{"tag1", "value1"};
    stringstream ss;
    ss << t;
    auto s = ss.str();
    auto prs = run_parser(function(p_XMLTag), s.c_str());
    if (prs) cout << *prs.get() << endl;
    else cout << prs.getMessage()<< endl;
    return 0;
}
