/*!

\author         Oliver Blaser
\date           17.02.2021
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
        rcChangeWDErr,

        rcNErrorBase = 10,

        rcMax = 127 // to stay compatible with systems that treat the app return code as signed 8bit value
    };

    void printHelp()
    {
        const int lw = 17;
        const int lwTag = 13;
        const int lwTagStr = 9;

        cout << "Usage:" << endl;
        cout << "  potoroo [-jf FILE] [--force-jf]" << endl;
        cout << "  potoroo -if FILE (-od DIR | -of FILE) [-t TAG] [-Werror]" << endl;
        cout << endl;
        cout << endl;
        cout << "Arguments:" << endl;
        cout << left << setw(lw) << "  " + argStr_jf + " FILE" << "specify a jobfile" << endl;
        cout << left << setw(lw) << "  " + argStr_forceJf << "force jobfile to be processed even if errors occured while parsing it" << endl;
        cout << endl;
        cout << left << setw(lw) << "  " + argStr_if + " FILE" << "input file" << endl;
        cout << left << setw(lw) << "  " + argStr_of + " FILE" << "output file" << endl;
        cout << left << setw(lw) << "  " + argStr_od + " DIR" << "output directory (same filename)" << endl;
        cout << left << setw(lw) << "  " + argStr_tag + " TAG" << "specify the tag" << endl;
        cout << left << setw(lw) << "  " + argStr_wError << "handles warnings as errors (only in processor, the jobfile parser is" << endl;
        cout << left << setw(lw) << "  " << "unaffected by this flag).Results in not writing the output file if" << endl;
        cout << left << setw(lw) << "  " << "any warning occured" << endl;
        cout << endl;
        cout << left << setw(lw) << "  " + argStr_help + ", " + argStr_help_alt << "prints this help text" << endl;
        cout << left << setw(lw) << "  " + argStr_version + ", " + argStr_version_alt << "prints version info" << endl;
        cout << endl;
        cout << endl;
        cout << "TAG values:" << endl;
        cout << left << setw(lwTag) << "  cpp" << left << setw(lwTagStr) << tagCpp << "for scripts with C++ like comments" << endl;
        cout << left << setw(lwTag) << "  bash" << left << setw(lwTagStr) << tagBash << "for scripts with bash like comments" << endl;
        cout << left << setw(lwTag) << "  batch" << left << setw(lwTagStr) << tagBatch << "for batch scripts" << endl;
        cout << left << setw(lwTag) << "  custom:CT" << left << setw(lwTagStr) << "CT" << "to use a custom comment tag" << endl;
        cout << endl;
    }

    void printVersion()
    {
        cout << "potoroo " << PRJ_VERSION << endl;
        cout << endl;
        cout << "project page: <https://github.com/oblaser/potoroo>" << endl;
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
            cout << endl; // windows console ads en extra line automatically
#endif
        }
    }
}





int main(int argc, char** argv)
{
    int result = rcInit;

    ArgList args = ArgList::parse(argc, argv);

#if PRJ_DEBUG && 1
    Arg a;

    a = Arg("-jf");
    a.setValue("../../../test/system/stressTest_jobfileParser.potorooJobs");
    a.setValue("../../../test/system/processor/potorooJobs");
    args.add(a);

    args.add(Arg("--force-jf"));

    //args.add(Arg("-v"));
    //args.add(Arg("-h"));
#endif

    argProcResult apr = argProc(args);

    if (apr == argProcResult::loadFile)
    {
        string jobfile = args.get(argType::jobFile).getValue();
        vector<Job> jobs;
        Result pr = Job::parseFile(jobfile, jobs);

        if ((pr.nErr == 0) ||
            (args.contains(argType::forceJf) && (pr.nErr > 0)) // only force if no file IO error
            )
        {
            Result cwdr = changeWD(jobfile);
            pr += cwdr;

            if (cwdr.nErr == 0)
            {
                if (pr.nErr > 0) cout << endl;
                pr += processJobs(jobs);
                if (pr.nErr) result = rcNErrorBase + pr.nErr;
                else result = rcOK;
            }
            else
            {
                result = rcChangeWDErr;
            }
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
