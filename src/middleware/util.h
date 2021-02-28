/*!

\author         Oliver Blaser
\date           28.02.2021
\copyright      GNU GPLv3 - Copyright (c) 2021 Oliver Blaser

*/

#ifndef _UTIL_H_
#define _UTIL_H_

#include <filesystem>
#include <iostream>
#include <string>

#include "project.h"



#ifndef __FILENAME__
#define __FILENAME__     (__FILE_TO_FILENAME__(__FILE__))
constexpr const char* __FILE_TO_FILENAME__(const char* p)
{
    const char* fn = p;
    while (*p)
    {
#if(PRJ_PLAT_WIN)
        if (*p++ == '\\') fn = p;
#else
        if (*p++ == '/') fn = p;
#endif
    }
    return fn;
}
#endif



struct Result
{
    Result();
    Result(int ne);
    Result(int ne, int nw);

    int err;
    int warn;

    Result operator+(const Result& summand);
    Result& operator+=(const Result& summand);
    friend bool operator>(const Result& left, int right);
    friend bool operator==(const Result& left, int right);
    friend std::ostream& operator<<(std::ostream& os, const Result& v);
};

Result changeWD(const std::string& jobfile);

std::string fsExceptionPath(const std::filesystem::filesystem_error& ex);

void printEWI(const std::string& file, const std::string& text, size_t line = 0, size_t col = 0, int ewi = 0x7FFFFFFF, int style = 0x7FFFFFFF);

#endif // _UTIL_H_
