/*!

\author         Oliver Blaser
\date           17.02.2021
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
    void printEWI(const std::string& file, const std::string& text, size_t line, size_t col, int ewi)
    {
        cout << sgr(SGRFGC_BRIGHT_WHITE) << file << sgr(SGR_RESET) << ":";
        if (line > 0) cout << sgr(SGRFGC_BRIGHT_WHITE) << line << sgr(SGR_RESET) << ":";
        if (col > 0)
        {
            if (line <= 0) cout << ":";
            cout << sgr(SGRFGC_BRIGHT_WHITE) << col << sgr(SGR_RESET) << ":";
        }
        cout << " ";

        if (ewi == 0) cout << sgr(SGRFGC_BRIGHT_RED, SGR_BOLD) << "error:";
        else if (ewi == 1) cout << sgr(SGRFGC_BRIGHT_YELLOW, SGR_BOLD) << "warning:";
        else if (ewi == 2) cout << sgr(SGRFGC_BRIGHT_CYAN, SGR_BOLD) << "info:";
        else if (ewi == -1) cout << sgr(SGRFGC_BRIGHT_MAGENTA, SGR_BOLD) << "debug:";
        else  cout << sgr(SGRFGC_BRIGHT_MAGENTA, SGR_BOLD) << "#ERROR# " << sgr(SGRFGC_BRIGHT_RED, SGR_BOLD) << "error:";


        cout << sgr(SGR_RESET) << " " << text << endl;
    }

    void printError(const std::string& file, const std::string& text, size_t line = 0, size_t col = 0)
    {
        printEWI(file, text, line, col, 0);
    }

    void printWarning(const std::string& file, const std::string& text, size_t line = 0, size_t col = 0)
    {
        printEWI(file, text, line, col, 1);
    }

    void printInfo(const std::string& file, const std::string& text, size_t line = 0, size_t col = 0)
    {
        printEWI(file, text, line, col, 2);
    }

#if PRJ_DEBUG
    void printDbg(const std::string& file, const std::string& text, size_t line = 0, size_t col = 0)
    {
        printEWI(file, text, line, col, -1);
    }
#endif












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
    //! @param ewiFile 
    //! @return Number of read bytes
    //! 
    //! Aligned to new lines and thus does neither chop UTF-8 chunks nor tags.
    //! 
    size_t readsome(ifstream& ifs, vector<fileIOt>& data, const string& ewiFile)
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

    Result caterpillarProc(const fs::path& inf, const fs::path& outf, const Job& job, const string& ewiFile)
    {
        Result r;

        vector<fileIOt> pb; // process buffer
        vector<fileIOt> ob; // out buffer

        const string tag = job.getTag() + " ";

        size_t pLine = 0; // 0 to detect first read
        size_t pCol = 1;  // processed line/column used to display error, threrefore starting with 1
        bool eof = false;

        ifstream ifs;
        ofstream ofs;

        ifs.exceptions(ios::failbit | ios::badbit | ios::eofbit);
        ofs.exceptions(ios::failbit | ios::badbit | ios::eofbit);

        ifs.open(inf, ios::in | ios::binary);
        ofs.open(outf, ios::out | ios::binary);

        while (!eof)
        {
            pb.clear();
            size_t nRead = readsome(ifs, pb, ewiFile);
            const fileIOt* const pPb = pb.data();
            const fileIOt* p = pPb;

            if ((nRead < pbSizeMin) || ifs.eof()) eof = true;

            // only on first read
            if (pLine == 0)
            {
                pLine = 1;

                // UTF BOM check
                if (pb.size() >= 4)
                {
                    if (pb[0] == static_cast<fileIOt>(0x00) && pb[1] == static_cast<fileIOt>(0x00) &&
                        pb[2] == static_cast<fileIOt>(0xFe) && pb[3] == static_cast<fileIOt>(0xFF))
                    {
                        throw runtime_error("encoding not supported: UTF-32 BE");
                    }
                    if (pb[0] == static_cast<fileIOt>(0xFF) && pb[1] == static_cast<fileIOt>(0xFe) &&
                        pb[2] == static_cast<fileIOt>(0x00) && pb[3] == static_cast<fileIOt>(0x00))
                    {
                        throw runtime_error("encoding not supported: UTF-32 LE");
                    }
                }

                if (pb.size() >= 2)
                {
                    if (pb[0] == static_cast<fileIOt>(0xFe) && pb[1] == static_cast<fileIOt>(0xFF)) throw runtime_error("encoding not supported: UTF-16 BE");
                    if (pb[0] == static_cast<fileIOt>(0xFF) && pb[1] == static_cast<fileIOt>(0xFe)) throw runtime_error("encoding not supported: UTF-16 LE");
                }

                // UTF-8 BOM is copied like every other UTF-8 byte
            }






            // process
            while (p < (pPb + nRead))
            {
                ob.clear();

                // skip whitespace
                while ((p < (pPb + nRead)) &&
                    ((*p == 0x09) || (*p == 0x20)))
                {
                    ob.push_back(*p);
                    ++p;
                }


                if (p < (pPb + nRead - tag.length()))
                {
                    if (tag.compare(0, tag.length(), p, tag.length()) == 0)
                    {
                        ob.push_back('#');
                        ob.push_back('#');
                        ob.push_back('#');
                    }
                }


                ob.push_back(*p);
                ++p;


                ofs.write(ob.data(), ob.size());
            }

#if PRJ_DEBUG
            for (int i = 0; i < 5; ++i) ofs << (char)0xE2 << (char)0x96 << (char)0x88; // full block
#endif
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
            ++r.nWarn;
            printWarning(ewiFile, rmFileErrorMsg + ":\n" + ex.code().message() + fsExceptionPath(ex));
        }
        catch (exception& ex)
        {
            ++r.nWarn;
            printWarning(ewiFile, rmFileErrorMsg + ":\n" + ex.what());
        }
        catch (...)
        {
            ++r.nWarn;
            printWarning(ewiFile, rmFileErrorMsg);
        }

        if (dir)
        {
            const string rmDirErrorMsg = "output directory not deleted";

            try { fs::remove(outf.parent_path()); }
            catch (fs::filesystem_error& ex)
            {
                ++r.nWarn;
                printWarning(ewiFile, rmDirErrorMsg + ":\n" + ex.code().message() + fsExceptionPath(ex));
            }
            catch (exception& ex)
            {
                ++r.nWarn;
                printWarning(ewiFile, rmDirErrorMsg + ":\n" + ex.what());
            }
            catch (...)
            {
                ++r.nWarn;
                printWarning(ewiFile, rmDirErrorMsg);
            }
        }

        return r;
    }
}

potoroo::Result potoroo::processJob(const Job& job) noexcept
{
    Result r;
    fs::path inf;
    fs::path outf;
    string ewiFile;
    bool createdOutDir = false;

#if !PRJ_DEBUG
    printInfo("processor", "not implemented, nothing done");
    return 1;
#endif

    try
    {
        inf = fs::path(job.getInputFile());
        outf = fs::path(job.getOutputFile());
    }
    catch (fs::filesystem_error& ex)
    {
        ++r.nErr;
        printError("", ex.code().message() + fsExceptionPath(ex));
    }
    catch (...)
    {
        ++r.nErr;
        printError("", "invalid in or out filename");
    }

    if (r.nErr == 0)
    {
        try { ewiFile = inf.filename().string(); }
        catch (...) { ewiFile = job.getInputFile(); }

        if (job.warningAsError()) printInfo(ewiFile, "\"Werror\" not yet implemented, has no effect");

        try
        {
            if (!fs::exists(inf)) throw runtime_error("file does not exist");

            createdOutDir = fs::create_directories(outf.parent_path());

            if (fs::exists(outf))
            {
                if (fs::equivalent(inf, outf)) throw runtime_error("in and out files are the same");
            }

            // if copy:
            //     if forced: fs::copy_file(inf, outf, fs::copy_options::overwrite_existing)
            //     else: fs::copy_file(inf, outf, fs::copy_options::update_existing)
            // else: caterpillarProc()
            r += caterpillarProc(inf, outf, job, ewiFile);

            if (r.nErr > 0) r += rmOut(outf, ewiFile, createdOutDir);
        }
        catch (fs::filesystem_error& ex)
        {
            ++r.nErr;

#if PRJ_DEBUG
            printError(ewiFile, ex.what());
#else
            printError(ewiFile, ex.code().message() + fsExceptionPath(ex));
#endif
        }
        catch (exception& ex)
        {
            ++r.nErr;
            printError(ewiFile, ex.what());
        }
        catch (...)
        {
            ++r.nErr;
            printError(ewiFile, "unknown");
        }
    }

    return r;
}

potoroo::Result potoroo::processJobs(const std::vector<Job>& jobs) noexcept
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

    for (size_t i = 0; i < jobs.size(); ++i)
    {
        Job j = jobs[i];

        cout << "process ";
        cout << j.getInputFile();
        cout << " " << j.getOutputFile();
        cout << " " << j.getTag();
        cout << (j.warningAsError() ? " Werror" : "");
        cout << endl;

        pr += processJob(j);
    }

    return pr;
}
