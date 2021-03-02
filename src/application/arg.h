/*!

\author         Oliver Blaser
\date           01.03.2021
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
    const std::string argStr_copy = "--copy";
    const std::string argStr_copyow = "--copy-ow";
    const std::string argStr_help = "-h";
    const std::string argStr_help_alt = "--help";
    const std::string argStr_version = "-v";
    const std::string argStr_version_alt = "--version";

    enum class ArgType
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
        wError,
        copy,
        copyow
    };

    enum class ArgProcResult
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

        ArgType getType() const;
        std::string getValue() const;

        void setValue(const std::string& value);

        bool hasValue() const;
        bool isValid() const;

#if PRJ_DEBUG
        std::string dbgType() const;
#endif

    private:
        ArgType type;
        std::string value;
        bool validity;
    };

    class ArgList
    {
    public:
        ArgList();

        void add(const Arg& arg);
        void clear();
        bool contains(ArgType at) const;
        bool containsInvalid() const;
        Arg get(ArgType at) const;
        size_t count() const;
        size_t count(ArgType at) const;

#if PRJ_DEBUG
        std::string dbgDump() const;
#endif

    private:
        std::vector<Arg> args;

    public:
        static ArgList parse(int argc, const char* const* argv);
        static ArgList parse(const char* args);
    };

    ArgProcResult argProc(ArgList& args);
    ArgProcResult argProcJF(const ArgList& args, std::string& errMsg);
}

#endif // _CLIARG_H_
