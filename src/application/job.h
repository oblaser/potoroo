/*!

\author         Oliver Blaser
\date           17.02.2021
\copyright      GNU GPLv3 - Copyright (c) 2021 Oliver Blaser

*/

#ifndef _JOB_H_
#define _JOB_H_

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "arg.h"


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



namespace potoroo
{
    const char jfcc = '#'; // jobfile comment char

    const std::string tagCpp = "//#p";
    const std::string tagBash = "#//p";
    const std::string tagBatch = "@rem #p";

    struct Result
    {
        Result();
        Result(int ne);
        Result(int ne, int nw);

        int nErr;
        int nWarn;

        Result operator+(const Result& summand);
        Result& operator+=(const Result& summand);
        friend bool operator>(const Result& left, int right);
        friend bool operator==(const Result& left, int right);
        friend std::ostream& operator<<(std::ostream& os, const Result& v);
    };

    struct JobFileLine
    {
        JobFileLine();
        JobFileLine(size_t l, const std::string& d);

        //! @brief Display line number
        //! 0 is an invalid line number.
        size_t line;

        std::string data;
    };

    class Job
    {
    public:
        Job();
        Job(const std::string& inputFile, const std::string& outputFile, const std::string& tag, bool warningAsError = false);

        void setValidity(bool validity);
        void setErrorMsg(const std::string& msg);

        std::string getInputFile() const;
        std::string getOutputFile() const;
        std::string getTag() const;
        bool warningAsError() const;

        bool isValid() const;
        std::string getErrorMsg() const;

        friend std::ostream& operator<<(std::ostream& os, const Job& j);

    private:
        std::string inFile;
        std::string outFile;
        std::string tag;
        bool wError;

        bool validity = false;
        std::string errorMsg;

    public:
        static Result parseFile(const std::string& filename, std::vector<Job>& jobs);
        static Job parseArgs(const ArgList& args);
    };

    Result changeWD(const std::string& jobfile);
    std::string fsExceptionPath(const std::filesystem::filesystem_error& ex);
    void printEWI(const std::string& file, const std::string& text, size_t line = 0, size_t col = 0, int ewi = 0x7FFFFFFF, int style = 0x7FFFFFFF);
}

#endif // _JOB_H_
