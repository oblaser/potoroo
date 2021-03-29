/*!

\author         Oliver Blaser
\date           07.03.2021
\copyright      GNU GPLv3 - Copyright (c) 2021 Oliver Blaser

*/

#ifndef _PROCESSOR_H_
#define _PROCESSOR_H_

#include <iostream>
#include <vector>

#include "job.h"

namespace potoroo
{
    Result processJob(const Job& job) noexcept;
    Result processJobs(const std::vector<Job>& jobs, std::vector<bool>& success) noexcept;
}

#endif // _PROCESSOR_H_
