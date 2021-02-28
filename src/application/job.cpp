/*!

\author         Oliver Blaser
\date           28.02.2021
\copyright      GNU GPLv3 - Copyright (c) 2021 Oliver Blaser

*/

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "job.h"
#include "middleware/cliTextFormat.h"

namespace fs = std::filesystem;

using namespace std;
using namespace cli;
using namespace potoroo;

namespace
{
    struct JobFileLine
    {
        JobFileLine() : line(0), data("") {}
        JobFileLine(size_t l, const std::string& d) : line(l), data(d) {}

        //! @brief Display line number
        //! 0 is an invalid line number.
        size_t line;

        std::string data;
    };




    void printError(const std::string& file, const std::string& text, size_t line = 0, size_t col = 0)
    {
        printEWI(file, text, line, col, 0, 0);
    }

    void printWarning(const std::string& file, const std::string& text, size_t line = 0, size_t col = 0)
    {
        printEWI(file, text, line, col, 1, 0);
    }

    bool tagCondCpp(const string& ext)
    {
        return (
            (ext == ".php") ||
            (ext == ".js") ||
            (ext == ".c") ||
            (ext == ".cpp") ||
            (ext == ".h") ||
            (ext == ".hpp")
            );
    }

    bool tagCondBash(const string& ext)
    {
        return (
            (ext == ".sh")
            );
    }

    bool tagCondBatch(const string& ext)
    {
        return (
            (ext == ".bat") ||
            (ext == ".cmd")
            );
    }

    Job invalidInFilenameJob(const string& filename, const string& moreInfo)
    {
        Job j;

        string msg = "invalid input filename: \"" + filename + "\"";

        if (moreInfo.length() > 0) msg += " - " + moreInfo;

        j.setValidity(false);
        j.setErrorMsg(msg);

        return j;
    }

    void jobFileExtractLines(const char* const data, const char* const end, vector<JobFileLine>& lines)
    {
        size_t line = 1;
        const char* p = data;

        // end-1 to make clean CRLR checks, needs the file to be terminated
        // with unimportent data (such as LF, comments or whitespace)
        while (p < (end - 1))
        {
            // skip whitespace
            while (((*p == 0x09) || (*p == 0x20)) && (p < (end - 1))) ++p;

            // comment
            if (*p == jfcc)
            {
                while (
                    (*p != 0x0A) &&
                    ((*p != 0x0D) || (*(p + 1) != 0x0A)) &&
                    (p < (end - 1))
                    )
                {
                    ++p;
                }
            }

            // LF
            else if (*p == 0x0A)
            {
                ++p;
                ++line;
            }

            // CRLF
            else if ((*p == 0x0D) && (*(p + 1) == 0x0A))
            {
                p += 2;
                ++line;
            }

            // data
            else
            {
                JobFileLine argl;
                argl.data = "";
                argl.line = line;

                while (
                    (*p != 0x0A) &&
                    ((*p != 0x0D) || (*(p + 1) != 0x0A)) &&
                    (p < (end - 1))
                    )
                {
                    argl.data += *p;
                    ++p;
                }

                lines.push_back(argl);
            }
        }
    }

