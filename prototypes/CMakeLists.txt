################################################################################
# 
# RUNTIME INFORMATION PROTOTYPE
#
# CMAKE HELP: http://www.cmake.org/cmake/help/cmake2.6docs.html
################################################################################

# wanna use cmake > 2.6
cmake_minimum_required(VERSION 2.6)

# set the project name
PROJECT("ProfileTest")

# include custom cmake modules
INCLUDE(CMakeModules/custom.cmake)

# force out of source builds (separated subdir for cmake files)
CHECK_OUT_OF_SOURCE()

# some compiler flags TODO: gcc dependent
# CMAKE_COMPILER_IS_GNUCC CMAKE_COMPILER_IS_GNUCXX
SET(CMAKE_C_FLAGS   "-g -finstrument-functions")
SET(CMAKE_CXX_FLAGS "-g -finstrument-functions")

# set executable output path
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})

# define source and header files
FILE(GLOB SRC *.cpp *.h)

# define how to build the executable
ADD_EXECUTABLE(profile_test ${SRC})
