#
# author        Oliver Blaser
# date          02.03.2021
# copyright     GNU GPLv3 - Copyright (c) 2021 Oliver Blaser
#

cmake_minimum_required(VERSION 3.19)

project(potoroo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

include_directories(../../src/)

add_executable(
potoroo
../../src/main.cpp
../../src/application/arg.cpp
../../src/application/job.cpp
../../src/application/processor.cpp
../../src/middleware/cliTextFormat.cpp
../../src/middleware/version.cpp
)
