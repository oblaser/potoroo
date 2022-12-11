/*!

\author         Oliver Blaser
\date           07.04.2021
\copyright      GNU GPLv3 - Copyright (c) 2022 Oliver Blaser

*/

#ifndef _UTIL_H_
#define _UTIL_H_

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

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



enum class lineEnding
{
    error,

    LF,
    CR,
    CRLF
};



Result changeWD(const std::filesystem::path& newWD, std::string* exWhat = nullptr);

void printEWI(const std::string& file, const std::string& text, size_t line = 0, size_t col = 0, int ewi = 0x7FFFFFFF, int style = 0x7FFFFFFF);

lineEnding detectLineEnding(const std::filesystem::path& filepath);
int convertLineEnding(const std::filesystem::path& inf, const std::filesystem::path& outf, lineEnding outfLineEnding);
int convertLineEnding(const std::filesystem::path& inf, lineEnding infLineEnding, const std::filesystem::path& outf, lineEnding outfLineEnding);
int convertLineEnding(const std::filesystem::path& inf, lineEnding infLineEnding, const std::filesystem::path& outf, lineEnding outfLineEnding, std::string& errMsg);

size_t strReplaceAll(std::string& str, const std::string& from, const std::string& to);

bool vectorContains(const std::vector<int>& vec, int value);

#endif // _UTIL_H_
