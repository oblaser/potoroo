/*!

\author         Oliver Blaser
\date           15.02.2021
\copyright      GNU GPLv3 - Copyright (c) 2021 Oliver Blaser

*/

#include <iostream>

#include "arg.h"
#include "project.h"

using namespace std;
using namespace potoroo;



namespace
{
    inline bool argProc_cond(const ArgList& args, int n)
    {
        return (
            (args.count() == n) ||
            ((args.count() == (n + 1)) && args.contains(argType::forceJf))
            );
    }

    inline bool argProcJF_cond_in(const ArgList& args)
    {
#if PRJ_DEBUG
        if ((args.count(argType::inFile) == 1) && !args.get(argType::inFile).isValid())
        {
            string s = args.dbgDump();
            int __dbg = 0;
        }
#endif

        return ((args.count(argType::inFile) == 1) && args.get(argType::inFile).isValid());
    }

    inline bool argProcJF_cond_out(const ArgList& args)
    {
        return (
            ((args.count(argType::outDir) == 0) && (args.count(argType::outFile) == 1) && args.get(argType::outFile).isValid()) ||
            ((args.count(argType::outFile) == 0) && (args.count(argType::outDir) == 1) && args.get(argType::outDir).isValid())
            );
    }

    inline bool argProcJF_cond_tag(const ArgList& args)
    {
        return (
            (args.count(argType::tag) == 0) ||
            ((args.count(argType::tag) == 1) && args.get(argType::tag).isValid())
            );
    }

    inline bool argProcJF_cond_misc(const ArgList& args, string& errMsg)
    {
        int cond = 0;
        int err = 0;
        
        if (args.count(argType::jobFile) == 0) cond |= (1 << 0);
        else
        {
            errMsg += argStr_jf + " not supported inside a jobfile";
            ++err;
        }

        if (!args.containsInvalid()) cond |= (1 << 1);
        else
        {
            if (err) errMsg += " | ";
            errMsg += "invalid argument";
            ++err;
        }

        return (cond == 0x03);
    }
}




potoroo::Arg::Arg()
    : validity(false), type(argType::argType_invalid)
{
}

potoroo::Arg::Arg(const std::string& arg)
    : validity(false), type(argType::argType_invalid)
{
    if (arg == argStr_jf) type = argType::jobFile;
    else if (arg == argStr_if) type = argType::inFile;
    else if (arg == argStr_of) type = argType::outFile;
    else if (arg == argStr_od) type = argType::outDir;
    else if (arg == argStr_tag) type = argType::tag;
    else if (arg == argStr_forceJf) type = argType::forceJf;
    else if (arg == argStr_wError) type = argType::wError;
    else if ((arg == argStr_help) || (arg == argStr_help_alt)) type = argType::help;
    else if ((arg == argStr_version) || (arg == argStr_version_alt)) type = argType::version;
    else type = argType::argType_invalid;

    if (hasValue() || (type == argType::argType_invalid)) validity = false;
    else validity = true;
}

argType potoroo::Arg::getType() const
{
    return type;
}

std::string potoroo::Arg::getValue() const
{
    return value;
}

void potoroo::Arg::setValue(const std::string& value)
{
    if (type == argType::argType_invalid) validity = false;
    else
    {
        this->value = value;
        validity = true;
    }
}

bool potoroo::Arg::hasValue() const
{
    bool result = true;

    if ((type == argType::wError) ||
        (type == argType::forceJf) ||
        (type == argType::help) ||
        (type == argType::version))
    {
        result = false;
    }

    return result;
}

bool potoroo::Arg::isValid() const
{
    return validity;
}

#if PRJ_DEBUG
std::string potoroo::Arg::dbgType() const
{
    if (type == argType::jobFile) return "jobFile";
    else if (type == argType::inFile) return "inFile";
    else if (type == argType::outFile) return "outFile";
    else if (type == argType::outDir) return "outDir";
    else if (type == argType::tag) return "tag";
    else if (type == argType::forceJf) return argStr_forceJf;
    else if (type == argType::wError) return "wError";
    else if (type == argType::help) return "help";
    else if (type == argType::version) return "version";
    else return "x";
}
#endif



potoroo::ArgList::ArgList()
{
    clear();
}

void potoroo::ArgList::add(const Arg& arg)
{
    args.push_back(arg);
}

void potoroo::ArgList::clear()
{
    args.clear();
}

bool potoroo::ArgList::contains(argType at) const
{
    for (size_t i = 0; i < args.size(); ++i)
    {
        if (args[i].getType() == at) return true;
    }

    return false;
}

bool potoroo::ArgList::containsInvalid() const
{
    for (size_t i = 0; i < args.size(); ++i)
    {
        if (!args[i].isValid()) return true;
    }

    return false;
}

Arg potoroo::ArgList::get(argType at) const
{
    for (size_t i = 0; i < args.size(); ++i)
    {
        if (args[i].getType() == at) return args[i];
    }

    return Arg();
}

size_t potoroo::ArgList::count() const
{
    return args.size();
}

