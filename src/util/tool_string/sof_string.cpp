#include "sof_string.h"

/*
此转换工具是参考StackOverFlow

C++ performance challenge: integer to std::string conversion
http://stackoverflow.com/questions/4351371/c-performance-challenge-integer-to-stdstring-conversion
*/

namespace sof_string
{
    // 两位数的预制字符串
    static const char digit_pairs[201] = {
        "00010203040506070809"
        "10111213141516171819"
        "20212223242526272829"
        "30313233343536373839"
        "40414243444546474849"
        "50515253545556575859"
        "60616263646566676869"
        "70717273747576777879"
        "80818283848586878889"
        "90919293949596979899"};

    std::string &itostr(int n, std::string &s)
    {
        if (n == 0)
        {
            s = "0";
            return s;
        }

        int sign = -(n < 0);
        unsigned int val = (n ^ sign) - sign;

        int size;
        if (val >= 10000)
        {
            if (val >= 10000000)
            {
                if (val >= 1000000000)
                {
                    size = 10;
                }
                else if (val >= 100000000)
                {
                    size = 9;
                }
                else
                {
                    size = 8;
                }
            }
            else
            {
                if (val >= 1000000)
                {
                    size = 7;
                }
                else if (val >= 100000)
                {
                    size = 6;
                }
                else
                {
                    size = 5;
                }
            }
        }
        else
        {
            if (val >= 100)
            {
                if (val >= 1000)
                {
                    size = 4;
                }
                else
                {
                    size = 3;
                }
            }
            else
            {
                if (val >= 10)
                {
                    size = 2;
                }
                else
                {
                    size = 1;
                }
            }
        }
        size -= sign;
        s.resize(size);
        char *c = &s[0];
        if (sign)
        {
            *c = '-';
        }

        c += size - 1;
        while (val >= 100)
        {
            int pos = val % 100;
            val /= 100;
            *(short *)(c - 1) = *(short *)(digit_pairs + 2 * pos);
            c -= 2;
        }
        while (val > 0)
        {
            *c-- = '0' + (val % 10);
            val /= 10;
        }
        return s;
    }

    std::string &itostr(unsigned val, std::string &s)
    {
        if (val == 0)
        {
            s = "0";
            return s;
        }

        int size;
        if (val >= 10000)
        {
            if (val >= 10000000)
            {
                if (val >= 1000000000)
                {
                    size = 10;
                }
                else if (val >= 100000000)
                {
                    size = 9;
                }
                else
                {
                    size = 8;
                }
            }
            else
            {
                if (val >= 1000000)
                {
                    size = 7;
                }
                else if (val >= 100000)
                {
                    size = 6;
                }
                else
                {
                    size = 5;
                }
            }
        }
        else
        {
            if (val >= 100)
            {
                if (val >= 1000)
                {
                    size = 4;
                }
                else
                {
                    size = 3;
                }
            }
            else
            {
                if (val >= 10)
                {
                    size = 2;
                }
                else
                {
                    size = 1;
                }
            }
        }

        s.resize(size);
        char *c = &s[size - 1];
        while (val >= 100)
        {
            int pos = val % 100;
            val /= 100;
            *(short *)(c - 1) = *(short *)(digit_pairs + 2 * pos);
            c -= 2;
        }
        while (val > 0)
        {
            *c-- = '0' + (val % 10);
            val /= 10;
        }
        return s;
    }

