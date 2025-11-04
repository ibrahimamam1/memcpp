cmake_minimum_required(VERSION 3.20)
project(memcpp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#for google tests
enable_testing()
find_package(GTEST REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})