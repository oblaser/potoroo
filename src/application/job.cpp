/*!

\author         Oliver Blaser
\date           18.02.2021
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







potoroo::Result::Result()
    : err(0), warn(0)
{
}

potoroo::Result::Result(int ne)
    : err(ne), warn(0)
{
}

potoroo::Result::Result(int ne, int nw)
    : err(ne), warn(nw)
{
}

potoroo::Result potoroo::Result::operator+(const Result& summand)
{
    return Result(this->err + summand.err, this->warn + summand.warn);
}

potoroo::Result& potoroo::Result::operator+=(const Result& summand)
{
    this->err += summand.err;
    this->warn += summand.warn;
    return *this;
}

bool potoroo::operator>(const potoroo::Result& left, int right)
{
    return ((left.err > right) || (left.warn > right) || ((left.err + left.warn) > right));
}

bool potoroo::operator==(const Result& left, int right)
{
    return ((left.err == right) && (left.warn == right));
}

std::ostream& potoroo::operator<<(std::ostream& os, const potoroo::Result& v)
{
    os << "errors: ";
    os << sgr(SGRFGC_BRIGHT_RED) << v.err << sgr(SGR_RESET);
    os << "   warnings: ";
    os << sgr(SGRFGC_BRIGHT_YELLOW) << v.warn << sgr(SGR_RESET);

    return os;
}





potoroo::JobFileLine::JobFileLine()
    : line(0), data("")
{
}

potoroo::JobFileLine::JobFileLine(size_t l, const std::string& d)
    : line(l), data(d)
{
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







//! @brief Change working dir to process jobfile
//! @param jobfile 
//! @return 0 on success
Result potoroo::changeWD(const std::string& jobfile)
{
    int r = 1;
    const string procStr = "change working dir";

#if PRJ_DEBUG && 0
    {
        ofstream f;
        f.open("000_cwd_before");
        f << "000_cwdTest" << endl;
        f.close();
    }
#define ___DBG_JOB_CWD_CREATEFILE (1)
#endif

    try
    {
        fs::current_path(fs::path(jobfile).parent_path());
        r = 0;
    }
    catch (fs::filesystem_error& ex)
    {
#if PRJ_DEBUG
        printError(procStr, ex.what());
#else
        printError(procStr, ex.code().message() + fsExceptionPath(ex));
#endif
    }
    catch (exception& ex)
    {
        printError(procStr, ex.what());
    }
    catch (...)
    {
        printError(procStr, "unknown");
    }

#if PRJ_DEBUG
#ifdef ___DBG_JOB_CWD_CREATEFILE
    if (r == 0)
    {
        ofstream f;
        f.open("000_cwd_after");
        f << "000_cwdTest" << endl;
        f.close();
    }
#endif
#endif

    return r;
}

std::string potoroo::fsExceptionPath(const std::filesystem::filesystem_error& ex)
{
    string path1 = ex.path1().string();
    string path2 = ex.path2().string();
    string path = "";

    if ((path1.length() >= 0) || (path2.length() >= 0))
    {
        if (path2.length() == 0) path += " \"" + path1 + "\"";
        else path += "\npath1: \"" + path1 + "\"\npath2: \"" + path2 + "\"";
    }

    return path;
}

void potoroo::printEWI(const std::string& file, const std::string& text, size_t line, size_t col, int ewi, int style)
{
    // because of the sgr formatting we cant use iomanip
    size_t printedWidth = 0;


    printedWidth = file.length() + 1;

    if (style == 0) cout << file << ":";
    else if (style == 1) cout << sgr(SGRFGC_BRIGHT_WHITE) << file << sgr(SGR_RESET) << ":";
    else cout << sgr(SGRFGC_BRIGHT_MAGENTA, SGR_BOLD) << "#printEWI style: " << style << "# " << sgr(SGR_RESET) << file << ":";


    if (line > 0)
    {
        const string lineStr = to_string(line);
        printedWidth += lineStr.length() + 1;

        cout << sgr(SGRFGC_BRIGHT_WHITE) << lineStr << sgr(SGR_RESET) << ":";
    }

    if (col > 0)
    {
        if (line <= 0)
        {
            ++printedWidth;
            cout << ":";
        }

        const string colStr = to_string(col);
        printedWidth += colStr.length() + 1;
        cout << sgr(SGRFGC_BRIGHT_WHITE) << colStr << sgr(SGR_RESET) << ":";
    }
    cout << " ";

    while (printedWidth++ < 21) cout << " ";


    const size_t ewiWidth = 9;

    if (ewi == 0) cout << sgr(SGRFGC_BRIGHT_RED, SGR_BOLD) << left << setw(ewiWidth) << "error:";
    else if (ewi == 1) cout << sgr(SGRFGC_BRIGHT_YELLOW, SGR_BOLD) << left << setw(ewiWidth) << "warning:";
    else if (ewi == 2) cout << sgr(SGRFGC_BRIGHT_CYAN, SGR_BOLD) << left << setw(ewiWidth) << "info:";

#if PRJ_DEBUG
    else if (ewi == -1) cout << sgr(SGRFGC_BRIGHT_MAGENTA, SGR_BOLD) << left << setw(ewiWidth) << "debug:";
#endif

    else  cout << sgr(SGRFGC_BRIGHT_MAGENTA, SGR_BOLD) << "#printEWI ewi: " << ewi << "# " << sgr(SGRFGC_BRIGHT_RED, SGR_BOLD) << "error: ";



    cout << sgr(SGR_RESET);

    if (text.length() > 5)
    {
        if ((text[0] == '#') &&
            (text[1] == '#') &&
            (text[2] == '#')
            )
        {
            bool on = false;

            size_t i = 3;

            while (i < text.length())
            {
                if (text[i] == '\"')
                {
                    if (on)
                    {
                        cout << sgr(SGR_RESET);
                        cout << text[i];
                        on = false;
                    }
                    else
                    {
                        cout << text[i];
                        cout << sgr(SGRFGC_BRIGHT_WHITE);
                        on = true;
                    }
                }
                else if (text[i] == '@')
                {
                    if (on)
                    {
                        cout << sgr(SGR_RESET);
                        on = false;
                    }
                    else
                    {
                        cout << sgr(SGRFGC_BRIGHT_WHITE);
                        on = true;
                    }
                }
                else cout << text[i];

                ++i;
            }

            cout << sgr(SGR_RESET) << endl;
            return;
        }
    }

    cout << text << endl;
}
