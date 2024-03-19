cmake_minimum_required(VERSION 3.15)

project(agrossuite)
ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB)
enable_testing()

find_package(PythonExtensions REQUIRED)
find_package(Cython REQUIRED)

add_subdirectory(src)
