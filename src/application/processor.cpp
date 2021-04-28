/*!

\author         Oliver Blaser
\date           08.04.2021
\copyright      GNU GPLv3 - Copyright (c) 2021 Oliver Blaser

*/

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "processor.h"
#include "middleware/cliTextFormat.h"
#include "middleware/util.h"

namespace fs = std::filesystem;

using namespace std;
using namespace cli;
using namespace potoroo;

namespace
{
    const string keyWord_rmStart = "rm";
    const string keyWord_rmEnd = "endrm";
    const string keyWord_rmn = "rmn";
    const string keyWord_ins = "ins";
    const string keyWord_include = "include";

    //const char incPathType_path_Char = '<';
    //const char incPathType_path_CloseingChar = '>';
    const char incPathType_rel_Char = '\"';
    const char incPathType_rel_CloseingChar = '\"';
    const char incPathType_dirty_Char = '\'';
    const char incPathType_dirty_CloseingChar = '\'';

    enum WARNING_ID : int
    {
        /* 0..99 reserved */

        _wID_first = 100,

        wID_rmOut_file,
        wID_rmOut_dir,
        wID_include_emptyFile, // r += warn(ewiFile, wID_, job, , ProcPos());
        wID_include_multiInc,
        wID_include_loop,
        wID_tagInRMx,
        wID_instrLineEnd,
        wID_rmnEOF,
        wID_endlAssumeLF,
        wID_convEndlFail,

        _wID_last
    };

    enum class KeyWord
    {
        unknown,

        rmStart,
        rmEnd,
        rmn,
        ins,
        include
    };

    struct ProcPos
    {
        ProcPos() : ln(0), col(0) {}
        ProcPos(size_t line) : ln(line), col(0) {}
        ProcPos(size_t line, size_t column) : ln(line), col(column) {}

        size_t ln;
        size_t col;
    };

    class AbsPathStack
    {
    public:
        AbsPathStack() { clear(); }
        ~AbsPathStack() {}

        void clear()
        {
            v.clear();
        }

        bool contains(const fs::path& path) const
        {
            error_code ec;

            for (size_t i = 0; i < v.size(); ++i)
            {
                if (fs::equivalent(path, v[i], ec)) return true;
            }

            return false;
        }

        void pop()
        {
            v.pop_back();
        }

        void push(const fs::path& path)
        {
            v.push_back(fs::absolute(path));
        }

        size_t size() const
        {
            return v.size();
        }

        std::string toString() const
        {
            string s = "";

            for (size_t i = 0; i < v.size(); ++i)
            {
                if (i > 0) s += '\n';
                s += v[i].string();
            }

            return s;
        }

    private:
        vector<fs::path> v;
    };

    AbsPathStack incPathStack;
    AbsPathStack incPathHistory;

    void printError(const std::string& file, const std::string& text, size_t line = 0, size_t col = 0)
    {
        printEWI(file, text, line, col, 0, 1);
    }
    void printError(const std::string& file, const std::string& text, const ProcPos& procPos)
    {
        printError(file, text, procPos.ln, procPos.col);
    }

    void printWarning(const std::string& file, const std::string& text, size_t line = 0, size_t col = 0)
    {
        printEWI(file, text, line, col, 1, 1);
    }
    void printWarning(const std::string& file, const std::string& text, const ProcPos& procPos)
    {
        printWarning(file, text, procPos.ln, procPos.col);
    }

    void printInfo(const std::string& file, const std::string& text, size_t line = 0, size_t col = 0)
    {
        printEWI(file, text, line, col, 2, 1);
    }
    void printInfo(const std::string& file, const std::string& text, const ProcPos& procPos)
    {
        printInfo(file, text, procPos.ln, procPos.col);
    }

#if PRJ_DEBUG
    void printDbg(const std::string& file, const std::string& text, size_t line = 0, size_t col = 0)
    {
        printEWI(file, text, line, col, -1, 1);
    }
    void printDbg(const std::string& file, const std::string& text, const ProcPos& procPos)
    {
        printDbg(file, text, procPos.ln, procPos.col);
    }
#endif

