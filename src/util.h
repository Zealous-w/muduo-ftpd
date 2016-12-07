#ifndef MUDUO_UTIL_H
#define MUDUO_UTIL_H

#include <memory>
#include <string>
#include <vector>

namespace util
{

void strSplit(const std::string & src, const std::string & sep, std::vector<std::string>& tokens );

std::string string_format( const std::string fmt_str, ... );

}

#endif //MUDUO_UTIL_H
