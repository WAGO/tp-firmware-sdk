cmake_minimum_required(VERSION 3.20)
file(STRINGS "VERSION" PROJECT_VERSION)
project(common-header
    VERSION ${PROJECT_VERSION}
    DESCRIPTION "Common Header")

# Default C/C++ options
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(WITHOUT_TEST "Disable unit test" OFF)
option(WITH_CPPTEST "Enable Parasoft C/C++test target" OFF)
option(WITH_COVERAGE "Enable code coverage" OFF)
option(BUILD_DOC "Build documentation" OFF)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif(CCACHE_FOUND)

# Enable C/C++test target
if(WITH_CPPTEST)
    set(CPPTEST_VERSION  "2024.2.0")
    set(CPPTEST_COMPILER gcc_13-aarch64)
    find_package(cpptest)
endif(WITH_CPPTEST)

add_library(libcommonheader INTERFACE)
target_include_directories(
    libcommonheader
    SYSTEM INTERFACE
    inc include
)
# common-header: only header files
install(DIRECTORY inc/ DESTINATION include)
configure_file(libcommonheader.pc.in libcommonheader.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libcommonheader.pc
    DESTINATION lib/pkgconfig)

# lib target which compiles the header files to generate a 
# compile_commands.json (required by e.g. C/C++test)
add_library(commonheader_c STATIC src/commonheader.c)
set_target_properties(commonheader_c PROPERTIES EXPORT_COMPILE_COMMANDS ON)
target_link_libraries(commonheader_c PUBLIC libcommonheader)
add_library(commonheader_cpp STATIC src/commonheader.cpp)
# Export for CPP module disabled to avoid false positives with parasoft C language rule set
set_target_properties(commonheader_cpp PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
target_link_libraries(commonheader_cpp PUBLIC libcommonheader)

if(NOT(WITHOUT_TEST))
    enable_testing()
    find_package(GTest REQUIRED)
    
    file(GLOB_RECURSE test_sources CONFIGURE_DEPENDS test-src/**/*.cpp test-src/**/*.c)

    add_executable(${PROJECT_NAME}_test ${test_sources})
    set_target_properties(${PROJECT_NAME}_test PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
    target_link_libraries(${PROJECT_NAME}_test PRIVATE GTest::gtest_main GTest::gmock)
    include(GoogleTest)
    gtest_discover_tests(${PROJECT_NAME}_test XML_OUTPUT_DIR "${CMAKE_BINARY_DIR}/test-results")
    target_include_directories(${PROJECT_NAME}_test PRIVATE inc)

    if(WITH_COVERAGE)
        list(APPEND CMAKE_PREFIX_PATH "cmake")
        find_package(CodeCoverage)
        # Enable GCC code coverage
        append_coverage_compiler_flags()
        # Define target `coverage`.
        if(PROJECT_IS_TOP_LEVEL)
            setup_target_for_coverage_gcovr(
                NAME coverage
                EXECUTABLE ${core_lib_target}_test
                EXCLUDE "test-*/*" "inc/*" # do not count test sources or headers towards coverage
                )
        endif()
    endif()

endif()

if(BUILD_DOC)
    # check if Doxygen is installed
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        message("Doxygen build started")
        add_custom_target(doc ALL
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
            # explicitly use line buffering for doxygen to avoid stdout being interrupted by stderr mid-line
            COMMAND rm -rf "doc/html" && rm -rf "doc/latex" && stdbuf --output=L ${DOXYGEN_EXECUTABLE} "doc/doxygen.config"
            COMMENT "Generating API documentation with Doxygen")
    else(DOXYGEN_FOUND)
        message("Doxygen needs to be installed to generate the doxygen documentation")
    endif(DOXYGEN_FOUND)
endif(BUILD_DOC)
