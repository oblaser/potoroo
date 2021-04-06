/*!

\author         Oliver Blaser
\date           06.04.2021
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
            ((args.count() == (n + 1)) && args.contains(ArgType::forceJf))
            );
    }

    inline bool argProcJF_cond_in(const ArgList& args)
    {
        return ((args.count(ArgType::inFile) == 1) && args.get(ArgType::inFile).isValid());
    }

    inline bool argProcJF_cond_out(const ArgList& args)
    {
        return (
            ((args.count(ArgType::outDir) == 0) && (args.count(ArgType::outFile) == 1) && args.get(ArgType::outFile).isValid()) ||
            ((args.count(ArgType::outFile) == 0) && (args.count(ArgType::outDir) == 1) && args.get(ArgType::outDir).isValid())
            );
    }

    inline bool argProcJF_cond_tag(const ArgList& args)
    {
        return (
            (args.count(ArgType::tag) == 0) ||
            ((args.count(ArgType::tag) == 1) && args.get(ArgType::tag).isValid())
            );
    }

    inline bool argProcJF_cond_misc(const ArgList& args, string& errMsg)
    {
        int cond = 0;
        int err = 0;

        if (args.count(ArgType::jobFile) == 0) cond |= (1 << 0);
        else
        {
            errMsg += argStr_jf + " not supported inside a jobfile";
            ++err;
        }

        if (!args.containsInvalid()) cond |= (1 << 1);
        else
        {
            if (err) errMsg += ", ";
            errMsg += "invalid argument";
            ++err;
        }

        if (((args.count(ArgType::copy) == 0) && (args.count(ArgType::copyow) == 0)) ||
            ((args.count(ArgType::copy) == 0) && (args.count(ArgType::copyow) == 1)) ||
            ((args.count(ArgType::copy) == 1) && (args.count(ArgType::copyow) == 0)))
        {
            cond |= (1 << 2);
        }
        else
        {
            if (err) errMsg += ", ";
            errMsg += "invalid copy arguments";
            ++err;
        }

        return (cond == 0x07);
    }
}




potoroo::Arg::Arg()
    : validity(false), type(ArgType::argType_invalid)
{
}

potoroo::Arg::Arg(const std::string& arg)
    : type(ArgType::argType_invalid)
{
    if (arg == argStr_jf) type = ArgType::jobFile;
    else if (arg == argStr_if) type = ArgType::inFile;
    else if (arg == argStr_of) type = ArgType::outFile;
    else if (arg == argStr_od) type = ArgType::outDir;
    else if (arg == argStr_tag) type = ArgType::tag;
    else if (arg == argStr_forceJf) type = ArgType::forceJf;
    else if (arg == argStr_wError) type = ArgType::wError;
    else if (arg == argStr_wSup) type = ArgType::wSup;
    else if (arg == argStr_copy) type = ArgType::copy;
    else if (arg == argStr_copyow) type = ArgType::copyow;
    else if ((arg == argStr_help) || (arg == argStr_help_alt)) type = ArgType::help;
    else if ((arg == argStr_version) || (arg == argStr_version_alt)) type = ArgType::version;
    else type = ArgType::argType_invalid;

    if (hasValue() || (type == ArgType::argType_invalid)) validity = false;
    else validity = true;
}

ArgType potoroo::Arg::getType() const
{
    return type;
}

std::string potoroo::Arg::getValue() const
{
    return value;
}

void potoroo::Arg::setValue(const std::string& value)
{
    if (type == ArgType::argType_invalid) validity = false;
    else
    {
        this->value = value;
        validity = true;
    }
}

bool potoroo::Arg::hasValue() const
{
    bool result = true;

    if ((type == ArgType::wError) ||
        (type == ArgType::copy) ||
        (type == ArgType::copyow) ||
        (type == ArgType::forceJf) ||
        (type == ArgType::help) ||
        (type == ArgType::version))
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
    if (type == ArgType::jobFile) return "jobFile";
    else if (type == ArgType::inFile) return "inFile";
    else if (type == ArgType::outFile) return "outFile";
    else if (type == ArgType::outDir) return "outDir";
    else if (type == ArgType::tag) return "tag";
    else if (type == ArgType::forceJf) return argStr_forceJf;
    else if (type == ArgType::wError) return "wError";
    else if (type == ArgType::wSup) return "wSup";
    else if (type == ArgType::help) return "help";
    else if (type == ArgType::version) return "version";
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

bool potoroo::ArgList::contains(ArgType at) const
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

Arg potoroo::ArgList::get(ArgType at) const
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

size_t potoroo::ArgList::count(ArgType at) const
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

    for (size_t i = 0; i < args.size(); ++i)
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

        string tmpArgStr(*(argv + i));

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

        for (size_t j = 0; j < list[i - 1].size(); ++j) *((*(argv + i)) + j) = list[i - 1][j];
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



int potoroo::wSupStrListToVector(std::vector<int>& list, const std::string& strList)
{
    const char* p = strList.c_str();
    const char* const pMax = p + strList.length();

    vector<string> tmpStrList;

    while (p < pMax)
    {
        string tmpStr = "";
        while ((p < pMax) && (*p != ','))
        {
            tmpStr += *p;
            ++p;
        }
        ++p; // skip the comma
        tmpStrList.push_back(tmpStr);
    }

    vector<int> tmpList;
    int r = 0;

    for (size_t i = 0; i < tmpStrList.size(); ++i)
    {
        try { tmpList.push_back(std::stoi(tmpStrList[i])); }
        catch (...)
        {
            r = 1;
            break;
        }
    }

    if (r == 0) list = vector<int>(tmpList);

    return r;
}



// -Werror is eighter present or not, no checks required.

ArgProcResult potoroo::argProc(ArgList& args)
{
    if (args.contains(ArgType::help)) return ArgProcResult::printHelp;
    if (args.contains(ArgType::version)) return ArgProcResult::printVersion;

    if (argProc_cond(args, 0))
    {
        Arg defaultJobFile(argStr_jf);
        defaultJobFile.setValue("./potorooJobs");
        args.add(defaultJobFile);
    }

    if (args.contains(ArgType::jobFile))
    {
        if (argProc_cond(args, 1) && (args.get(ArgType::jobFile).isValid())) return ArgProcResult::loadFile;
        else return ArgProcResult::error;
    }

    string jfErrMsg = "";
    return potoroo::argProcJF(args, jfErrMsg);
}

ArgProcResult potoroo::argProcJF(const ArgList& args, std::string& errMsg)
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
        if (err) errMsg += ", ";
        errMsg += "invalid output";
        ++err;
    }

    if (argProcJF_cond_tag(args)) cond |= (1 << 2);
    else
    {
        if (err) errMsg += ", ";
        errMsg += "invalid tag";
        ++err;
    }

    string errMsgMisc = "";

    if (argProcJF_cond_misc(args, errMsgMisc)) cond |= (1 << 3);
    else
    {
        if (err) errMsg += ", ";
        errMsg += errMsgMisc;
        ++err;
    }


    if (cond == 0x0F)
    {
        return ArgProcResult::process;
    }

    return ArgProcResult::error;
}
