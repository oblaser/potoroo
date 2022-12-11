/*!

\author         Oliver Blaser
\date           07.02.2021
\copyright      GNU GPLv3 - Copyright (c) 2022 Oliver Blaser

*/

#ifndef _CLITEXTFORMAT_H_
#define _CLITEXTFORMAT_H_

#include <cstdint>
#include <string>

namespace cli
{
    // SGR parameters -  see https://en.wikipedia.org/wiki/ANSI_escape_code#SGR_parameters
    const uint8_t SGR_RESET = 0;
    const uint8_t SGR_BOLD = 1;
    const uint8_t SGR_FAINT = 2;
    const uint8_t SGR_ITALIC = 3;
    const uint8_t SGR_UNDERLINE = 4;
    const uint8_t SGR_BLINK_SLOW = 5;
    const uint8_t SGR_BLINK_FAST = 6;
    const uint8_t SGR_REVERSE_VIDEO = 7;
    const uint8_t SGR_CONCEAL = 8;
    const uint8_t SGR_STRIKE_OUT = 9;
    const uint8_t SGR_FRAKTUR = 20;
    const uint8_t SGR_BOLD_FAINT_OFF = 22;
    const uint8_t SGR_ITALIC_FRAKTUR_OFF = 23;
    const uint8_t SGR_UNDERLINE_OFF = 24;
    const uint8_t SGR_BLINK_OFF = 25;
    const uint8_t SGR_PROPORTIONAL_SPACING = 26;
    const uint8_t SGR_REVERSE_VIDEO_OFF = 27;
    const uint8_t SGR_CONCEAL_OFF = 28;
    const uint8_t SGR_STRIKE_OUT_OFF = 29;
    const uint8_t SGR_FGCOLOR_SET_BASE = 30;
    const uint8_t SGR_FGCOLOR_SET = 38;
    const uint8_t SGR_FGCOLOR_DEFAULT = 39;
    const uint8_t SGR_BGCOLOR_SET_BASE = 40;
    const uint8_t SGR_BGCOLOR_SET = 48;
    const uint8_t SGR_BGCOLOR_DEFAULT = 49;
    const uint8_t SGR_PROPORTIONAL_SPACING_OFF = 50;
    const uint8_t SGR_FRAMED = 51;
    const uint8_t SGR_ENCIRCLED = 52;
    const uint8_t SGR_OVERLINED = 53;
    const uint8_t SGR_FRAME_ENCIRCLE_OFF = 54;
    const uint8_t SGR_OVERLINE_OFF = 55;
    const uint8_t SGR_ULCOLOR_SET = 58;
    const uint8_t SGR_ULCOLOR_DEFAULT = 59;
    const uint8_t SGR_FGCOLOR_BRIGHT_SET_BASE = 90;
    const uint8_t SGR_BGCOLOR_BRIGHT_SET_BASE = 100;

    // SGR colors
    const uint8_t SGRCOLOR_BLACK = 0;
    const uint8_t SGRCOLOR_RED = 1;
    const uint8_t SGRCOLOR_GREEN = 2;
    const uint8_t SGRCOLOR_YELLOW = 3;
    const uint8_t SGRCOLOR_BLUE = 4;
    const uint8_t SGRCOLOR_MAGENTA = 5;
    const uint8_t SGRCOLOR_CYAN = 6;
    const uint8_t SGRCOLOR_WHITE = 7;

