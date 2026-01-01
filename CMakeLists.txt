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

# 2. Installation rules
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

# 3. Package configuration
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/memcppConfigVersion.cmake" 
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "cmake/MemcppConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/memcppConfig.cmake"
    INSTALL_DESTINATION lib/cmake/memcpp
)

install(
    FILES 
        "${CMAKE_CURRENT_BINARY_DIR}/memcppConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/memcppConfigVersion.cmake"
    DESTINATION lib/cmake/memcpp
)

install(
    EXPORT MemcppTargets
    NAMESPACE memcpp::
    DESTINATION lib/cmake/memcpp
)

# 4. Testing
enable_testing()
find_package(GTest REQUIRED)

add_executable(runTests test/alloc_test.cpp)

target_link_libraries(
    runTests
    PRIVATE
    memcpp
    GTest::GTest
    GTest::Main
)

include(GoogleTest)
gtest_discover_tests(runTests)