    Result warn(const std::string& file, int wID, const Job& job, const std::string& msg, const ProcPos& procPos = ProcPos(0, 0))
    {
        Result r;

        if (!vectorContains(job.getWSupList(), wID))
        {
            ++r.warn;
            printWarning(file, msg + " [" + to_string(wID) + "]", procPos);
        }

        return r;
    }

    //! @brief Checks if *p is space
    bool isSpace(const char* p)
    {
        return (
            (*p == 0x09) || // TAB
            (*p == 0x20)    // Space
            );
    }

    //! @brief Checks if *p is LF
    bool isNewLine(const char* p)
    {
        return (*p == 0x0A);
    }

    //! @brief Checks if *p is space or LF
    bool isWhiteSpace(const char* p)
    {
        return (
            isSpace(p) ||
            isNewLine(p)
            );
    }

    KeyWord getKW(const string& kwStr)
    {
        KeyWord kw = KeyWord::unknown;

        if (kwStr.compare(keyWord_rmStart) == 0) kw = KeyWord::rmStart;
        if (kwStr.compare(keyWord_rmEnd) == 0) kw = KeyWord::rmEnd;
        if (kwStr.compare(keyWord_rmn) == 0)  kw = KeyWord::rmn;
        if (kwStr.compare(keyWord_ins) == 0) kw = KeyWord::ins;
        if (kwStr.compare(keyWord_include) == 0) kw = KeyWord::include;
        //if (kwStr.compare(keyWord_) == 0) kw = KeyWord::;

        return kw;
    }

    Result rmOut(const fs::path& outf, const string& ewiFile, const Job& job, bool dir) noexcept
    {
        Result r;

        const string rmFileErrorMsg = "invalid/temporary output file not deleted";

        try { fs::remove(outf); }
        catch (fs::filesystem_error& ex)
        {
            r += warn(ewiFile, wID_rmOut_file, job, rmFileErrorMsg + ": " + ex.what());
        }
        catch (exception& ex)
        {
            r += warn(ewiFile, wID_rmOut_file, job, rmFileErrorMsg + ":\n" + ex.what());
        }
        catch (...)
        {
            r += warn(ewiFile, wID_rmOut_file, job, rmFileErrorMsg);
        }

        if (dir)
        {
            const string rmDirErrorMsg = "temporary output directory not deleted";

            try { fs::remove(outf.parent_path()); }
            catch (fs::filesystem_error& ex)
            {
                r += warn(ewiFile, wID_rmOut_dir, job, rmDirErrorMsg + ": " + ex.what());
            }
            catch (exception& ex)
            {
                r += warn(ewiFile, wID_rmOut_dir, job, rmDirErrorMsg + ":\n" + ex.what());
            }
            catch (...)
            {
                r += warn(ewiFile, wID_rmOut_dir, job, rmDirErrorMsg);
            }
        }

        return r;
    }

    char getCloseingIncPathChar(char openingChar)
    {
        //if (openingChar == incPathType_path_Char) return incPathType_path_CloseingChar;       // future use
        if (openingChar == incPathType_rel_Char) return incPathType_rel_CloseingChar;
        if (openingChar == incPathType_dirty_Char) return incPathType_dirty_CloseingChar;

        return 0;
    }

    Result includeDirtyProc(ofstream& outFileStream, const fs::path& incFile, const Job& job, const string& ewiFile, const ProcPos& pPos, size_t pathCol)
    {
        Result r;

        ofstream& ofs = outFileStream;
        ifstream ifs;

        ifs.open(incFile, ios::in | ios::binary);

        char c = static_cast<char>(ifs.get());

        if (!ifs.good())
        {
            r += warn(ewiFile, wID_include_emptyFile, job, "empty include file", ProcPos(pPos.ln, pathCol));
        }

        while (ifs.good())
        {
            ofs.put(c);
            c = static_cast<char>(ifs.get());
        }

        return r;
    }

