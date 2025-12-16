cmake_minimum_required(VERSION 3.25)

# determine cmake project version from VERSION file
file(STRINGS "VERSION" PROJECT_VERSION_FROM_FILE)
if(PROJECT_VERSION_FROM_FILE MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)(-(.+))?$")
    set(PROJECT_VERSION_NUMBERS "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}")
    set(PROJECT_VERSION_REVISION "${CMAKE_MATCH_5}")
    set(FULL_PROJECT_VERSION "${PROJECT_VERSION_FROM_FILE}")
else()
    message(ERROR "`VERSION` does not match semver format")
endif()

project(wdx-core
    VERSION ${PROJECT_VERSION_NUMBERS}
    DESCRIPTION "WDx Core")

# Default C++ options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(WITHOUT_TEST "Disable unit test" OFF)
option(WITH_CPPTEST "Enable Parasoft C/C++test target" OFF)
option(WITH_COVERAGE "Enable code coverage" OFF)
option(FETCH_DEPENDENCIES "Download dependencies locally" OFF)
option(WDA_DISABLE_AUTH "Turn off the need for authentication on the REST-API" OFF)
option(WDA_ENABLE_SUBDEVICES "Turn on experimental subsevices functionality for the REST-API" ON)
option(ADMIN_WITH_ALL_PERMISSIONS "admin user has all permissions" OFF)
option(WDA_PARAMETER_PROVIDER "Build libwdxwda with included parameter provider for WDA's own settings" OFF)

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

find_package(PkgConfig)
find_package(nlohmann_json REQUIRED)

if (NOT(FETCH_DEPENDENCIES))
    pkg_check_modules(WC_LIB libcommonheader) # populates WC_LIB_LIBRARIES
    set(common_header ${WC_LIB_LIBRARIES})
else()
    include(FetchContent)
    FetchContent_Declare(
        common-header
        GIT_REPOSITORY git@svgithub01001.wago.local:BU-Automation/common-header.git
        GIT_TAG        4eebbd1d80c02d82318fb97c922b81600bf255d9 # v2.2.0
    )
    set(common_header libcommonheader)
    FetchContent_MakeAvailable(common-header)
endif()