    // color macros
    const uint8_t SGRFGC_BLACK = SGR_FGCOLOR_SET_BASE + SGRCOLOR_BLACK;
    const uint8_t SGRFGC_RED = SGR_FGCOLOR_SET_BASE + SGRCOLOR_RED;
    const uint8_t SGRFGC_GREEN = SGR_FGCOLOR_SET_BASE + SGRCOLOR_GREEN;
    const uint8_t SGRFGC_YELLOW = SGR_FGCOLOR_SET_BASE + SGRCOLOR_YELLOW;
    const uint8_t SGRFGC_BLUE = SGR_FGCOLOR_SET_BASE + SGRCOLOR_BLUE;
    const uint8_t SGRFGC_MAGENTA = SGR_FGCOLOR_SET_BASE + SGRCOLOR_MAGENTA;
    const uint8_t SGRFGC_CYAN = SGR_FGCOLOR_SET_BASE + SGRCOLOR_CYAN;
    const uint8_t SGRFGC_WHITE = SGR_FGCOLOR_SET_BASE + SGRCOLOR_WHITE;
    const uint8_t SGRFGC_BRIGHT_BLACK = SGR_FGCOLOR_BRIGHT_SET_BASE + SGRCOLOR_BLACK;
    const uint8_t SGRFGC_BRIGHT_RED = SGR_FGCOLOR_BRIGHT_SET_BASE + SGRCOLOR_RED;
    const uint8_t SGRFGC_BRIGHT_GREEN = SGR_FGCOLOR_BRIGHT_SET_BASE + SGRCOLOR_GREEN;
    const uint8_t SGRFGC_BRIGHT_YELLOW = SGR_FGCOLOR_BRIGHT_SET_BASE + SGRCOLOR_YELLOW;
    const uint8_t SGRFGC_BRIGHT_BLUE = SGR_FGCOLOR_BRIGHT_SET_BASE + SGRCOLOR_BLUE;
    const uint8_t SGRFGC_BRIGHT_MAGENTA = SGR_FGCOLOR_BRIGHT_SET_BASE + SGRCOLOR_MAGENTA;
    const uint8_t SGRFGC_BRIGHT_CYAN = SGR_FGCOLOR_BRIGHT_SET_BASE + SGRCOLOR_CYAN;
    const uint8_t SGRFGC_BRIGHT_WHITE = SGR_FGCOLOR_BRIGHT_SET_BASE + SGRCOLOR_WHITE;
    const uint8_t SGRBGC_BLACK = SGR_BGCOLOR_SET_BASE + SGRCOLOR_BLACK;
    const uint8_t SGRBGC_RED = SGR_BGCOLOR_SET_BASE + SGRCOLOR_RED;
    const uint8_t SGRBGC_GREEN = SGR_BGCOLOR_SET_BASE + SGRCOLOR_GREEN;
    const uint8_t SGRBGC_YELLOW = SGR_BGCOLOR_SET_BASE + SGRCOLOR_YELLOW;
    const uint8_t SGRBGC_BLUE = SGR_BGCOLOR_SET_BASE + SGRCOLOR_BLUE;
    const uint8_t SGRBGC_MAGENTA = SGR_BGCOLOR_SET_BASE + SGRCOLOR_MAGENTA;
    const uint8_t SGRBGC_CYAN = SGR_BGCOLOR_SET_BASE + SGRCOLOR_CYAN;
    const uint8_t SGRBGC_WHITE = SGR_BGCOLOR_SET_BASE + SGRCOLOR_WHITE;
    const uint8_t SGRBGC_BRIGHT_BLACK = SGR_BGCOLOR_BRIGHT_SET_BASE + SGRCOLOR_BLACK;
    const uint8_t SGRBGC_BRIGHT_RED = SGR_BGCOLOR_BRIGHT_SET_BASE + SGRCOLOR_RED;
    const uint8_t SGRBGC_BRIGHT_GREEN = SGR_BGCOLOR_BRIGHT_SET_BASE + SGRCOLOR_GREEN;
    const uint8_t SGRBGC_BRIGHT_YELLOW = SGR_BGCOLOR_BRIGHT_SET_BASE + SGRCOLOR_YELLOW;
    const uint8_t SGRBGC_BRIGHT_BLUE = SGR_BGCOLOR_BRIGHT_SET_BASE + SGRCOLOR_BLUE;
    const uint8_t SGRBGC_BRIGHT_MAGENTA = SGR_BGCOLOR_BRIGHT_SET_BASE + SGRCOLOR_MAGENTA;
    const uint8_t SGRBGC_BRIGHT_CYAN = SGR_BGCOLOR_BRIGHT_SET_BASE + SGRCOLOR_CYAN;
    const uint8_t SGRBGC_BRIGHT_WHITE = SGR_BGCOLOR_BRIGHT_SET_BASE + SGRCOLOR_WHITE;

    std::string sgr(uint8_t param);
    std::string sgr(uint8_t param, uint8_t arg);
    std::string sgr(uint8_t param, uint8_t arg0, uint8_t arg1);
    std::string sgr(uint8_t param, uint8_t arg0, uint8_t arg1, uint8_t arg2);
    std::string sgr(const uint8_t* paramList, size_t n);
}

#endif // _CLITEXTFORMAT_H_
