/*

\author         Oliver Blaser
\date           03.03.2021
\copyright      GNU GPLv3 - Copyright (c) 2022 Oliver Blaser

*/

#ifndef _PROJECT_H_
#define _PROJECT_H_

#include "middleware/version.h"

namespace project
{
    const Version version(0, 2, 0);
}



#define PRJ_VERSION (project::version)
#define PRJ_VERSION_PRERELEASE (0)



//
// platform
//
#ifdef _WIN32
#define PRJ_PLAT_WIN32 (1)
#endif
#ifdef _WIN64
#define PRJ_PLAT_WIN64 (1)
#endif
#ifdef __unix__
#define PRJ_PLAT_UNIX (1)
#endif
#if (PRJ_PLAT_WIN32 || PRJ_PLAT_WIN64)
#define PRJ_PLAT_WIN (1)
#endif


//
// debugging
//
#ifdef _DEBUG
#define PRJ_DEBUG (1)
#else
#define PRJ_DEBUG (0)
#endif


//
// invalid combinations
//
#if ((PRJ_PLAT_WIN32 || PRJ_PLAT_WIN64 || PRJ_PLAT_WIN) && PRJ_PLAT_UNIX)
#error invalid platform
#endif

#ifndef _DEBUG
#if PRJ_DEBUG
#error invalid debug configuration
#endif
#endif

#endif // _PROJECT_H_