    /*
    (std::numeric_limits<INT64>::max)() --  9223372036854775807  9223,3720,3685,4775,807 19位
    (std::numeric_limits<INT64>::min)() -- -9223372036854775808
    */
    std::string &itostr(int64_t n, std::string &s)
    {
        if (0 == n)
        {
            s = "0";
            return s;
        }

        int sign = -(n < 0);
        uint64_t val = (n ^ sign) - sign;

        int size;

        // 20~11位
        if (10000000000 <= val)
        {
            // 20~15位
            if (100000000000000 <= val)
            {
                if (100000000000000000 <= val)
                {
                    if (10000000000000000000ULL <= val)
                    {
                        size = 20;
                    }
                    else if (1000000000000000000 <= val)
                    {
                        size = 19;
                    }
                    else
                    {
                        size = 18;
                    }
                }
                else
                {
                    if (10000000000000000 <= val)
                    {
                        size = 17;
                    }
                    else if (1000000000000000 <= val)
                    {
                        size = 16;
                    }
                    else
                    {
                        size = 15;
                    }
                }
            }
            else // 14~11位
            {
                if (1000000000000 <= val)
                {
                    if (10000000000000 <= val)
                    {
                        size = 14;
                    }
                    else
                    {
                        size = 13;
                    }
                }
                else
                {
                    if (100000000000 <= val)
                    {
                        size = 12;
                    }
                    else
                    {
                        size = 11;
                    }
                }
            }
        }
        else if (val >= 10000) // 10~5位
        {
            if (val >= 10000000)
            {
                if (val >= 1000000000)
                {
                    size = 10;
                }
                else if (val >= 100000000)
                {
                    size = 9;
                }
                else
                {
                    size = 8;
                }
            }
            else
            {
                if (val >= 1000000)
                {
                    size = 7;
                }
                else if (val >= 100000)
                {
                    size = 6;
                }
                else
                {
                    size = 5;
                }
            }
        }
        else // 4~1位
        {
            if (val >= 100)
            {
                if (val >= 1000)
                {
                    size = 4;
                }
                else
                {
                    size = 3;
                }
            }
            else
            {
                if (val >= 10)
                {
                    size = 2;
                }
                else
                {
                    size = 1;
                }
            }
        }

        size -= sign;
        s.resize(size);
        char *c = &s[0];
        if (sign)
        {
            *c = '-';
        }

        c += size - 1;
        while (val >= 100)
        {
            int pos = val % 100;
            val /= 100;
            *(short *)(c - 1) = *(short *)(digit_pairs + 2 * pos);
            c -= 2;
        }
        while (val > 0)
        {
            *c-- = '0' + (val % 10);
            val /= 10;
        }
        return s;
    }

    /*
    (std::numeric_limits<uint64_t>::max)() --  18446744073709551615 1844,6744,0737,0955,1615 20位
    (std::numeric_limits<uint64_t>::min)() --  0
    */
    std::string &itostr(uint64_t val, std::string &s)
    {
        if (0 == val)
        {
            s = "0";
            return s;
        }

        int size;

        // 20~11位
        if (10000000000 <= val)
        {
            // 20~15位
            if (100000000000000 <= val)
            {
                if (100000000000000000 <= val)
                {
                    if (10000000000000000000ULL <= val)
                    {
                        size = 20;
                    }
                    else if (1000000000000000000 <= val)
                    {
                        size = 19;
                    }
                    else
                    {
                        size = 18;
                    }
                }
                else
                {
                    if (10000000000000000 <= val)
                    {
                        size = 17;
                    }
                    else if (1000000000000000 <= val)
                    {
                        size = 16;
                    }
                    else
                    {
                        size = 15;
                    }
                }
            }
            else // 14~11位
            {
                if ((uint64_t)1000000000000 <= val)
                {
                    if ((uint64_t)10000000000000 <= val)
                    {
                        size = 14;
                    }
                    else
                    {
                        size = 13;
                    }
                }
                else
                {
                    if ((uint64_t)100000000000 <= val)
                    {
                        size = 12;
                    }
                    else
                    {
                        size = 11;
                    }
                }
            }
        }
        else if (val >= 10000) // 10~5位
        {
            if (val >= 10000000)
            {
                if (val >= 1000000000)
                {
                    size = 10;
                }
                else if (val >= 100000000)
                {
                    size = 9;
                }
                else
                {
                    size = 8;
                }
            }
            else
            {
                if (val >= 1000000)
                {
                    size = 7;
                }
                else if (val >= 100000)
                {
                    size = 6;
                }
                else
                {
                    size = 5;
                }
            }
        }
        else // 4~1位
        {
            if (val >= 100)
            {
                if (val >= 1000)
                {
                    size = 4;
                }
                else
                {
                    size = 3;
                }
            }
            else
            {
                if (val >= 10)
                {
                    size = 2;
                }
                else
                {
                    size = 1;
                }
            }
        }

        s.resize(size);
        char *c = &s[size - 1];
        while (val >= 100)
        {
            int pos = val % 100;
            val /= 100;
            *(short *)(c - 1) = *(short *)(digit_pairs + 2 * pos);
            c -= 2;
        }
        while (val > 0)
        {
            *c-- = '0' + (val % 10);
            val /= 10;
        }
        return s;
    }
}