/*!

\author         Oliver Blaser
\date           28.04.2021
\copyright      GNU GPLv3 - Copyright (c) 2021 Oliver Blaser

*/

#ifndef _JOB_H_
#define _JOB_H_

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "arg.h"
#include "middleware/util.h"

namespace potoroo
{
    //! @brief jobfile comment char
    const char jfcc = '#';

    const std::string tagCpp = "//#p";
    const std::string tagBash = "##p";
    const std::string tagBatch = "@rem #p";

    enum class JobMode
    {
        proc,
        copy,
        copyow
    };

    class Job
    {
    public:
        Job();
        Job(const std::string& inputFile, const std::string& outputFile, const std::string& tag,
            bool warningAsError = false,
            bool writeErrorLine = false, const std::string& writeErrorLineStr = std::string("--write-error-line"),
            JobMode mode = JobMode::proc,
            const std::string* wSup = nullptr);

        void setValidity(bool validity);
        void setErrorMsg(const std::string& msg);

        std::string getInputFile() const;
        std::string getOutputFile() const;
        std::string getTag() const;
        JobMode getMode() const;
        bool warningAsError() const;
        bool writeErrorLine() const;
        std::string writeErrorLineStr() const;
        const std::vector<int>& getWSupList() const;

        void setInputFile(const std::string& inputFile);
        void setOutputFile(const std::string& outputFile);
        void setTag(const std::string& t);
        void setMode(const JobMode& m);
        void setWarningAsError(bool warningAsError = true);
        void clrWarningAsError();
        void setWSupList(const int* list, size_t count);
        void setWSupList(const std::vector<int>& list);
        int setWSupList(const std::string& list);

        void wSupListAdd(int wID);
        void wSupListAddRange(const int* list, size_t count);
        void wSupListAddRange(const std::vector<int>& list);
        int wSupListAddRange(const std::string& list);
        std::string wSupListToString() const;

        bool isValid() const;
        std::string getErrorMsg() const;

        friend std::ostream& operator<<(std::ostream& os, const Job& j);

    private:
        std::string inFile;
        std::string outFile;
        std::string tag;
        JobMode mode;
        bool wError;
        bool wrErrLn;
        std::string wrErrLnStr;
        std::vector<int> wSupList;

        bool validity = false;
        std::string errorMsg;

    public:
        static Result parseFile(const std::string& filename, std::vector<Job>& jobs);
        static Job parseArgs(const ArgList& args);
    };
}

#endif // _JOB_H_