    Result includeDirty(ofstream& outFileStream, const fs::path& incFile, const fs::path& outf, const Job& job, const string& ewiFile, const ProcPos& pPos, size_t pathCol)
    {
        Result r;

        lineEnding ile = detectLineEnding(incFile);

        if (ile == lineEnding::error)
        {
            r += warn(ewiFile, wID_endlAssumeLF, job, "Unable to determine line ending, assuming LF");
            ile = lineEnding::LF;
        }

        if (ile == lineEnding::LF) r += includeDirtyProc(outFileStream, incFile, job, ewiFile, pPos, pathCol);
        else
        {
            const fs::path tmpProcDir(outf.parent_path() / processorTmpDirLineEnding);
            const fs::path incfLF(tmpProcDir / (incFile.filename().string() + ".incfLF"));

            fs::create_directories(tmpProcDir);

            string errMsg;
            bool deleteTmpProcDir = true;

            if (convertLineEnding(incFile, ile, incfLF, lineEnding::LF, errMsg) == 0)
            {
                r += includeDirtyProc(outFileStream, incfLF, job, ewiFile, pPos, pathCol);

                if (r.err != 0) deleteTmpProcDir = false;
            }
            else
            {
                ++r.err;
                printError(ewiFile, "convert line ending of include file failed" + (errMsg.length() > 0 ? (" - " + errMsg) : ""), ProcPos(pPos.ln, pathCol));
            }

            if (deleteTmpProcDir) fs::remove_all(tmpProcDir);
        }

        return r;
    }

    Result includeRel(ofstream& outFileStream, const fs::path& incFile, const fs::path& outf, const Job& job, const string& ewiFile, const ProcPos& pPos, size_t pathCol)
    {
        Result r;

        ofstream& ofs = outFileStream;

        fs::path incOutf = outf.parent_path() / processorTmpDirIncOut / incFile.filename();
        Job tmpJob = job;
        tmpJob.setInputFile(incFile.string());
        tmpJob.setOutputFile(incOutf.string());

        const fs::path currentWD = fs::current_path();
        string cwdExWhat;
        Result cwdr = changeWD(incFile.parent_path(), &cwdExWhat);

        if (cwdr > 0)
        {
            ++r.err;
            printError(ewiFile, "could not change working dir to process included file: " + cwdExWhat);
        }
        else
        {
            Result recoursiveResult = processJob(tmpJob, true);
            r += recoursiveResult;

            cwdr = changeWD(currentWD, &cwdExWhat);
            if (cwdr > 0)
            {
                ++r.err;
                printError(ewiFile, "could not change back to working dir after processing included file: " + cwdExWhat);
            }

            if ((recoursiveResult.err == 0) && (cwdr == 0))
            {
                r += includeDirty(ofs, incOutf, outf, job, ewiFile, pPos, pathCol);
                r += rmOut(incOutf, ewiFile, job, true);
            }
        }

        return r;
    }







#if PRJ_DEBUG && 0
    const size_t pbSizeMin = 200;
#else
    const size_t pbSizeMin = 100 * 1024; // 100k
#endif

    typedef char fileIOt;
    //typedef uint8_t fileIOt;


    //! @brief Reads some bytes from the input file
    //! @param ifs 
    //! @param [out] data 
    //! @return Number of read bytes
    //! 
    //! Aligned to new lines and thus does neither chop UTF-8 chunks nor tags.
    //! 
    size_t readsome(ifstream& ifs, vector<fileIOt>& data)
    {
        size_t nRead = 0;

        try
        {
            data.push_back(static_cast<fileIOt>(ifs.get()));
            ++nRead;
        }
        catch (...) { return 0; }

        while (
            (nRead < pbSizeMin) ||
            (data.at(data.size() - 1) != static_cast<fileIOt>(0x0A))
            )
        {
            try { data.push_back(static_cast<fileIOt>(ifs.get())); }
            catch (...) { break; }

            ++nRead;
        }

        return nRead;
    }

