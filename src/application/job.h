/*!

\author         Oliver Blaser
\date           14.02.2021
\copyright      GNU GPLv3 - Copyright (c) 2021 Oliver Blaser

*/

#ifndef _JOB_H_
#define _JOB_H_

#include <string>
#include <vector>

#include "arg.h"

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
}

#endif // _JOB_H_
