/*!

\author         Oliver Blaser
\date           15.02.2021
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
    const int rbSize = 200;
#else
    const int rbSize = 100 * 1024; // 100k
#endif
    const int rbHeadroom = 1024;
    const int rbSecRes = project::customTagMaxLen + 3;



    int readsome(ifstream& ifs, uint8_t* const pData, const string& ewiFile)
    {
        int nRead;

        try
        {
            nRead = 0;

            // cppreference.com says we cant trust ifstream::readsome()
            while (nRead < rbSize)
            {
                try { *(pData + nRead) = (char)ifs.get(); }
                catch (...) { break; }

                if (!ifs.good()) break;

                ++nRead;
            }

            // -1 = 0xFF is an invalid UTF-8 char and can be used as marking
            for (int i = 0; i < rbSecRes; ++i) pData[nRead + i] = -1;
        }
        catch (ios::failure& ex)
        {
            printDbg(ewiFile + ": IO", ex.what());
            nRead = -1;
        }
        catch (exception& ex)
        {
            printDbg(ewiFile, ex.what());
            nRead = -1;
        }
        catch (...)
        {
            printDbg(ewiFile, "unknown");
            nRead = -1;
        }

        return nRead;
    }

    Result ringBuffProc(const fs::path& inf, const fs::path& outf, const Job& job, const string& ewiFile)
    {
        Result r;

        vector<uint8_t> rb;

        const uint8_t* const pHR = rb + rbSize;
        const uint8_t* const pEnd = rb + rbSize + rbHeadroom;
        const uint8_t* p;
        const uint8_t* pMax;

        ifstream ifs;
        ofstream ofs;

        ifs.exceptions(ios::failbit | ios::badbit | ios::eofbit);
        ofs.exceptions(ios::failbit | ios::badbit | ios::eofbit);

        ifs.open(inf, ios::in | ios::binary);
        ofs.open(outf, ios::out | ios::binary);

        bool processing = true;
        bool firstRead = true;
        bool eof = false;

        while (processing)
        {
            int nRead = readsome(ifs, rb, ewiFile);

            if (nRead < rbSize) eof = true;

            p = rb;
            pMax = rb + nRead;

            if (firstRead)
            {
                firstRead = false;

                // UTF BOM check
                if (rb[0] == 0xFe && rb[1] == 0xFF) throw runtime_error("invalid file (UTF-16 BE encoded)");
                if (rb[0] == 0xFF && rb[1] == 0xFe) throw runtime_error("invalid file (UTF-16 LE encoded)");
                if (rb[0] == 0x00 && rb[1] == 0x00 && rb[2] == 0xFe && rb[3] == 0xFF) throw runtime_error("invalid file (UTF-32 BE encoded)");
                if (rb[0] == 0xFF && rb[1] == 0xFe && rb[2] == 0x00 && rb[3] == 0x00) throw runtime_error("invalid file (UTF-32 LE encoded)");

                // UTF-8 BOM is copied like every other UTF-8 byte
            }




            // process

            while (p < pMax)
            {
                ofs.put(*p);
                ++p;
            }

            for (int i = 0; i < 5; ++i) ofs << (char)0xE2 << (char)0x96 << (char)0x88;



            if (eof) processing = false;
        }

        ifs.close();
        ofs.close();

        return r;
    }

    Result rmOut(const fs::path& outf, const string& ewiFile, bool dir)
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

potoroo::Result potoroo::processJob(const Job& job)
{
    Result pr;
    string ewiFile;

#if !PRJ_DEBUG
    printInfo("processor", "not implemented, nothing done");
    return 1;
#endif

    try { ewiFile = fs::path(job.getInputFile()).filename().string(); }
    catch (...) { ewiFile = job.getInputFile(); }

    if (job.warningAsError()) printInfo(ewiFile, "\"Werror\" not yet implemented, has no effect");

    try
    {
        fs::path inf(job.getInputFile());
        fs::path outf(job.getOutputFile());

        bool createdOutDir = fs::create_directories(outf.parent_path());

        if (fs::exists(outf))
        {
            if (fs::equivalent(inf, outf)) throw runtime_error("in and out files are the same");
        }

        pr += ringBuffProc(inf, outf, job, ewiFile);

        if (pr.nErr > 0) pr += rmOut(outf, ewiFile, createdOutDir);
    }
    catch (fs::filesystem_error& ex)
    {
        ++pr.nErr;

#if PRJ_DEBUG
        printError(ewiFile, ex.what());
#else
        printError(ewiFile, ex.code().message() + fsExceptionPath(ex));
#endif
    }
    catch (exception& ex)
    {
        ++pr.nErr;
        printError(ewiFile, ex.what());
    }
    catch (...)
    {
        ++pr.nErr;
        printError(ewiFile, "unknown");
    }

    return pr;
}

potoroo::Result potoroo::processJobs(const std::vector<Job>& jobs)
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