    // should not throw explicitly because then the out file does not get deleted.
    // expects LF files
    Result caterpillarProc(const fs::path& inf, const fs::path& outf, const Job& job, const string& ewiFile)
    {
        Result r;

        vector<fileIOt> procBuff;
        vector<fileIOt> outBuff;

        const string tag = job.getTag() + " ";

        ProcPos pPos(0, 1); // 0 to detect first read
                            // processed line/column used to display error, threrefore starting with 1

        bool eof = false;
        bool skipThisLine = false;
        bool proc_rm = false;
        ProcPos proc_rm_startPos;
        size_t proc_rmn = 0;

        ifstream ifs;
        ofstream ofs;

        ifs.exceptions(ios::failbit | ios::badbit | ios::eofbit);
        ofs.exceptions(ios::failbit | ios::badbit | ios::eofbit);

        ifs.open(inf, ios::in | ios::binary);
        ofs.open(outf, ios::out | ios::binary);

        while (!eof)
        {
            procBuff.clear();
            size_t nRead = readsome(ifs, procBuff);
            const fileIOt* const pb = procBuff.data();
            const fileIOt* const pMax = pb + nRead;
            const fileIOt* p = pb;

            if ((nRead < pbSizeMin) || ifs.eof()) eof = true;

            // only on first read
            if (pPos.ln == 0)
            {
                pPos.ln = 1;

                // UTF BOM check
                if (procBuff.size() >= 4)
                {
                    if (procBuff[0] == static_cast<fileIOt>(0x00) && procBuff[1] == static_cast<fileIOt>(0x00) &&
                        procBuff[2] == static_cast<fileIOt>(0xFe) && procBuff[3] == static_cast<fileIOt>(0xFF))
                    {
                        printError(ewiFile, "encoding not supported: UTF-32 BE");
                        return 1;
                    }
                    if (procBuff[0] == static_cast<fileIOt>(0xFF) && procBuff[1] == static_cast<fileIOt>(0xFe) &&
                        procBuff[2] == static_cast<fileIOt>(0x00) && procBuff[3] == static_cast<fileIOt>(0x00))
                    {
                        printError(ewiFile, "encoding not supported: UTF-32 LE");
                        return 1;
                    }
                }

                if (procBuff.size() >= 2)
                {
                    if (procBuff[0] == static_cast<fileIOt>(0xFe) && procBuff[1] == static_cast<fileIOt>(0xFF))
                    {
                        printError(ewiFile, "encoding not supported: UTF-16 BE");
                        return 1;
                    }
                    if (procBuff[0] == static_cast<fileIOt>(0xFF) && procBuff[1] == static_cast<fileIOt>(0xFe))
                    {
                        printError(ewiFile, "encoding not supported: UTF-16 LE");
                        return 1;
                    }
                }

                // UTF-8 BOM is copied like every other UTF-8 byte
            }






            // process
            while (p < pMax)
            {
                outBuff.clear();

                // copy whitespace
                while ((p < pMax) && isSpace(p))
                {
                    outBuff.push_back(*p);
                    ++p;
                    ++pPos.col;
                }


                // tag?
                if (p < (pMax - tag.length()))
                {
                    if (tag.compare(0, tag.length(), p, tag.length()) == 0)
                    {
                        // yes, tag

                        const size_t tagCol = pPos.col;

                        p += tag.length();
                        pPos.col += tag.length();


                        // skip space
                        while ((p < pMax) && isSpace(p))
                        {
                            ++p;
                            ++pPos.col;
                        }


                        const size_t kwCol = pPos.col;

                        // determine keyword
                        string kwStr = "";

                        while ((p < pMax) && !isWhiteSpace(p))
                        {
                            kwStr += *p;
                            ++p;
                            ++pPos.col;
                        }

                        KeyWord kw = getKW(kwStr);


                        if ((proc_rm && (kw != KeyWord::rmEnd)) || proc_rmn)
                        {
                            r += warn(ewiFile, wID_tagInRMx, job, "###tags inside @rm@ or @rmn@ scopes are ignored", ProcPos(pPos.ln, tagCol));
                        }
                        else
                        {

                            // process key words
                            if (kw == KeyWord::rmStart)
                            {
                                proc_rm = true;
                            }
                            else if (kw == KeyWord::rmEnd)
                            {
                                if (proc_rm)
                                {
                                    proc_rm = false;
                                    outBuff.clear();
                                    skipThisLine = true;
                                }
                                else
                                {
                                    ++r.err;
                                    printError(ewiFile, "###unexpected \"endrm\"", pPos.ln, kwCol);
                                }
                            }
                            else if (kw == KeyWord::rmn)
                            {
                                // skip space
                                while ((p < pMax) && isSpace(p))
                                {
                                    ++p;
                                    ++pPos.col;
                                }

                                const size_t argCol = pPos.col;

                                // get arg
                                string arg = "";
                                while ((p < pMax) && !isWhiteSpace(p))
                                {
                                    arg += *p;
                                    ++p;
                                    ++pPos.col;
                                }


                                if (arg.length() == 0)
                                {
                                    ++r.err;
                                    printError(ewiFile, "###missing argument of @rmn@", pPos.ln, pPos.col);
                                }
                                else if ((arg.length() == 1) && (arg[0] >= 0x30) && (arg[0] <= 0x39))
                                {
                                    proc_rmn = (arg[0] - 0x30) + 1;
                                }
                                else
                                {
                                    ++r.err;
                                    printError(ewiFile, "###invalid argument of @rmn@: \"" + arg + "\"", pPos.ln, argCol);
                                }
                            }
                            else if (kw == KeyWord::ins)
                            {
                                if (p < pMax)
                                {
                                    ++p; // skip the space
                                    ++pPos.col;
                                }
                            }
                            else if (kw == KeyWord::include)
                            {
                                skipThisLine = true;

                                // skip space
                                while ((p < pMax) && isSpace(p))
                                {
                                    ++p;
                                    ++pPos.col;
                                }

                                const size_t pathCol = pPos.col;

                                if (p >= pMax)
                                {
                                    ++r.err;
                                    printError(ewiFile, "###missing argument of @include@", pPos.ln, pPos.col);
                                }
                                else
                                {
                                    // get path type
                                    char pathTypeChar = *p;
                                    char pathTypeCloseingChar = getCloseingIncPathChar(pathTypeChar);
                                    ++p;
                                    ++pPos.col;

                                    if (pathTypeCloseingChar == 0)
                                    {
                                        ++r.err;
                                        printError(ewiFile, "invalid path type", pPos.ln, pathCol);
                                    }
                                    else
                                    {
                                        // get path
                                        string pathStr = "";
                                        while ((p < pMax) && ((*p != pathTypeCloseingChar) || (*(p - 1) == '\\')) && !isNewLine(p)) // p-1 is a valid pointer at this position, because there is allway a tag before p.
                                        {
                                            pathStr += *p;
                                            ++p;
                                            ++pPos.col;
                                        }
                                        char replace[] = { '\\', pathTypeChar, 0 };
                                        char replaceWith[] = { pathTypeChar, 0 };
                                        strReplaceAll(pathStr, replace, replaceWith);
                                        replace[1] = pathTypeCloseingChar;
                                        replaceWith[0] = pathTypeCloseingChar;
                                        strReplaceAll(pathStr, replace, replaceWith);

                                        if ((*p == pathTypeCloseingChar) && (pathStr.length() > 0))
                                        {
                                            ++p;
                                            ++pPos.col;

                                            fs::path incPath(pathStr);

                                            if (incPath.is_relative())
                                            {
                                                incPath = fs::absolute(job.getInputFile()).parent_path() / pathStr;
                                            }
#if PRJ_DEBUG && 0
                                            string incTypeDispStr = "?";
                                            if (pathTypeChar == incPathType_rel_Char) incTypeDispStr = "relative to file";
                                            else if (pathTypeChar == incPathType_path_Char) incTypeDispStr = "relative to path in potoroo config";
                                            else if (pathTypeChar == incPathType_dirty_Char) incTypeDispStr = "relative to file (no preProc, dirty include)";
                                            printDbg(ewiFile, "###include path: \"" + pathStr + "\" => \"" + incPath.string() + "\" - " + incTypeDispStr, pPos);
#endif
                                            if (fs::exists(incPath))
                                            {
                                                if (!incPathStack.contains(incPath))
                                                {
                                                    incPathStack.push(incPath);

                                                    if (incPathHistory.contains(incPath))
                                                    {
                                                        r += warn(ewiFile, wID_include_multiInc, job, "included same file multiple times", ProcPos(pPos.ln, pathCol));
                                                    }

                                                    incPathHistory.push(incPath);

                                                    if (pathTypeChar == incPathType_rel_Char) r += includeRel(ofs, incPath, outf, job, ewiFile, pPos, pathCol);
                                                    else if (pathTypeChar == incPathType_dirty_Char) r += includeDirty(ofs, incPath, outf, job, ewiFile, pPos, pathCol);
                                                    else
                                                    {
                                                        ++r.err;
                                                        printError(ewiFile, "ERROR - unimplemented include path type - " + string(__FILENAME__) + ":" + to_string(__LINE__), pPos);
                                                    }

                                                    incPathStack.pop();
                                                }
                                                else
                                                {
                                                    ++r.err;
                                                    printError(ewiFile, "include loop detected - include stack:\n" + incPathStack.toString(), pPos.ln, pathCol);
                                                }
                                            }
                                            else
                                            {
                                                ++r.err;
                                                printError(ewiFile, "include file does not exist", pPos.ln, pathCol);
                                            }
                                        }
                                        else
                                        {
                                            ++r.err;
                                            printError(ewiFile, "invalid include path", pPos.ln, pathCol);
                                        }
                                    }
                                }
                            }
                            else
                            {
                                ++r.err;
                                printError(ewiFile, "###unknown key word \"" + kwStr + "\"", pPos.ln, kwCol);
                            }


                            // check if line is empty after key word
                            if (p < pMax)
                            {
                                if ((kw != KeyWord::ins) && !isNewLine(p))
                                {
                                    r += warn(ewiFile, wID_instrLineEnd, job, "no new line after expression", pPos);
                                }
                            }
                        }
                    }
                }



                // copy until line end
                while ((p < pMax) && !isNewLine(p))
                {
                    outBuff.push_back(*p);
                    ++p;
                    ++pPos.col;
                }



                // new line
                if (p < pMax)
                {
                    outBuff.push_back(*p);
                    ++p;
                }

                ++pPos.ln;
                pPos.col = 1;

                if ((proc_rmn > 0) || proc_rm || skipThisLine)
                {
                    skipThisLine = false;
                    outBuff.clear();
                }

                if (proc_rmn > 0) --proc_rmn;



                // write to outf
                if (outBuff.size() > 0) ofs.write(outBuff.data(), outBuff.size());
            }

#if PRJ_DEBUG && 0
            for (int i = 0; i < 2; ++i) ofs << (char)0xE2 << (char)0x96 << (char)0x88; // full block
#endif
        }

        if (proc_rm)
        {
            ++r.err;
            printError(ewiFile, "###missing @endrm@", pPos.ln, pPos.col);
        }

        if (proc_rmn)
        {
            r += warn(ewiFile, wID_rmnEOF, job, "###@rmn@ overlapped EOF", pPos);
        }

        ifs.close();
        ofs.close();

        return r;
    }
}

