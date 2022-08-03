#ifndef __SOF_STRING_H__
#define __SOF_STRING_H__

#include <string>
#include <stdint.h>

/*
String工具类
*/
namespace sof_string
{
    //
    // Members
    //

    //
    // Functions
    //

    std::string &itostr(int n, std::string &s);

    std::string &itostr(unsigned val, std::string &s);

    std::string &itostr(int64_t n, std::string &s);

    std::string &itostr(uint64_t val, std::string &s);

};

#endif