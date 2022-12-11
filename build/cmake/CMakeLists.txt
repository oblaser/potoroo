#
# author        Oliver Blaser
# date          15.08.2022
# copyright     GNU GPLv3 - Copyright (c) 2022 Oliver Blaser
#

cmake_minimum_required(VERSION 3.13)

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
../../src/middleware/util.cpp
../../src/middleware/version.cpp
)
