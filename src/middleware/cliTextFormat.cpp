/*!

\author         Oliver Blaser
\date           07.02.2021
\copyright      GNU GPLv3 - Copyright (c) 2021 Oliver Blaser

*/

#include <iostream>

#include "cliTextFormat.h"
#include "project.h"

#if(PRJ_PLAT_WIN)
#include <Windows.h>
#endif

using namespace std;

namespace
{
#if(PRJ_PLAT_WIN)

    const uint16_t FOREGROUND_MASK = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
    const uint16_t BACKGROUND_MASK = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY;

    bool setAttr(uint16_t attr)
    {
        return SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), attr);
    }

    uint16_t getCurrentAttr()
    {
        CONSOLE_SCREEN_BUFFER_INFO cInfo;
        uint16_t attr;

        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cInfo)) attr = cInfo.wAttributes;
        else attr = 0x0007;

        return attr;
    }

    bool convertColorBitsAnsiToWin(uint8_t ansiColor, uint16_t* winAttr)
    {
        bool result;

        if ((ansiColor >= 0) && (ansiColor <= 7))
        {
            *winAttr &= ~0x0007;
            if (ansiColor & cli::SGRCOLOR_RED) *winAttr |= 0x0004;
            if (ansiColor & cli::SGRCOLOR_GREEN) *winAttr |= 0x0002;
            if (ansiColor & cli::SGRCOLOR_BLUE) *winAttr |= 0x0001;
            result = true;
        }
        else result = false;

        return result;
    }

    bool setAnsiColorToWinAttr(uint8_t ansiColor, uint16_t* winAttr)
    {
        bool result;
        uint16_t attr = 0;

        // foreground
        if ((ansiColor >= 30) && (ansiColor <= 37))
        {
            convertColorBitsAnsiToWin(ansiColor - 30, &attr);
            *winAttr &= ~FOREGROUND_MASK;
            *winAttr |= attr & FOREGROUND_MASK;
            result = true;
        }

        // background
        else if ((ansiColor >= 40) && (ansiColor <= 47))
        {
            convertColorBitsAnsiToWin(ansiColor - 40, &attr);
            attr <<= 4;
            *winAttr &= ~BACKGROUND_MASK;
            *winAttr |= attr & BACKGROUND_MASK;
            result = true;
        }

        // foreground intens
        else if ((ansiColor >= 90) && (ansiColor <= 97))
        {
            convertColorBitsAnsiToWin(ansiColor - 90, &attr);
            *winAttr &= ~FOREGROUND_MASK;
            *winAttr |= (attr & FOREGROUND_MASK) | FOREGROUND_INTENSITY;
            result = true;
        }

        // background intens
        else if ((ansiColor >= 100) && (ansiColor <= 107))
        {
            convertColorBitsAnsiToWin(ansiColor - 100, &attr);
            attr <<= 4;
            *winAttr &= ~BACKGROUND_MASK;
            *winAttr |= (attr & BACKGROUND_MASK) | BACKGROUND_INTENSITY;
            result = true;
        }

        else result = false;

        return result;
    }

#endif
}

std::string cli::sgr(uint8_t param)
{
    uint8_t p[] = { param };
    return cli::sgr(p, sizeof(p) / sizeof(p[0]));
}

std::string cli::sgr(uint8_t param, uint8_t arg)
{
    uint8_t p[] = { param, arg };
    return cli::sgr(p, sizeof(p) / sizeof(p[0]));
}

std::string cli::sgr(uint8_t param, uint8_t arg0, uint8_t arg1)
{
    uint8_t p[] = { param, arg0, arg1 };
    return cli::sgr(p, sizeof(p) / sizeof(p[0]));
}

std::string cli::sgr(uint8_t param, uint8_t arg0, uint8_t arg1, uint8_t arg2)
{
    uint8_t p[] = { param, arg0, arg1, arg2 };
    return cli::sgr(p, sizeof(p) / sizeof(p[0]));
}

string cli::sgr(const uint8_t* paramList, size_t n)
{
    string result;

#if(PRJ_PLAT_WIN)

    uint16_t attr = getCurrentAttr();

    for (size_t i = 0; i < n; ++i)
    {
        if (!setAnsiColorToWinAttr(*(paramList + i), &attr))
        {
            switch (*(paramList + i))
            {
                case SGR_RESET:
                    attr = 0x0007;
                    break;

                case SGR_UNDERLINE:
                    attr |= COMMON_LVB_UNDERSCORE;
                    break;

                case SGR_UNDERLINE_OFF:
                    attr &= ~COMMON_LVB_UNDERSCORE;
                    break;

                case SGR_REVERSE_VIDEO:
                    attr |= COMMON_LVB_REVERSE_VIDEO;
                    break;

                case SGR_REVERSE_VIDEO_OFF:
                    attr &= ~COMMON_LVB_REVERSE_VIDEO;
                    break;

                case SGR_FGCOLOR_DEFAULT:
                    attr &= ~0x000F;
                    attr |= 0x0007;
                    break;

                case SGR_BGCOLOR_DEFAULT:
                    attr &= ~0x00F0;
                    break;

                default:
                    // nop 
                    break;
            }
        }
    }

    setAttr(attr);

    result.clear();

#else
#ifndef _DEBUG_ECLIPSE

    result = "\x1B[";

    for (size_t i = 0; i < n; ++i)
    {
        if (i > 0) result += ';';
        result += to_string(*(paramList + i));
    }

    result += "m";

#endif
#endif

    return result;
}