    //! @brief Reads the jobfile and prepares lines for parsing
    //! @param filename 
    //! @param [out] lines 
    //! @return 0 on success
    Result readJobFile(const std::string& filename, vector<JobFileLine>& lines)
    {
        const string procStr = "read jobfile";
        Result result;
        char* fileBuff = nullptr;

        try
        {
            ifstream jfs;

            int fileSize = 0;

            fs::path jobfile(filename);

            if (!fs::exists(jobfile)) throw runtime_error("not found");

            jfs.exceptions(ios::failbit | ios::badbit | ios::eofbit);
            jfs.open(jobfile, ios::binary | ios::in);

            jfs.seekg(0, ios::end);
            fileSize = jfs.tellg();
            jfs.seekg(0, ios::beg);

            if (fileSize == 0)
            {
                printWarning(procStr, "empty file");
                return Result(0, 1);
            }

            // to make sure that UTF BOM checks work
            // biggest UTF BOM is 4 bytes
            // length of minimal parameter line is greater than 8 bytes
            if (fileSize < 8)
            {
                throw runtime_error("invalid file");
            }






            //
            // new
            //

            fileBuff = new char[fileSize];
            jfs.read(fileBuff, fileSize);
            try { jfs.close(); }
            catch (exception& ex) { printWarning(procStr, "closing file failed: " + string(ex.what())); ++result.warn; }
            catch (...) { printWarning(procStr, "closing file failed"); ++result.warn; }

            // UTF BOM check
            if (fileBuff[0] == static_cast<char>(0x00) && fileBuff[1] == static_cast<char>(0x00) && fileBuff[2] == static_cast<char>(0xFe) && fileBuff[3] == static_cast<char>(0xFF)) throw runtime_error("encoding not supported: UTF-32 BE");
            if (fileBuff[0] == static_cast<char>(0xFF) && fileBuff[1] == static_cast<char>(0xFe) && fileBuff[2] == static_cast<char>(0x00) && fileBuff[3] == static_cast<char>(0x00)) throw runtime_error("encoding not supported: UTF-32 LE");
            if (fileBuff[0] == static_cast<char>(0xFe) && fileBuff[1] == static_cast<char>(0xFF)) throw runtime_error("encoding not supported: UTF-16 BE");
            if (fileBuff[0] == static_cast<char>(0xFF) && fileBuff[1] == static_cast<char>(0xFe)) throw runtime_error("encoding not supported: UTF-16 LE");

            const char* tmpFB = fileBuff;
            if (fileBuff[0] == static_cast<char>(0xeF) && fileBuff[1] == static_cast<char>(0xBB) && fileBuff[2] == static_cast<char>(0xBF)) tmpFB += 3; // skip UTF-8 BOM

            if (fileBuff[fileSize - 1] != 0x0A)
            {
                printWarning(procStr, "file does not end with a new line (may cause jobfile parse errors)");
                ++result.warn;
            }

            jobFileExtractLines(tmpFB, fileBuff + fileSize, lines);
        }
        catch (ios::failure& ex)
        {
            printError(procStr + ": IO", ex.what());
            result = -1;
        }
        catch (exception& ex)
        {
            printError(procStr, ex.what());
            cout << endl;
            result = -1;
        }
        catch (...)
        {
            printError(procStr, "unknown");
            result = -1;
        }

        delete[] fileBuff;

        return result;
    }
}




potoroo::Job::Job()
    : wError(false), validity(false), errorMsg("unset")
{
}

potoroo::Job::Job(const std::string& inputFile, const std::string& outputFile, const std::string& tag, bool warningAsError)
    : tag(tag), wError(warningAsError), validity(true)
{
    try { inFile = fs::path(inputFile).lexically_normal().string(); }
    catch (...) { inFile = string(inputFile); }

    try { outFile = fs::path(outputFile).lexically_normal().string(); }
    catch (...) { outFile = string(outputFile); }

    errorMsg.clear();
    errorMsg.shrink_to_fit();
}

void potoroo::Job::setValidity(bool validity)
{
    this->validity = validity;
}

void potoroo::Job::setErrorMsg(const std::string& msg)
{
    errorMsg = msg;
}

std::string potoroo::Job::getInputFile() const
{
    return inFile;
}

std::string potoroo::Job::getOutputFile() const
{
    return outFile;
}

std::string potoroo::Job::getTag() const
{
    return tag;
}

bool potoroo::Job::warningAsError() const
{
    return wError;
}

bool potoroo::Job::isValid() const
{
    return validity;
}

std::string potoroo::Job::getErrorMsg() const
{
    return errorMsg;
}

std::ostream& potoroo::operator<<(std::ostream& os, const Job& j)
{
    os << "\"" << j.getInputFile() << "\" \"" << j.getOutputFile() << "\" ";
    os << "\"" << j.getTag() << "\"";
    os << (j.warningAsError() ? " Werror" : "");
    return os;
}

