cmake_minimum_required(VERSION 3.20)
project(memcpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(
    memcpp
    src/alloc.cpp
    src/alignment.cpp
)

target_include_directories(
    memcpp PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

install(
    TARGETS memcpp
    EXPORT MemcppTargets
    RUNTIME DESTINATION bin 
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib 
)

install(
    DIRECTORY include/
    DESTINATION include 
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "MemcppConfigVersion.cmake" 
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "cmake/memcppconfig.cmake.in"
    "MemcppConfig.cmake"
    INSTALL_DESTINATION lib/cmake/Memcpp
)

install(FILES 
    "${CMAKE_CURRENT_BINARY_DIR}/MemcppConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/MemcppConfigVersion.cmake"
    DESTINATION lib/cmake/memcpp
)

install(EXPORT MemcppTargets
    NAMESPACE memcpp::
    DESTINATION lib/cmake/memcpp
)

#for google tests
enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(
    runTests
    test/alloc_test.cpp
    src/alloc.cpp
    src/alignment.cpp
)

target_link_libraries(runTests
GTest::GTest
GTest::Main
pthread
)
