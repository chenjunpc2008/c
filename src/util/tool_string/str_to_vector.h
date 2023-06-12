#ifndef __STRING_TO_VERCTOR_H__
#define __STRING_TO_VERCTOR_H__

#include <string>
#include <stdint.h>
#include <vector>
#include <memory>

namespace n_stringUtil
{
    /*
   将 string 字符串转换成 vector<uint8_t>
   */
    std::shared_ptr<std::vector<uint8_t>> strToVctUi8(const std::string &src);
};

#endif