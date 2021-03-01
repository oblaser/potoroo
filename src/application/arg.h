/*!

\author         Oliver Blaser
\date           14.02.2021
\copyright      GNU GPLv3 - Copyright (c) 2021 Oliver Blaser

*/

#ifndef _CLIARG_H_
#define _CLIARG_H_

#include <string>
#include <vector>

#include "project.h"

namespace potoroo
{
    const std::string argStr_jf = "-jf";
    const std::string argStr_if = "-if";
    const std::string argStr_of = "-of";
    const std::string argStr_od = "-od";
    const std::string argStr_tag = "-t";
    const std::string argStr_forceJf = "--force-jf";
    const std::string argStr_wError = "-Werror";
    const std::string argStr_help = "-h";
    const std::string argStr_help_alt = "--help";
    const std::string argStr_version = "-v";
    const std::string argStr_version_alt = "--version";

    enum class argType
    {
        argType_invalid,

        help,
        version,
        jobFile,
        inFile,
        outDir,
        outFile,
        tag,
        forceJf,
        wError
    };

    enum class argProcResult
    {
        error,

        loadFile,
        process,
        printVersion,
        printHelp
    };

    class Arg
    {
    public:
        Arg();
        Arg(const std::string& arg);

        argType getType() const;
        std::string getValue() const;

        void setValue(const std::string& value);

        bool hasValue() const;
        bool isValid() const;

#if PRJ_DEBUG
        std::string dbgType() const;
#endif

    private:
        argType type;
        std::string value;
        bool validity;
    };

    class ArgList
    {
    public:
        ArgList();

        void add(const Arg& arg);
        void clear();
        bool contains(argType at) const;
        bool containsInvalid() const;
        Arg get(argType at) const;
        size_t count() const;
        size_t count(argType at) const;

#if PRJ_DEBUG
        std::string dbgDump() const;
#endif

    private:
        std::vector<Arg> args;

    public:
        static ArgList parse(int argc, const char* const* argv);
        static ArgList parse(const char* args);
    };

    argProcResult argProc(ArgList& args);
    argProcResult argProcJF(const ArgList& args, std::string& errMsg);
}

#endif // _CLIARG_H_
