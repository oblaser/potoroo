/*!

\author         Oliver Blaser
\date           28.04.2021
\copyright      GNU GPLv3 - Copyright (c) 2022 Oliver Blaser

*/

#include <cmath>
#include <filesystem>
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

namespace fs = std::filesystem;

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
        cout << "  potoroo -if FILE (-od DIR | -of FILE) [options]" << endl;
        cout << endl;
        cout << endl;
        cout << "Arguments:" << endl;
        cout << left << setw(lw) << "  " + argStr_jf + " FILE" << "specify a jobfile" << endl;
        cout << left << setw(lw) << "  " + argStr_forceJf << "force jobfile to be processed even if errors occured while parsing it" << endl;
        cout << left << setw(lw) << "  " + argStr_if + " FILE" << "input file" << endl;
        cout << left << setw(lw) << "  " + argStr_of + " FILE" << "output file" << endl;
        cout << left << setw(lw) << "  " + argStr_od + " DIR" << "output directory (same filename)" << endl;
        cout << left << setw(lw) << "  " + argStr_tag + " TAG" << "specify the tag" << endl;
        cout << left << setw(lw) << "  " + argStr_wError << "handles warnings as errors (in processor, the jobfile parser is unaffected)" << endl;
        cout << left << setw(lw) << "  " + argStr_wSup + " LIST" << "suppresses the reporting of the specified warnings. LIST is a comma separated" << endl;
        cout << left << setw(lw) << "  " << "list of integer warning IDs. (in processor, the jobfile parser is unaffected)" << endl;
        cout << left << setw(lw) << "  " + argStr_wrErrLn + " TEXT" << "     instead of deleting the output file on error, writes TEXT to it" << endl;
        cout << left << setw(lw) << "  " + argStr_copy << "copy, replaces the existing file only if it is older than the input file" << endl;
        cout << left << setw(lw) << "  " + argStr_copyow << "copy, overwrites the existing file" << endl;
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
        cout << endl;
        cout << "Website:" << endl << "  <https://github.com/oblaser/potoroo>" << endl;
        cout << endl;
    }

    void printVersion()
    {
        cout << "potoroo " << PRJ_VERSION;

#if PRJ_VERSION_PRERELEASE
        cout << " " << sgr(SGRFGC_BRIGHT_MAGENTA) << "pre-release" << sgr(SGR_RESET);

#define VERSION_PRERELEASE_TYPE 0

#if (VERSION_PRERELEASE_TYPE == 1)
        cout << " built tag " << "TAG";
#elif (VERSION_PRERELEASE_TYPE == 2)
        cout << " built commit " << "COMMIT-HASH";
#else
        cout << " built at " << __DATE__ << " " << __TIME__;
#endif
#endif

#if PRJ_DEBUG
        cout << "   " << sgr(SGRFGC_BRIGHT_RED) << "DEBUG" << sgr(SGR_RESET);
#endif

        cout << endl << endl;
        cout << "project page: <https://github.com/oblaser/potoroo>" << endl;
        cout << endl;
        cout << "Copyright (c) 2022 Oliver Blaser." << endl;
        cout << "License: GNU GPLv3 <http://gnu.org/licenses/>." << endl;
        cout << "This is free software. There is NO WARRANTY." << endl;
    }

    void printProcessJobsResult(const Result& pr, const vector<Job>& jobs, const vector<bool>& success)
    {
        size_t nJobs;
        size_t nSucceeded = 0;
        size_t nInvalid = 0;

        if (jobs.size() != success.size())
        {
            printEWI("internal", "fatal! " + string(__FILENAME__) + ":" + to_string(__LINE__) + ": jobs.size() != success.size()", 0, 0, 0, 0);
        }

        vector<bool> succ(success);
        while (succ.size() < jobs.size()) succ.push_back(false);

        for (size_t i = 0; i < jobs.size(); ++i)
        {
            if (!jobs[i].isValid()) ++nInvalid;
            if (succ[i]) ++nSucceeded;
        }

        nJobs = jobs.size() - nInvalid;



        cout << "========";

        cout << "  " << sgr(SGRFGC_BRIGHT_WHITE);
        cout << nSucceeded << "/" << nJobs;
        if (nInvalid) cout << "(" << jobs.size() << ")";
        cout << sgr(SGR_RESET) << " succeeded";

        cout << ", ";
        if (pr.err) cout << sgr(SGRFGC_BRIGHT_RED);
        cout << pr.err;
        if (pr.err) cout << sgr(SGR_RESET);
        cout << " error";
        if (abs(pr.err) != 1) cout << "s";

        cout << ", ";
        if (pr.warn) cout << sgr(SGRFGC_BRIGHT_YELLOW);
        cout << pr.warn;
        if (pr.warn) cout << sgr(SGR_RESET);
        cout << " warning";
        if (abs(pr.warn) != 1) cout << "s";

        cout << " ========" << endl;
    }
}





int main(int argc, char** argv)
{
    int result = rcInit;

    ArgList args = ArgList::parse(argc, argv);

#if PRJ_DEBUG && 1
    Arg a;

#if 1 // jobfile
    a = Arg("-jf");
#ifdef _DEBUG_ECLIPSE
    a.setValue("../../test/system/processor/potorooJobs");
#else
    //a.setValue("../../../test/system/stressTest_jobfileParser.potorooJobs");
    //a.setValue("../../../test/system/lineEndings/potorooJobs");
    //a.setValue("../../../test/system/include/potorooJobs");
    a.setValue("../../../test/system/processor/potorooJobs");
#endif
    args.add(a);
#endif


#if 0 // ./potoroo -if ../../test/system/processor/js/index.js -od .
    a = Arg("-if");
#ifdef _DEBUG_ECLIPSE
    a.setValue("../../test/system/processor/js/index.js");
#else
    a.setValue("../../../test/system/processor/js/index.js");
#endif
    args.add(a);
    a = Arg("-od");
    a.setValue(".");
    args.add(a);

    //args.add(Arg("-Werror"));
    //args.add(Arg("--copy-ow"));
#endif


    //args.add(Arg("--force-jf"));

    //args.add(Arg("-v"));
    args.add(Arg("-h"));
#endif

    ArgProcResult apr = argProc(args);

    if (apr == ArgProcResult::loadFile)
    {
        string jobfile = args.get(ArgType::jobFile).getValue();
        vector<Job> jobs;
        Result pr = Job::parseFile(jobfile, jobs);

        if ((pr.err == 0) ||
            (args.contains(ArgType::forceJf) && (pr.err > 0)) // only force if no file IO error
            )
        {
            Result cwdr = changeWD(fs::path(jobfile).parent_path());
            pr += cwdr;

            if (cwdr.err == 0)
            {
                if (pr.err > 0) cout << endl;

                vector<bool> success(jobs.size(), false);
                pr += processJobs(jobs, success);

                if (pr.err) result = rcNErrorBase + pr.err;
                else result = rcOK;

                printProcessJobsResult(pr, jobs, success);
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
    }
    else if (apr == ArgProcResult::process)
    {
        Job job = Job::parseArgs(args);

        Result pr = processJob(job);

        if (pr.err) result = rcNErrorBase + pr.err;
        else result = rcOK;

        if ((pr.err != 0) || (pr.warn != 0)) cout << "\n   " << pr << "\n" << endl;
    }
    else if (apr == ArgProcResult::printHelp)
    {
        printHelp();
        result = rcOK;
    }
    else if (apr == ArgProcResult::printVersion)
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
    int ___dbg_getc = getc(stdin);
#endif

    if (result > rcMax) result = rcMax;

    return result;
}
