/*!

\author         Oliver Blaser
\date           14.02.2021
\copyright      GNU GPLv3 - Copyright (c) 2021 Oliver Blaser

*/

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "project.h"
#include "application/arg.h"
#include "application/job.h"
#include "application/processor.h"
#include "middleware/cliTextFormat.h"

using namespace std;
using namespace cli;
using namespace potoroo;

namespace
{
    enum returnCode
    {
        rcInit = -1,

        rcOK = 0,
        rcInvArg,
        rcJobFileErr,

        rcNErrorBase = 10,

        rcMax = 127 // to stay compatible with systems that treat the app return code as signed 8bit value
    };

    void printHelp()
    {
        const int lw = 17;

        cout << "Usage:" << endl;
        cout << "  potoroo [-jf FILE] [--force-jf]" << endl;
        cout << "  potoroo -if FILE (-od DIR | -of FILE) [-t TAG] [-Werror]" << endl;
        cout << endl;
        cout << "Arguments:" << endl;
        cout << left << setw(lw) << "  " + argStr_jf+" FILE" << "description" << endl;
        cout << left << setw(lw) << "  " + argStr_if+" FILE" << "description" << endl;
        cout << left << setw(lw) << "  " + argStr_of+" FILE" << "description" << endl;
        cout << left << setw(lw) << "  " + argStr_od +" DIR" << "description" << endl;
        cout << left << setw(lw) << "  " + argStr_tag+" TAG" << "description" << endl;
        cout << left << setw(lw) << "  " + argStr_forceJf << "description" << endl;
        cout << left << setw(lw) << "  " + argStr_wError << "description" << endl;
        cout << left << setw(lw) << "  " + argStr_help + ", " + argStr_help_alt << "prints this help text" << endl;
        cout << left << setw(lw) << "  " + argStr_version + ", " + argStr_version_alt << "prints version info" << endl;
        cout << endl;
    }

    void printVersion()
    {
        cout << "potoroo " << PRJ_VERSION << endl;
        cout << endl;
        cout << "Copyright (c) 2021 Oliver Blaser." << endl;
        cout << "License: GNU GPLv3 <http://gnu.org/licenses/>." << endl;
        cout << "This is free software. There is NO WARRANTY." << endl;
    }

    void printProcessorResult(const Result& pr)
    {
#if PRJ_DEBUG
        if (pr > 0)
#else
        if (pr > 10)
#endif
        {
            cout << "\n   " << pr << endl;

#ifndef PRJ_PLAT_WIN
            cout << endl; // windows console ads en extra line anyway
#endif
        }
    }
}

int main(int argc, char** argv)
{
    int result = rcInit;

    ArgList args = ArgList::parse(argc, argv);

#if PRJ_DEBUG && 0
    args.add(Arg("--force-jf"));
#endif

    argProcResult apr = argProc(args);

    if (apr == argProcResult::loadFile)
    {
        vector<Job> jobs;
        Result pr = Job::parseFile(args.get(argType::jobFile).getValue(), jobs);

        if ((pr.nErr == 0) ||
            (args.contains(argType::forceJf) && (pr.nErr > 0)) // only force if no file IO error
            )
        {
            if (pr.nErr > 0) cout << endl;
            pr += processJobs(jobs);
            if (pr.nErr) result = rcNErrorBase + pr.nErr;
            else result = rcOK;
        }
        else
        {
            result = rcJobFileErr;
        }

        printProcessorResult(pr);
    }
    else if (apr == argProcResult::process)
    {
        Result pr = processJob(Job::parseArgs(args));
        printProcessorResult(pr);
        if (pr.nErr) result = rcNErrorBase + pr.nErr;
        else result = rcOK;
    }
    else if (apr == argProcResult::printHelp)
    {
        printHelp();
        result = rcOK;
    }
    else if (apr == argProcResult::printVersion)
    {
        printVersion();
        result = rcOK;
    }
    else
    {
        cout << "invalid arguments" << endl;
        result = rcInvArg;
    }

#if PRJ_DEBUG && 1
    cout << "return " << result << endl;
    getc(stdin);
#endif

    if (result > rcMax) result = rcMax;

    return result;
}
