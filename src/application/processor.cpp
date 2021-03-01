/*!

\author         Oliver Blaser
\date           01.03.2021
\copyright      GNU GPLv3 - Copyright (c) 2021 Oliver Blaser

*/

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include "processor.h"
#include "middleware/cliTextFormat.h"

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

    enum class KeyWord
    {
        unknown,

        rmStart,
        rmEnd,
        rmn,
        ins
    };

    struct ProcPos
    {
        ProcPos() : ln(0), col(0) {}
        ProcPos(size_t line, size_t column) : ln(line), col(column) {}

        size_t ln;
        size_t col;
    };

    void printError(const std::string& file, const std::string& text, size_t line = 0, size_t col = 0)
    {
        printEWI(file, text, line, col, 0, 1);
    }

    void printWarning(const std::string& file, const std::string& text, size_t line = 0, size_t col = 0)
    {
        printEWI(file, text, line, col, 1, 1);
    }

    void printInfo(const std::string& file, const std::string& text, size_t line = 0, size_t col = 0)
    {
        printEWI(file, text, line, col, 2, 1);
    }

#if PRJ_DEBUG
    void printDbg(const std::string& file, const std::string& text, size_t line = 0, size_t col = 0)
    {
        printEWI(file, text, line, col, -1, 1);
    }
#endif

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
        //if (kwStr.compare(keyWord_) == 0) kw = KeyWord::;

        return kw;
    }








#if PRJ_DEBUG
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
                            ++r.warn;
                            printWarning(ewiFile, "###tags inside @rm@ or @rmn@ scopes are ignored", pPos.ln, tagCol);
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
                                    ++r.warn;
                                    printWarning(ewiFile, "no new line after expression", pPos.ln, pPos.col);
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
            ++r.warn;
            printWarning(ewiFile, "###@rmn@ overlapped EOF", pPos.ln, pPos.col);
        }

        ifs.close();
        ofs.close();

        return r;
    }

    Result rmOut(const fs::path& outf, const string& ewiFile, bool dir) noexcept
    {
        Result r;

        const string rmFileErrorMsg = "invalid output file not deleted";

        try { fs::remove(outf); }
        catch (fs::filesystem_error& ex)
        {
            ++r.warn;
            printWarning(ewiFile, rmFileErrorMsg + ":\n" + ex.code().message() + fsExceptionPath(ex));
        }
        catch (exception& ex)
        {
            ++r.warn;
            printWarning(ewiFile, rmFileErrorMsg + ":\n" + ex.what());
        }
        catch (...)
        {
            ++r.warn;
            printWarning(ewiFile, rmFileErrorMsg);
        }

        if (dir)
        {
            const string rmDirErrorMsg = "output directory not deleted";

            try { fs::remove(outf.parent_path()); }
            catch (fs::filesystem_error& ex)
            {
                ++r.warn;
                printWarning(ewiFile, rmDirErrorMsg + ":\n" + ex.code().message() + fsExceptionPath(ex));
            }
            catch (exception& ex)
            {
                ++r.warn;
                printWarning(ewiFile, rmDirErrorMsg + ":\n" + ex.what());
            }
            catch (...)
            {
                ++r.warn;
                printWarning(ewiFile, rmDirErrorMsg);
            }
        }

        return r;
    }
}

Result potoroo::processJob(const Job& job) noexcept
{
    Result r;
    fs::path inf;
    fs::path outf;
    string ewiFile;
    bool createdOutDir = false;

    try
    {
        inf = fs::path(job.getInputFile());
        outf = fs::path(job.getOutputFile());
    }
    catch (fs::filesystem_error& ex)
    {
        ++r.err;
        printError("", ex.code().message() + fsExceptionPath(ex));
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

            createdOutDir = fs::create_directories(outf.parent_path());

            if (fs::exists(outf))
            {
                if (fs::equivalent(inf, outf)) throw runtime_error("in and out files are the same");
            }

            /*if copy:
                if forced: r += !fs::copy_file(inf, outf, fs::copy_options::overwrite_existing)
                else: r += !fs::copy_file(inf, outf, fs::copy_options::update_existing)
            else*/
            {
                lineEnding ile = detectLineEnding(inf);

                if (ile == lineEnding::error)
                {
                    ++r.warn;
                    printWarning(ewiFile, "Unable to determine line ending, assuming LF");
                    ile = lineEnding::LF;
                }

                if (ile == lineEnding::LF) r += caterpillarProc(inf, outf, job, ewiFile);
                else
                {
                    fs::path tmpProcDir(outf.parent_path() / ".potorooTemp");
                    fs::path infLF(tmpProcDir / (inf.filename().string() + ".infLF"));
                    fs::path outfLF(tmpProcDir / (outf.filename().string() + ".outfLF"));

                    fs::create_directories(tmpProcDir);

                    string errMsg;
                    bool deleteTmpProcDir = true;

                    if (convertLineEnding(inf, ile, infLF, lineEnding::LF, errMsg) == 0)
                    {
                        r += caterpillarProc(infLF, outfLF, job, ewiFile);

                        if (r.err == 0)
                        {
                            if (convertLineEnding(outfLF, lineEnding::LF, outf, ile, errMsg) != 0)
                            {
                                deleteTmpProcDir = false;

                                ++r.warn;
                                printWarning("convert line ending", errMsg);
                            }
                        }
                    }
                    else
                    {
                        ++r.err;
                        printError("convert line ending", errMsg);
                    }

                    if (deleteTmpProcDir) fs::remove_all(tmpProcDir);
                }
            }

            if (r.err > 0) r += rmOut(outf, ewiFile, createdOutDir);
        }
        catch (fs::filesystem_error& ex)
        {
            ++r.err;

#if PRJ_DEBUG
            printError(ewiFile, ex.what());
#else
            printError(ewiFile, ex.code().message() + fsExceptionPath(ex));
#endif
    }
        catch (exception& ex)
        {
            ++r.err;
            printError(ewiFile, ex.what());
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

    return r;
}

Result potoroo::processJobs(const std::vector<Job>& jobs) noexcept
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
    Result lastR;

    for (size_t i = 0; i < jobs.size(); ++i)
    {
        Job j = jobs[i];

        if (lastR > 0) cout << "\n";

        cout << "process " << j << endl;

        lastR = processJob(j);
        pr += lastR;
    }

    return pr;
}