Result potoroo::processJob(const Job& job, bool forceOutfLineEndLF) noexcept
{
    Result r;
    fs::path inf_data;
    fs::path outf_data;
    const fs::path& inf = inf_data;
    const fs::path& outf = outf_data;
    string ewiFile;
    bool createdOutDir = false;

    try
    {
        inf_data = fs::absolute(job.getInputFile());
        outf_data = fs::absolute(job.getOutputFile());
    }
    catch (exception& ex)
    {
        ++r.err;
        printError("", ex.what());
    }
    catch (...)
    {
        ++r.err;
        printError("", "invalid in or out filename");
    }

    if (r.err == 0)
    {
        try { ewiFile = inf.filename().string(); }
        catch (...) { ewiFile = job.getInputFile(); }

        try
        {
            if (!fs::exists(inf)) throw runtime_error("file does not exist");

            if (fs::exists(outf))
            {
                if (fs::equivalent(inf, outf)) throw runtime_error("in and out files are the same");
            }

            createdOutDir = fs::create_directories(outf.parent_path());



            if (job.getMode() == JobMode::proc)
            {
                lineEnding ile = detectLineEnding(inf);

                if (ile == lineEnding::error)
                {
                    r += warn(ewiFile, wID_endlAssumeLF, job, "Unable to determine line ending, assuming LF");
                    ile = lineEnding::LF;
                }

                if (ile == lineEnding::LF) r += caterpillarProc(inf, outf, job, ewiFile);
                else
                {
                    const fs::path tmpProcDir(outf.parent_path() / processorTmpDirLineEnding);
                    const fs::path infLF(tmpProcDir / (inf.filename().string() + ".infLF"));
                    fs::path outfLF;

                    if (forceOutfLineEndLF) outfLF = fs::path(outf);
                    else outfLF = fs::path(tmpProcDir / (outf.filename().string() + ".outfLF"));

                    fs::create_directories(tmpProcDir);

                    string errMsg;
                    bool deleteTmpProcDir = true;

                    if (convertLineEnding(inf, ile, infLF, lineEnding::LF, errMsg) == 0)
                    {
                        r += caterpillarProc(infLF, outfLF, job, ewiFile);

                        if ((r.err == 0) && !forceOutfLineEndLF)
                        {
                            if (convertLineEnding(outfLF, lineEnding::LF, outf, ile, errMsg) != 0)
                            {
                                deleteTmpProcDir = false;
                                r += warn(ewiFile, wID_convEndlFail, job, "convert line ending failed" + (errMsg.length() > 0 ? (" - " + errMsg) : ""));
                            }
                        }
                    }
                    else
                    {
                        ++r.err;
                        printError(ewiFile, "convert line ending of include file failed" + (errMsg.length() > 0 ? (" - " + errMsg) : ""));
                    }

                    if (deleteTmpProcDir) fs::remove_all(tmpProcDir);
                }
            }
            else if (job.getMode() == JobMode::copy)
            {
                bool fileCopied = fs::copy_file(inf, outf, fs::copy_options::update_existing);

#if PRJ_DEBUG && 0
                if (!fileCopied) printDbg(ewiFile, "file not copied, it's up to date");
#endif
            }
            else if (job.getMode() == JobMode::copyow)
            {
                bool fileCopied = fs::copy_file(inf, outf, fs::copy_options::overwrite_existing);

                if (!fileCopied)
                {
                    ++r.err;
                    printError(ewiFile, "file not copied");
                }
            }
            else
            {
                ++r.err;
                printError("processor", "invalid job mode");
            }
        }
        catch (exception& ex)
        {
            ++r.err;
            printError(ewiFile, ex.what());

#if PRJ_DEBUG
            int ___dbg_breakpoint = 0;
#endif
        }
        catch (...)
        {
            ++r.err;
            printError(ewiFile, "unknown");
        }
    }

    if (job.warningAsError() && (r.warn > 0))
    {
        ++r.err;
        printError(ewiFile, "###[@Werror@] " + to_string(r.warn) + " warnings");
    }

    if ((r.err > 0) && !fs::equivalent(inf, outf)) r += rmOut(outf, ewiFile, job, createdOutDir);

    return r;
}

