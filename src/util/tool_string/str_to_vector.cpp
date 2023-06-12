#include "str_to_vector.h"

using namespace std;

namespace n_stringUtil
{
    /*
    将 string 字符串转换成 vector<uint8_t>
    */
    shared_ptr<vector<uint8_t>> strToVctUi8(const string &src)
    {
        string::const_iterator cit = src.begin();

        shared_ptr<vector<uint8_t>> pDst = make_shared<vector<uint8_t>>();

        for (; cit != src.end(); ++cit)
        {
            pDst->push_back(*cit);
        }

        return pDst;
    }
};