//! @brief Parses a given jobfile
//! @param filename 
//! @param jobs 
//! @return 
//! 
//! return codes of potoroo::Result::err:
//! - <0 on file IO error
//! - 0 on success
//! - >0 number of parse errors
//! 
Result potoroo::Job::parseFile(const std::string& filename, std::vector<Job>& jobs)
{
    vector<JobFileLine> line;

#if PRJ_DEBUG & 0
    /*char* line[] =
    {
        "-if \"./a dir/asdf.ext\"  \t  -od ../../000\t-Werror",
        "-if \"./a dir/asdf.ext\"  \t  -of ./ptro\t-Werror -tag cpp        ",
        "-if \"./a dir/asdf.ext\" -of ./ptro -Werror -tag cpp",
        "    -if \"./a dir/asdf.ext\"  \t  -od ../../000\t-Werror -tag custom:\"*p",
        "    -if \"./a dir/asdf.ext\" -od ../../000 -Werror -tag custom:\"*p",
        "-if index.js -od ./deploy",
        "-if index.js -od ./deploy -of a",
        "-if ./sdf/ -od . -tag batch"
    };*/

    Result r;
    size_t ___dbg_l = 0;

    line.push_back(JobFileLine(++___dbg_l, "-if \"./a dir/asdf.ext\"  \t  -od ../../000\t-Werror"));
    line.push_back(JobFileLine(++___dbg_l, "-if \"./a dir/asdf.ext\"  \t  -of ./ptro\t-Werror -tag cpp        "));
    line.push_back(JobFileLine(++___dbg_l, "-if \"./a dir/asdf.ext\" -of ./ptro -Werror -tag cpp"));
    line.push_back(JobFileLine(++___dbg_l, "    -if \"./a dir/asdf.ext\"  \t  -od ../../000\t-Werror -tag custom:\"*p"));
    line.push_back(JobFileLine(++___dbg_l, "    -if \"./a dir/asdf.ext\" -od ../../000 -Werror -tag custom:\"*p"));
    line.push_back(JobFileLine(++___dbg_l, "-if index.js -od ./deploy"));
    line.push_back(JobFileLine(++___dbg_l, "-if index.js -od ./deploy -of a"));
    line.push_back(JobFileLine(++___dbg_l, "-if ./sdf/ -od . -tag batc"));
    line.push_back(JobFileLine(++___dbg_l, "-if ./sdf/ -od . -tag batch"));
#else
    Result r = readJobFile(filename, line);
    if (r.err != 0) return -1;
#endif



    for (size_t i = 0; i < line.size(); ++i)
    {
        ArgList args = ArgList::parse(line[i].data.c_str());

        string aprErrMsg = "";
        argProcResult apr = argProcJF(args, aprErrMsg);
        Job job = Job::parseArgs(args);

        if (apr != argProcResult::process)
        {
            ++r.err;
            printError("jobfile", aprErrMsg, line[i].line);
        }
        else if (!job.isValid())
        {
            ++r.err;
            printError("jobfile", job.getErrorMsg(), line[i].line);
        }
#if PRJ_DEBUG && 0
        jobs.push_back(job);
#else
        else
        {
            jobs.push_back(job);
        }
#endif  
    }

    return r;
}

Job potoroo::Job::parseArgs(const ArgList& args)
{
    string in = args.get(argType::inFile).getValue();
    fs::path inPath;
    string out;
    string tag;

    try { inPath = fs::path(in); }
    catch (exception& ex) { return invalidInFilenameJob(in, ex.what()); }
    catch (...) { return invalidInFilenameJob(in, ""); }

    if (args.contains(argType::outFile))
    {
        out = args.get(argType::outFile).getValue();
    }
    else
    {
        try
        {
            fs::path p(args.get(argType::outDir).getValue());
            p /= inPath.filename();
            out = p.string();
        }
        catch (...)
        {
            out = args.get(argType::outDir).getValue();
            // further exception handling in processor
        }
    }

    string ext;
    try { ext = inPath.extension().string(); }
    catch (exception& ex) { return invalidInFilenameJob(in, ex.what()); }
    catch (...) { return invalidInFilenameJob(in, ""); }

    if (args.get(argType::tag).getValue().compare(0, 7, "custom:") == 0) tag = string(args.get(argType::tag).getValue(), 7);
    else if (tagCondCpp(ext) || (args.get(argType::tag).getValue() == "cpp")) tag = tagCpp;
    else if (tagCondBash(ext) || (args.get(argType::tag).getValue() == "bash")) tag = tagBash;
    else if (tagCondBatch(ext) || (args.get(argType::tag).getValue() == "batch")) tag = tagBatch;
    else
    {
        Job j;
        j.setValidity(false);
        j.setErrorMsg("unable to determine tag");
        return j;
    }

    if (tag.length() > 15)
    {
        Job j;
        j.setValidity(false);
        j.setErrorMsg("tag too long");
        return j;
    }

    try { return Job(inPath.string(), out, tag, args.contains(argType::wError)); }
    catch (exception& ex) { return invalidInFilenameJob(in, ex.what()); }
    catch (...) { return invalidInFilenameJob(in, ""); }
}