Result potoroo::processJobs(const std::vector<Job>& jobs, std::vector<bool>& success) noexcept
{
#if PRJ_DEBUG && 0
    cout << "===============\n" << "jobs:" << endl;
    for (size_t i = 0; i < jobs.size(); ++i)
    {
        cout << setw(3) << i << " ";

        if (jobs[i].isValid())
        {
            cout << "\"" << jobs[i].getInputFile() << "\" " << "\"" << jobs[i].getOutputFile() << "\" " << "\"" << jobs[i].getTag() << "\"" << (jobs[i].warningAsError() ? " Werror" : "");
        }
        else cout << "invalid: " << jobs[i].getErrorMsg();

        cout << endl;
    }
    cout << "===============" << endl;
#endif

#if PRJ_DEBUG && 0
    printInfo("processor", "current dir: " + pathToStr(fs::current_path()) + "\n");
#endif


    Result pr;

    if (jobs.size() != success.size())
    {
        printEWI("internal", "fatal! " + string(__FILENAME__) + ":" + to_string(__LINE__) + ": jobs.size() != success.size()", 0, 0, 0, 0);
        ++pr.err;
    }

    for (size_t i = 0; i < jobs.size(); ++i)
    {
        Job j = jobs[i];

        cout << "process " << j << endl;

        incPathStack.clear();
        incPathHistory.clear();

        Result r = processJob(j);
        pr += r;

        if (i < success.size())
        {
            success[i] = (r.err == 0);
        }
    }

    return pr;
}