size_t potoroo::ArgList::count(argType at) const
{
    size_t cnt = 0;

    for (size_t i = 0; i < args.size(); ++i)
    {
        if (args[i].getType() == at) ++cnt;
    }

    return cnt;
}

#if PRJ_DEBUG
std::string potoroo::ArgList::dbgDump() const
{
    string s;

    for (int i = 0; i < args.size(); ++i)
    {
        s += to_string(i) + "  ";
        s += args[i].dbgType();
        if (args[i].hasValue()) s += " " + args[i].getValue();
        s += "\n";
    }

    return s;
}
#endif

ArgList potoroo::ArgList::parse(int argc, const char* const* argv)
{
    if (!argv) return ArgList();
    else
    {
        for (int i = 0; i < argc; ++i)
        {
            if (!(*(argv + i)))
            {
                argc = i;
                break;
            }
        }
    }

    ArgList list;

    for (int i = 1; i < argc; ++i)
    {
        Arg a(*(argv + i));

        if (a.hasValue())
        {
            ++i;
            if (i < argc) a.setValue(*(argv + i));
        }

        list.add(a);
    }


#if PRJ_DEBUG && 0
    cout << "\nArgList dump:\n" << list.dbgDump() << endl;
#endif


    return list;
}

ArgList potoroo::ArgList::parse(const char* args)
{
    if (!args) return ArgList();

    vector<vector<char>> list;

    for (size_t i = 0; args[i] != 0; /*increment is done in loop*/)
    {
        while ((args[i] == 0x09) || (args[i] == 0x20)) ++i;
        
        if (args[i] == '"')
        {
            vector<char> tmpVec;

            ++i;
            while ((args[i] != '"') && (args[i] != 0))
            {
                tmpVec.push_back(args[i]);
                ++i;
            }
            if (args[i] == '"') ++i;

            tmpVec.push_back(0);
            list.push_back(tmpVec);
        }
        else if (args[i] != 0)
        {
            vector<char> tmpVec;

            while ((args[i] != 0x09) && (args[i] != 0x20) && (args[i] != 0))
            {
                tmpVec.push_back(args[i]);
                ++i;
            }

            tmpVec.push_back(0);
            list.push_back(tmpVec);
        }
    }

    int argc = list.size() + 1;
    char** argv = new char* [argc];

    // skip first argument because parse(argc, argv) assumes a C standart
    // format in which the first arg is the binarys filename
    *argv = new char[1];
    **argv = 0;

#if PRJ_DEBUG && 0
    cout << "parse args from string\n" << args << endl;
#define ___DBG_ARG_PARSE_DUMP (1)
#endif

    for (int i = 1; i < argc; ++i)
    {
        *(argv + i) = new char[list[i - 1].size()];

        for (int j = 0; j < list[i - 1].size(); ++j) *((*(argv + i)) + j) = list[i - 1][j];
        //copy(list[i - 1].begin(), list[i - 1].end(), *(argv + i));

#if PRJ_DEBUG
#ifdef ___DBG_ARG_PARSE_DUMP
        cout << i << "  \"" << *(argv + i) << "\"" << endl;;
#endif
#endif
    }

    ArgList result = potoroo::ArgList::parse(argc, argv);

    for (int i = 0; i < argc; ++i) delete[](*(argv + i));
    delete[] argv;

    return result;
}




// -Werror is eighter present or not, no checks required.

argProcResult potoroo::argProc(ArgList& args)
{
    if (args.contains(argType::help)) return argProcResult::printHelp;
    if (args.contains(argType::version)) return argProcResult::printVersion;

    if (argProc_cond(args, 0))
    {
        Arg defaultJobFile(argStr_jf);
        defaultJobFile.setValue("./potorooJobs");
        args.add(defaultJobFile);
    }

    if (args.contains(argType::jobFile))
    {
        if (argProc_cond(args, 1) && (args.get(argType::jobFile).isValid())) return argProcResult::loadFile;
        else return argProcResult::error;
    }

    string jfErrMsg = "";
    return potoroo::argProcJF(args, jfErrMsg);
}

argProcResult potoroo::argProcJF(const ArgList& args, std::string& errMsg)
{
    int cond = 0;
    int err = 0;

    if (argProcJF_cond_in(args)) cond |= (1 << 0);
    else
    {
        errMsg += "invalid input";
        ++err;
    }

    if (argProcJF_cond_out(args)) cond |= (1 << 1);
    else
    {
        if (err) errMsg += " | ";
        errMsg += "invalid output";
        ++err;
    }

    if (argProcJF_cond_tag(args)) cond |= (1 << 2);
    else
    {
        if (err) errMsg += " | ";
        errMsg += "invalid tag";
        ++err;
    }

    string errMsgMisc = "";

    if (argProcJF_cond_misc(args, errMsgMisc)) cond |= (1 << 3);
    else
    {
        if (err) errMsg += " | ";
        errMsg += errMsgMisc;
        ++err;
    }


    if(cond == 0x0F)
    {
        return argProcResult::process;
    }

    return argProcResult::error;
}
