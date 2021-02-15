/*!

\author         Oliver Blaser
\date           14.02.2021
\copyright      GNU GPLv3 - Copyright (c) 2021 Oliver Blaser

*/

#include <filesystem>
#include <iomanip>
#include <iostream>

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

        if (ewi==0) cout << sgr(SGRFGC_BRIGHT_RED, SGR_BOLD) << "error:";
        else if(ewi == 1) cout << sgr(SGRFGC_BRIGHT_YELLOW, SGR_BOLD) << "warning:";
        else if(ewi == 2) cout << sgr(SGRFGC_BRIGHT_CYAN,SGR_BOLD) << "info:";
        else  cout << sgr(SGRFGC_BRIGHT_MAGENTA, SGR_BOLD) << "#internal error# " << sgr(SGRFGC_BRIGHT_RED, SGR_BOLD) << "error:";


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

    Result ringBuffProc()
    {
        return 1;
    }
}

potoroo::Result potoroo::processJob(const Job& job)
{
    Result pr;
    string ewiFile;

#if(!PRJ_DEBUG)
    cout << sgr(SGRFGC_BRIGHT_MAGENTA) << "   processor not yet implemented" << sgr(SGR_RESET) << endl;
    ++pr.nErr;
#endif

    try { ewiFile = fs::path(job.getInputFile()).filename().string(); }
    catch (...) { ewiFile = job.getInputFile(); }

    if (job.warningAsError()) printInfo(ewiFile, "\"Werror\" not yet implemented, has no effect");

    // open streams

    // process
    pr += ringBuffProc();

    // move files

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

    Result pr;

    for (size_t i = 0; i < jobs.size(); ++i)
    {
        Job j = jobs[i];

#if PRJ_DEBUG && 0
        if (i == 4) { j.setValidity(false); j.setErrorMsg("-== TEST ==-"); }
#endif

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
