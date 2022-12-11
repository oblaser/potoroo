/*!

\author         Oliver Blaser
\date           08.04.2021
\copyright      GNU GPLv3 - Copyright (c) 2022 Oliver Blaser

*/

#ifndef _PROCESSOR_H_
#define _PROCESSOR_H_

#include <iostream>
#include <string>
#include <vector>

#include "job.h"

namespace potoroo
{
    const std::string processorTmpDirIncOut = "potorooTempIncProcOut";
    const std::string processorTmpDIncLineEnding = "potorooTempDIncLineEnding"; // temporary dir for dirty include line ending conversion
    const std::string processorTmpDirLineEnding = "potorooTempLineEnding";

    Result processJob(const Job& job, bool forceOutfLineEndLF = false) noexcept;
    Result processJobs(const std::vector<Job>& jobs, std::vector<bool>& success) noexcept;
}

#endif // _PROCESSOR_H_
