/*!

\author         Oliver Blaser
\date           07.04.2021
\copyright      GNU GPLv3 - Copyright (c) 2022 Oliver Blaser

*/

#include "util.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

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
    if (v.err != 0)
    {
        os << sgr(SGRFGC_BRIGHT_RED) << v.err << sgr(SGR_RESET) << " error";
        if ((v.err > 1) || (v.err < (-1))) os << "s";
        if (v.warn != 0) os << " and ";
    }

    if (v.warn != 0)
    {
        os << sgr(SGRFGC_BRIGHT_YELLOW) << v.warn << sgr(SGR_RESET) << " warning";
        if ((v.warn > 1) || (v.warn < (-1))) os << "s";
    }

    return os;
}

//! @brief Change working dir
//! @param newWD 
//! @return 0 on success, 1 on error
Result changeWD(const std::filesystem::path& newWD, std::string* exWhat)
{
    int r = 1;
    const string procStr = "change working dir";
    string errMsg;

    try
    {
        fs::current_path(newWD);
        r = 0;
    }
    catch (exception& ex)
    {
        errMsg = ex.what();
    }
    catch (...)
    {
        errMsg = "unknown";
    }

    if (r != 0)
    {
        if (exWhat) *exWhat = std::string(errMsg);
        else printEWI(procStr, errMsg, 0, 0, 0, 0);
    }

    return r;
}

//! @brief Prints a formatted Error, Warning or Info message
//! @param file File- or processname
//! @param text Message
//! @param line Linenumber (omitted if 0)
//! @param col Column (omitted if 0)
//! @param ewi Message type
//! @param style Format style
//! 
//! Message types: 0 error / 1 warning / 2 info
//! Styles: 0 process / 1 file
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

lineEnding detectLineEnding(const std::filesystem::path& filepath)
{
    lineEnding le = lineEnding::error;

    ifstream ifs;

    ifs.open(filepath, ios::in | ios::binary);

    if (ifs.is_open())
    {
        char buffer[2] = { 0, 0 };
        bool searching = true;

        while (searching)
        {
            buffer[1] = static_cast<char>(ifs.get());

            if (ifs.good())
            {
                if ((buffer[0] != static_cast<char>(0x0D)) && (buffer[1] == static_cast<char>(0x0A)))
                {
                    searching = false;
                    le = lineEnding::LF;
                }
                else if ((buffer[0] == static_cast<char>(0x0D)) && (buffer[1] != static_cast<char>(0x0A)))
                {
                    searching = false;
                    le = lineEnding::CR;
                }
                else if ((buffer[0] == static_cast<char>(0x0D)) && (buffer[1] == static_cast<char>(0x0A)))
                {
                    searching = false;
                    le = lineEnding::CRLF;
                }
                // else ; nop, still searching
            }
            else if (ifs.eof())
            {
                if (buffer[0] == static_cast<char>(0x0D))
                {
                    searching = false;
                    le = lineEnding::CR;
                }
                else // no new line in file -> assume LF because its the simpliest
                {
                    searching = false;
                    le = lineEnding::LF;
                }
            }
            else
            {
                searching = false;
                le = lineEnding::error;
            }

            buffer[0] = buffer[1];
        }

        ifs.close();
    }

    return le;
}

int convertLineEnding(const std::filesystem::path& inf, const std::filesystem::path& outf, lineEnding outfLineEnding)
{
    lineEnding ile = detectLineEnding(inf);
    string deadEnd;
    return convertLineEnding(inf, ile, outf, outfLineEnding, deadEnd);
}

int convertLineEnding(const std::filesystem::path& inf, lineEnding infLineEnding, const std::filesystem::path& outf, lineEnding outfLineEnding)
{
    string deadEnd;
    return convertLineEnding(inf, infLineEnding, outf, outfLineEnding, deadEnd);
}

int convertLineEnding(const std::filesystem::path& inf, lineEnding infLineEnding, const std::filesystem::path& outf, lineEnding outfLineEnding, std::string& errMsg)
{
    int result = 0;
    errMsg.clear();

    ifstream ifs;
    ofstream ofs;

    ofs.exceptions(ios::failbit | ios::badbit | ios::eofbit);

    ifs.open(inf, ios::in | ios::binary);
    ofs.open(outf, ios::out | ios::binary);

    bool proc = true;

    if (infLineEnding == lineEnding::CRLF)
    {
        char c[2] = { 0, 0 };

        while (proc)
        {
            c[0] = static_cast<char>(ifs.get());

            if (ifs.good())
            {
                if (c[0] == static_cast<char>(0x0D))
                {
                    c[1] = static_cast<char>(ifs.get());

                    if (ifs.good())
                    {
                        if ((c[0] == static_cast<char>(0x0D)) && (c[1] == static_cast<char>(0x0A)))
                        {
                            if (outfLineEnding == lineEnding::LF) ofs.put(0x0A);
                            else if (outfLineEnding == lineEnding::CR) ofs.put(0x0D);
                            else if (outfLineEnding == lineEnding::CRLF) { ofs.put(0x0D); ofs.put(0x0A); }
                            else
                            {
                                errMsg = "invalid out line ending";
                                result = 2;
                                proc = false;
                            }
                        }
                        else
                        {
                            ofs.put(c[0]);
                            ofs.put(c[1]);
                        }
                    }
                    else if (ifs.eof())
                    {
                        ofs.put(c[0]);
                        proc = false;
                    }
                    else
                    {
                        errMsg = "in file IO error: ";

                        if (ifs.bad()) errMsg += "badbit";
                        else if (ifs.fail()) errMsg += "failbit";
                        else errMsg += "unknown";

                        result = 3;
                        proc = false;
                    }
                }
                else ofs.put(c[0]);
            }
            else proc = false;
        }
    }
    else if ((infLineEnding == lineEnding::LF) || (infLineEnding == lineEnding::CR))
    {
        while (proc)
        {
            char c = static_cast<char>(ifs.get());

            if (ifs.good())
            {
                if (((infLineEnding == lineEnding::LF) && (c == static_cast<char>(0x0A))) ||
                    ((infLineEnding == lineEnding::CR) && (c == static_cast<char>(0x0D)))
                    )
                {
                    if (outfLineEnding == lineEnding::LF) ofs.put(0x0A);
                    else if (outfLineEnding == lineEnding::CR) ofs.put(0x0D);
                    else if (outfLineEnding == lineEnding::CRLF) { ofs.put(0x0D); ofs.put(0x0A); }
                    else
                    {
                        errMsg = "invalid out line ending";
                        result = 2;
                        proc = false;
                    }
                }
                else ofs.put(c);
            }
            else proc = false;
        }
    }
    else
    {
        errMsg = "invalid in line ending";
        result = 1;
    }

    ifs.close();
    ofs.close();

    return result;
}



size_t strReplaceAll(std::string& str, const std::string& from, const std::string& to)
{
    size_t cnt = 0;
    size_t pos = 0;

    if (from.empty()) return 0;

    while (1)
    {
        pos = str.find(from, pos);

        if (pos == std::string::npos) break;
        else
        {
            str.replace(pos, from.length(), to);
            pos += to.length();
            ++cnt;
        }
    }

    return cnt;
}

bool vectorContains(const std::vector<int>& vec, int value)
{
    for (size_t i = 0; i < vec.size(); ++i)
    {
        if (vec[i] == value) return true;
    }

    return false;
}
