/*!

\author         Oliver Blaser
\date           28.02.2021
\copyright      GNU GPLv3 - Copyright (c) 2021 Oliver Blaser

*/

#include "util.h"

#include "cliTextFormat.h"

namespace fs = std::filesystem;

using namespace std;
using namespace cli;

//! @brief Initializes Result::err and Result::warn to 0
Result::Result()
    : err(0), warn(0)
{
}

//! @brief Initializes Result::err to the specified value and Result::warn to 0
//! @param ne Default Result::err value
Result::Result(int ne)
    : err(ne), warn(0)
{
}

//! @brief Initializes Result::err and Result::warn to the specified values
//! @param ne Default Result::err value
//! @param nw Default Result::warn value
Result::Result(int ne, int nw)
    : err(ne), warn(nw)
{
}

Result Result::operator+(const Result& summand)
{
    return Result(this->err + summand.err, this->warn + summand.warn);
}

Result& Result::operator+=(const Result& summand)
{
    this->err += summand.err;
    this->warn += summand.warn;
    return *this;
}

bool operator>(const Result& left, int right)
{
    return ((left.err > right) || (left.warn > right) || ((left.err + left.warn) > right));
}

bool operator==(const Result& left, int right)
{
    return ((left.err == right) && (left.warn == right));
}

std::ostream& operator<<(std::ostream& os, const Result& v)
{
    os << "errors: ";
    os << sgr(SGRFGC_BRIGHT_RED) << v.err << sgr(SGR_RESET);
    os << "   warnings: ";
    os << sgr(SGRFGC_BRIGHT_YELLOW) << v.warn << sgr(SGR_RESET);

    return os;
}



//! @brief Change working dir to process jobfile
//! @param jobfile String of the jobfile path
//! @return 0 on success
Result changeWD(const std::string& jobfile)
{
    int r = 1;
    const string procStr = "change working dir";

#if PRJ_DEBUG && 0
    {
        ofstream f;
        f.open("000_cwd_before");
        f << "000_cwdTest" << endl;
        f.close();
    }
#define ___DBG_UTIL_CWD_CREATEFILE (1)
#endif

    try
    {
        fs::current_path(fs::path(jobfile).parent_path());
        r = 0;
    }
    catch (fs::filesystem_error& ex)
    {
#if PRJ_DEBUG
        printEWI(procStr, ex.what(), 0, 0, 0, 0);
#else
        printEWI(procStr, ex.code().message() + fsExceptionPath(ex), 0, 0, 0, 0);
#endif
    }
    catch (exception& ex)
    {
        printEWI(procStr, ex.what(), 0, 0, 0, 0);
    }
    catch (...)
    {
        printEWI(procStr, "unknown", 0, 0, 0, 0);
    }

#if PRJ_DEBUG
#ifdef ___DBG_UTIL_CWD_CREATEFILE
    if (r == 0)
    {
        ofstream f;
        f.open("000_cwd_after");
        f << "000_cwdTest" << endl;
        f.close();
    }
#endif
#endif

    return r;
}

std::string fsExceptionPath(const std::filesystem::filesystem_error& ex)
{
    string path1 = ex.path1().string();
    string path2 = ex.path2().string();
    string path = "";

    if ((path1.length() >= 0) || (path2.length() >= 0))
    {
        if (path2.length() == 0) path += " \"" + path1 + "\"";
        else path += "\npath1: \"" + path1 + "\"\npath2: \"" + path2 + "\"";
    }

    return path;
}

void printEWI(const std::string& file, const std::string& text, size_t line, size_t col, int ewi, int style)
{
    // because of the sgr formatting we cant use iomanip
    size_t printedWidth = 0;


    printedWidth = file.length() + 1;

    if (style == 0) cout << file << ":";
    else if (style == 1) cout << sgr(SGRFGC_BRIGHT_WHITE) << file << sgr(SGR_RESET) << ":";
    else cout << sgr(SGRFGC_BRIGHT_MAGENTA, SGR_BOLD) << "#printEWI style: " << style << "# " << sgr(SGR_RESET) << file << ":";


    if (line > 0)
    {
        const string lineStr = to_string(line);
        printedWidth += lineStr.length() + 1;

        cout << sgr(SGRFGC_BRIGHT_WHITE) << lineStr << sgr(SGR_RESET) << ":";
    }

    if (col > 0)
    {
        if (line <= 0)
        {
            ++printedWidth;
            cout << ":";
        }

        const string colStr = to_string(col);
        printedWidth += colStr.length() + 1;
        cout << sgr(SGRFGC_BRIGHT_WHITE) << colStr << sgr(SGR_RESET) << ":";
    }
    cout << " ";

    while (printedWidth++ < 21) cout << " ";


    const size_t ewiWidth = 9;

    if (ewi == 0) cout << sgr(SGRFGC_BRIGHT_RED, SGR_BOLD) << left << setw(ewiWidth) << "error:";
    else if (ewi == 1) cout << sgr(SGRFGC_BRIGHT_YELLOW, SGR_BOLD) << left << setw(ewiWidth) << "warning:";
    else if (ewi == 2) cout << sgr(SGRFGC_BRIGHT_CYAN, SGR_BOLD) << left << setw(ewiWidth) << "info:";

#if PRJ_DEBUG
    else if (ewi == -1) cout << sgr(SGRFGC_BRIGHT_MAGENTA, SGR_BOLD) << left << setw(ewiWidth) << "debug:";
#endif

    else  cout << sgr(SGRFGC_BRIGHT_MAGENTA, SGR_BOLD) << "#printEWI ewi: " << ewi << "# " << sgr(SGRFGC_BRIGHT_RED, SGR_BOLD) << "error: ";



    cout << sgr(SGR_RESET);

    if (text.length() > 5)
    {
        if ((text[0] == '#') &&
            (text[1] == '#') &&
            (text[2] == '#')
            )
        {
            bool on = false;

            size_t i = 3;

            while (i < text.length())
            {
                if (text[i] == '\"')
                {
                    if (on)
                    {
                        cout << sgr(SGR_RESET);
                        cout << text[i];
                        on = false;
                    }
                    else
                    {
                        cout << text[i];
                        cout << sgr(SGRFGC_BRIGHT_WHITE);
                        on = true;
                    }
                }
                else if (text[i] == '@')
                {
                    if (on)
                    {
                        cout << sgr(SGR_RESET);
                        on = false;
                    }
                    else
                    {
                        cout << sgr(SGRFGC_BRIGHT_WHITE);
                        on = true;
                    }
                }
                else cout << text[i];

                ++i;
            }

            cout << sgr(SGR_RESET) << endl;
            return;
        }
    }

    cout << text << endl;
}
