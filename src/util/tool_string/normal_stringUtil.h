#ifndef __NORMAL_STRING_UTIL_H__
#define __NORMAL_STRING_UTIL_H__

#ifdef _MSC_VER
#include <Windows.h>
#else

#endif

#include <time.h>
#include <mutex>
#include <stdint.h>
#include <vector>
#include <map>

/*
自用String工具类
*/
namespace n_stringUtil
{
    //
    // Functions
    //

    /*
    将<io_strResult>字符串的 <in_oldValue>子串 转换成<in_newValue>
    */
    void String_replace(const std::string &in_oldValue, const std::string &in_newValue, std::string &io_strResult);

    /*
    安全的将String转换为Int
    Return:
    0 -- 转换成功
    非0 -- 转换失败
    */
    int String2Int_atoi(const std::string &in_strInt, int &out_int);

    /*
    安全的将String转换为Long
    Return:
    0 -- 转换成功
    非0 -- 转换失败
    */
    long String2Long_atol(const std::string &in_strInt, long &out_long);

    /*
    安全的将String(正数，有可能带小数)转换为uint64_t的正整数(不含小数点)
    Return :
    0 -- 转换成功
    非0 -- 转换失败
    */
    int String2UInt64_strtoui64(const std::string &in_strInt, uint64_t &out_ui64Int);

    /*
    安全的将String(整数，有可能带小数)转换为INT64的整数(不含小数点)
    Return :
    0 -- 转换成功
    非0 -- 转换失败
    */
    int String2Int64_atoi64(const std::string &in_strInt, int64_t &out_i64Int);

    /*
    安全的将String转换为Double
    Return :
    0 -- 转换成功
    非0 -- 转换失败
    */
    int String2Double_atof(const std::string &in_strInt, double &out_double);

    // 循环替换字符串中旧字符串为新值
    std::string &replace_all(std::string &str, const std::string &old_value, const std::string &new_value);

    /*
    将浮点字值转为字符串值
    Params :

    const double &in_dValue :
    double值

    string& out_strValue :
    字符串.

    unsigned int uiDecimalLen :
    小数点后数字长度.

    Return :
    0 -- 转换成功
    非0 -- 转换失败
    */
    int DoubleToString(const double &in_dValue, std::string &out_strValue, unsigned int uiDecimalLen);

    /*
    从原始字符串中识别的转移符号，转换成供json解析正则表达式的字符串

    在web端用户填写正则表达式如 {2}\b,存储在字符串中"{2}\b"传给撮合，撮合无法解析该字符串。
    要达到正确解析效果，需转换为"{2}\\b"，json读取的才是"{2}\b"

    转换关系表：
    正则表达式   |    C/C++
    字符串
    \\		  | 	\\\\
    \/		  |  	\\/
    \b		  |		\\b
    \f		  | 	\\f
    \n		  | 	\\n
    \r		  | 	\\n
    \t		  | 	\\t
    \'		  |		\\'
    \"		  |		\\'"

    @param const std::string& in_strSource : 从web端获取的原始字符串strSource
    @param std::string& out_strOutputRes : 转义符保证后的字符串

    @return 0 : 转换完毕
    @return -1 : 解析异常
    */
    int RawStringToJsonString(const std::string &in_strSource, std::string &out_strOutputRes);

    /*
    获取map的内容

    @return 0 : 转换完毕
    @return -1 : 解析异常
    */
    int GetString(bool bDebug, const std::map<uint64_t, int> &in_maps, std::string &out_strOutputMag);

    /*
    获取vector的内容

    @return 0 : 转换完毕
    @return -1 : 解析异常
    */
    int GetString(const std::vector<std::string> &in_vcts, std::string &out_strOutputMag);

    /*
    按最大长度截取字符串，如不超过最大长度返回原字符串，如超过则截断尾部

    @return 0 : 转换完毕
    @return -1 : 解析异常
    */
    int CutByMaxLen(const std::string &src, std::string &out_dest, unsigned int uiMaxLen);

}

#endif