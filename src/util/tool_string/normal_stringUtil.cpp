#include "normal_stringUtil.h"

#ifdef _MSC_VER

#else

#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctime>
#include <math.h>
#include <stdint.h>
#include <iostream>
#include <map>

#include "sof_string.h"

using namespace std;

namespace n_stringUtil
{
    /*
    将<io_strResult>字符串的 <in_oldValue>子串 转换成<in_newValue>
    */
    void String_replace(const std::string &in_oldValue, const std::string &in_newValue, std::string &io_strResult)
    {
        for (std::string::size_type pos = io_strResult.find(in_oldValue);
             pos != std::string::npos;
             pos = io_strResult.find(in_oldValue, pos))
        {
            io_strResult.replace(pos, in_oldValue.size(), in_newValue);
            pos += in_newValue.size();
        }
    }

    /*
    安全的将String转换为Int
    Return:
    0 -- 转换成功
    非0 -- 转换失败
    */
    int String2Int_atoi(const string &in_strInt, int &out_int)
    {
        static const string ftag("String2Int_atoi()");

        if (in_strInt.empty() || 0 == in_strInt.length())
        {
            out_int = 0;
            return 0;
        }

        try
        {
            out_int = atoi(in_strInt.c_str());
            return 0;
        }
        catch (exception &e)
        {
            cout << ftag << e.what() << endl;
            return -1;
        }
    }

    /*
    安全的将String转换为Long
    Return:
    0 -- 转换成功
    非0 -- 转换失败
    */
    long String2Long_atol(const string &in_strInt, long &out_long)
    {
        static const string ftag("String2Long_atol()");

        if (in_strInt.empty() || 0 == in_strInt.length())
        {
            out_long = 0;
            return 0;
        }

        try
        {
            out_long = atol(in_strInt.c_str());
        }
        catch (exception &e)
        {
            std::string sDebug("[");
            sDebug += in_strInt;
            sDebug += "]";
            cout << ftag << sDebug << e.what() << endl;
            return -1;
        }

        return 0;
    }

    /*
    安全的将String(整数，有可能带小数)转换为INT64的整数(不含小数点)
    Return :
    0 -- 转换成功
    非0 -- 转换失败
    */
    int String2Int64_atoi64(const string &in_strInt, int64_t &out_i64Int)
    {
        static const string ftag("String2Int64_atoi64()");

        if (in_strInt.empty())
        {
            out_i64Int = 0;
            return 0;
        }

        try
        {
            out_i64Int = strtoll(in_strInt.c_str(), NULL, 10);
        }
        catch (exception &e)
        {
            cout << ftag << e.what() << endl;
            return -1;
        }
        return 0;
    }

    /*
    安全的将String转换为Double
    Return :
    0 -- 转换成功
    非0 -- 转换失败
    */
    int String2Double_atof(const string &in_strInt, double &out_double)
    {
        static const string ftag("String2Double_atof()");

        if (in_strInt.empty())
        {
            out_double = 0;
            return 0;
        }

        try
        {
            out_double = atof(in_strInt.c_str());
            return 0;
        }
        catch (exception &e)
        {
            cout << ftag << e.what() << endl;
            return -1;
        }
    }

    // 循环替换字符串中旧字符串为新值
    string &replace_all(string &str, const string &old_value, const string &new_value)
    {
        while (true)
        {
            string::size_type pos(0);
            if (string::npos != (pos = str.find(old_value)))
            {
                str.replace(pos, old_value.length(), new_value);
            }
            else
            {
                break;
            }
        }

        return str;
    }

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
    int RawStringToJsonString(const std::string &in_strSource, std::string &out_strOutputRes)
    {
        static const string ftag("RawStringToJsonString()");

        if (in_strSource.empty())
        {
            return 0;
        }

        size_t iPos = 0;
        size_t iMaxPos = in_strSource.length() - 1;

        out_strOutputRes = in_strSource;

        try
        {
            char szTemp = '\0';
            const char cszInsert = '\\';

            do
            {
                // 循环从上次\定位的位置开始往后寻找‘\’
                iPos = out_strOutputRes.find('\\', iPos);

                if (out_strOutputRes.npos == iPos)
                {
                    break;
                }
                else if (0 == iPos || iPos >= iMaxPos)
                {
                    break;
                }
                else if (0 < iPos && iPos < iMaxPos) // 未找到或者查找完成
                {
                    // 匹配\字符下一个字符
                    szTemp = out_strOutputRes.at(iPos + 1);
                    switch (szTemp)
                    {
                        // 转义
                    case '\\':
                    case '\"':
                        // 需插入双斜杆
                        out_strOutputRes.insert(iPos, 2, cszInsert);
                        // 比如当前位置指示器指向\.1\.2X的\.1 ,转成\\\.1\.2X ,字符串\.2已被识别，指示器将移动到\.2的下一位 x，移动距离4;
                        iPos += 4;
                        iMaxPos += 2;
                        break;

                    case '/':
                    case 'b':
                    case 'f':
                    case 'n':
                    case 'r':
                    case 't':
                    default:
                        // 需插入单斜杆
                        out_strOutputRes.insert(iPos, 1, cszInsert);
                        // 比如当前位置指示器指向\bx的\ ,转成\\bx ,b已被识别，指示器将移动到b的下一位 x，移动距离3
                        iPos += 3;
                        ++iMaxPos;
                        break;
                    }
                } // end if
            } while (iPos < iMaxPos);
        }
        catch (std::exception &e)
        {
            cout << ftag << e.what() << endl;
            return -1;
        }

        return 0;
    }

    /*
    获取map的内容

    @return 0 : 转换完毕
    @return -1 : 解析异常
    */
    int GetString(bool bDebug, const std::map<uint64_t, int> &in_maps, std::string &out_strOutputMag)
    {
        string strItoa;
        out_strOutputMag += "map[";
        std::map<uint64_t, int>::const_iterator it = in_maps.begin();
        for (; in_maps.end() != it; ++it)
        {
            sof_string::itostr(it->first, strItoa);
            out_strOutputMag += strItoa;
            if (bDebug)
            {
                out_strOutputMag += ", ";
                sof_string::itostr(it->second, strItoa);
                out_strOutputMag += strItoa;
            }
            out_strOutputMag += "; ";
        }
        out_strOutputMag += "]";

        return 0;
    }

    /*
    获取vector的内容

    @return 0 : 转换完毕
    @return -1 : 解析异常
    */
    int GetString(const std::vector<std::string> &in_vcts, std::string &out_strOutputMag)
    {
        out_strOutputMag += "vector[";
        std::vector<std::string>::const_iterator it = in_vcts.begin();
        for (; in_vcts.end() != it; ++it)
        {
            out_strOutputMag += *it;
            out_strOutputMag += " ";
        }
        out_strOutputMag += "]";

        return 0;
    }

    /*
    按最大长度截取字符串，如不超过最大长度返回原字符串，如超过则截断尾部

    @return 0 : 转换完毕
    @return -1 : 解析异常
    */
    int CutByMaxLen(const std::string &src, std::string &out_dest, unsigned int uiMaxLen)
    {
        size_t iLenSrc = src.length();
        size_t iRequire = uiMaxLen;

        if (0 == iRequire)
        {
            out_dest = "";
            return 0;
        }

        if (iLenSrc <= iRequire)
        {
            out_dest = src;
            return 0;
        }

        out_dest = src.substr(0, iRequire);
        return 0;
    }

}