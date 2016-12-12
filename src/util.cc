#include "util.h"
#include <stdarg.h>
#include <stdio.h>

namespace util
{

void strSplit(const std::string & src, const std::string & sep,
              std::vector<std::string>& tokens )
{
    tokens.clear();
    std::string s;
    for(std::string::const_iterator i = src.begin(); i != src.end(); i++)
    {
        if (sep.find(*i) != std::string::npos)
        {
            tokens.push_back(s);
            s = "";
        }
        else
        {
            s += *i;
        }
    }
    if ( s.length() || tokens.size() ) tokens.push_back(s);
}

std::string string_format( const std::string fmt, ... )
{
    int size = (static_cast<int>(fmt.size())) * 2 + 50; /* Reserve two times as much as the length of the fmt_str */
    std::string str;
    va_list ap;
    while ( true ) {     // Maximum two passes on a POSIX system...
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf(const_cast<char*>(str.data()), size, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size) {  // Everything worked
            str.resize(n);
            return str;
        }
        if (n > -1)  // Needed size returned
            size = n + 1;   // For null char
        else
            size *= 2;      // Guess at a larger size (OS specific)
    }
    return str;
}

}