# libwdxcore: static library, single header, .pc-file
set(core_lib_target wdxcore)
set(core_lib_description "WDx Core library.")
file(GLOB_RECURSE sources src/libwdxcore/**/*.cpp src/libwdxcore/*.cpp)
add_library(${core_lib_target} STATIC ${sources})
set_target_properties(${core_lib_target} PROPERTIES EXPORT_COMPILE_COMMANDS ON)
target_link_libraries(${core_lib_target} PUBLIC ${common_header})
target_link_libraries(${core_lib_target} PRIVATE nlohmann_json::nlohmann_json)
target_include_directories(${core_lib_target} PUBLIC inc)
target_include_directories(${core_lib_target} PRIVATE src/libwdxcore)
target_compile_definitions(${core_lib_target} PRIVATE 
    ADMIN_WITH_ALL_PERMISSIONS=$<IF:$<BOOL:${ADMIN_WITH_ALL_PERMISSIONS}>,true,false>
)
install(TARGETS ${core_lib_target} ARCHIVE DESTINATION lib)
install(FILES inc/wago/future.hpp DESTINATION include/wago)
file(GLOB core_lib_headers
     RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
     inc/wago/wdx/*.hpp
)
install(FILES ${core_lib_headers} DESTINATION include/wago/wdx)
install(DIRECTORY inc/wago/wdx/wdmm DESTINATION include/wago/wdx)
install(DIRECTORY inc/wago/wdx/file_transfer DESTINATION include/wago/wdx)
configure_file(${core_lib_target}.pc.in ${core_lib_target}.pc @ONLY) 
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${core_lib_target}.pc
    DESTINATION lib/pkgconfig)

# libwdxwda: static library, .pc-file
set(IMPLEMENTED_WDA_REST_API_VERSION 1.5.2)
set(wda_lib_target wdxwda)
set(wda_lib_description "Implementation of WDA REST-API including File-API.")
file(GLOB_RECURSE sources src/libwdxwda/**/*.cpp src/libwdxwda/*.cpp)
add_library(${wda_lib_target} STATIC ${sources})
set_target_properties(${wda_lib_target} PROPERTIES EXPORT_COMPILE_COMMANDS ON)
set(wda_pc_req_public "${wda_pc_req_public} wdxcore")
target_link_libraries(${wda_lib_target} PUBLIC ${core_lib_target} nlohmann_json::nlohmann_json ${common_header})
target_link_libraries(${wda_lib_target} PRIVATE nlohmann_json::nlohmann_json)
target_include_directories(${wda_lib_target} PUBLIC inc)
target_include_directories(${wda_lib_target} PRIVATE src/libwdxcore)
target_include_directories(${wda_lib_target} PRIVATE src/libwdxwda)
target_compile_definitions(${wda_lib_target} PRIVATE 
    WDXWDA_API=WC_API_EXPORT
    REST_API_VERSION=\"${IMPLEMENTED_WDA_REST_API_VERSION}\"
    WDXWDA_VERSION=${FULL_PROJECT_VERSION}
    WDXWDA_MAJOR=${PROJECT_VERSION_MAJOR}
    WDXWDA_MINOR=${PROJECT_VERSION_MINOR}
    WDXWDA_BUGFIX=${PROJECT_VERSION_PATCH}
    WDXWDA_REVISION=${PROJECT_VERSION_REVISION}
    ALLOW_HTTP=false
    DISABLE_AUTH=$<IF:$<BOOL:${WDA_DISABLE_AUTH}>,true,false>
    ENABLE_SUBDEVICES=$<IF:$<BOOL:${WDA_ENABLE_SUBDEVICES}>,true,false>
    PARAMETER_PROVIDER=$<IF:$<BOOL:${WDA_PARAMETER_PROVIDER}>,true,false>
)
install(TARGETS ${wda_lib_target} ARCHIVE DESTINATION lib)
install(DIRECTORY inc/wago/wdx/wda DESTINATION include/wago/wdx)
configure_file(${wda_lib_target}.pc.in ${wda_lib_target}.pc @ONLY) 
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${wda_lib_target}.pc
    DESTINATION lib/pkgconfig)

# libwdxtest: static library
find_package(GTest REQUIRED)
set(test_lib_target wdxtest)
set(test_lib_description "Library which provides utilities for testing WDx components, such as providers.")
file(GLOB_RECURSE sources src/libwdxtest/**/*.cpp src/libwdxtest/*.cpp)
add_library(${test_lib_target} STATIC ${sources})
set_target_properties(${test_lib_target} PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
set_target_properties(${test_lib_target} PROPERTIES VERSION ${PROJECT_VERSION})
set(test_pc_req_public "${test_pc_req_public} wdxcore") # Avoid 'GTest'
target_link_libraries(${test_lib_target} PUBLIC ${core_lib_target} ${common_header} nlohmann_json::nlohmann_json GTest::GTest)
target_include_directories(${test_lib_target} PUBLIC inc)
target_include_directories(${test_lib_target} PRIVATE src/libwdxtest)
install(TARGETS ${test_lib_target} ARCHIVE DESTINATION lib)
install(DIRECTORY inc/wago/wdx/test DESTINATION include/wago/wdx)
configure_file(${test_lib_target}.pc.in ${test_lib_target}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${test_lib_target}.pc
    DESTINATION lib/pkgconfig)

if(NOT(WITHOUT_TEST))
    enable_testing()
    file(GLOB_RECURSE test_sources test-src/**/*.cpp)
    add_executable(${PROJECT_NAME}_test ${test_sources})
    set_target_properties(${PROJECT_NAME}_test PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
    target_link_libraries(${PROJECT_NAME}_test ${core_lib_target} ${wda_lib_target} ${test_lib_target} nlohmann_json::nlohmann_json ${common_header} GTest::gtest_main GTest::gmock)
    include(GoogleTest)
    gtest_discover_tests(${PROJECT_NAME}_test XML_OUTPUT_DIR "${CMAKE_BINARY_DIR}/test-results.xml")
    target_include_directories(${PROJECT_NAME}_test PRIVATE inc)
    target_include_directories(${PROJECT_NAME}_test PRIVATE src/libwdxcore src/libwdxwda src/libwdxtest)
    target_include_directories(${PROJECT_NAME}_test PRIVATE test-inc)
    target_compile_definitions(${PROJECT_NAME}_test PRIVATE 
        REST_API_VERSION=\"${IMPLEMENTED_WDA_REST_API_VERSION}\"
        ENABLE_SUBDEVICES=$<IF:$<BOOL:${WDA_ENABLE_SUBDEVICES}>,true,false>
    )
    add_custom_target(check DEPENDS ${PROJECT_NAME}_test COMMAND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}_test --gtest_output=xml:${CMAKE_BINARY_DIR}/test-results.xml)

    # Add memcheck target to run valgrind on all test executables
    find_program(VALGRIND_EXECUTABLE valgrind)
    if(VALGRIND_EXECUTABLE)
        # Define valgrind options as a variable to avoid repetition
        set(VALGRIND_OPTIONS 
            --tool=memcheck
            --leak-check=full 
            --track-origins=yes 
            --fullpath-after= 
            --sigill-diagnostics=no 
            --demangle=yes 
            --num-callers=30 
            --sim-hints=no-nptl-pthread-stackcache
        )
        
        # List of all test targets
        set(TEST_TARGETS 
            wdx-core_test
        )
        
        # Create individual memcheck targets for each test using foreach loop
        set(MEMCHECK_INDIVIDUAL_TARGETS "")
        foreach(test_target ${TEST_TARGETS})
            set(memcheck_target_name "memcheck_${test_target}")
            add_custom_target(${memcheck_target_name}
                COMMAND ${CMAKE_COMMAND} -E echo "Running memcheck on ${test_target}..."
                COMMAND ${VALGRIND_EXECUTABLE} ${VALGRIND_OPTIONS} $<TARGET_FILE:${test_target}>
                DEPENDS ${test_target}
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Running valgrind memcheck on ${test_target}"
                EXCLUDE_FROM_ALL TRUE
            )
            list(APPEND MEMCHECK_INDIVIDUAL_TARGETS ${memcheck_target_name})
        endforeach()
        
        # Master memcheck target that depends on all individual memcheck targets
        add_custom_target(memcheck_wdx_core
            COMMAND ${CMAKE_COMMAND} -E echo "All memcheck tasks completed successfully!"
            DEPENDS ${MEMCHECK_INDIVIDUAL_TARGETS}
            COMMENT "Running valgrind memcheck on all test executables"
            EXCLUDE_FROM_ALL TRUE
        )
    else()
        message(WARNING "valgrind not found, memcheck target will not be available")
    endif()

    if(WITH_COVERAGE)
        list(APPEND CMAKE_PREFIX_PATH "cmake")
        find_package(CodeCoverage)
        # Enable GCC code coverage
        append_coverage_compiler_flags()
        # Define target `coverage`.
        if(PROJECT_IS_TOP_LEVEL)
            setup_target_for_coverage_gcovr(
                NAME coverage
                EXECUTABLE ${PROJECT_NAME}_test
                EXCLUDE "test-*/*" "inc/*" # do not count test sources or headers towards coverage
                )
        endif()
    endif()

endif()

### Dist target configuration ###
add_custom_target(dist ALL COMMAND "${CMAKE_MAKE_PROGRAM}" package_source)
# Read snapshot suffix from the environment variable unless it is set on the command line
set(SNAPSHOT_SUFFIX $ENV{SNAPSHOT_SUFFIX} CACHE STRING "Snapshot suffix to distinguish integration build artifacts")

set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-Source-${PROJECT_VERSION}${SNAPSHOT_SUFFIX}")
set(CPACK_SOURCE_IGNORE_FILES
    "\.devcontainer/;\.vscode/;build/;jenkins/;tools/;\.docker.*;\.git.*;docker.*")
include(CPack)
