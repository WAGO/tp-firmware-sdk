cmake_minimum_required(VERSION 3.20)
file(STRINGS "VERSION" PROJECT_VERSION)
project(daemon-utils
    VERSION ${PROJECT_VERSION}
    DESCRIPTION "Daemon Utils")

# Default C/C++ options
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(WITHOUT_TEST "Disable unit test" OFF)
option(WITH_CPPTEST "Enable Parasoft C/C++test target" OFF)
option(WITH_COVERAGE "Enable code coverage" OFF)
option(BUILD_DOC "Build documentation" OFF)
option(WITH_SYSTEMD_INTEGRATION "Build utils with systemd integration" OFF)
option(FETCH_DEPENDENCIES "Download dependencies locally" OFF)

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

if(WITH_SYSTEMD_INTEGRATION)
    pkg_check_modules(SYSTEMD REQUIRED libsystemd)
endif()
set(systemd_libs ${SYSTEMD_LIBRARIES})

if (NOT(FETCH_DEPENDENCIES))
    pkg_check_modules(WC_LIB REQUIRED libcommonheader) # pupulates WC_LIB_LIBRARIES
    set(common_header ${WC_LIB_LIBRARIES})
else()
    # fetch dependencies if needed
    include(FetchContent)
    FetchContent_Declare(
        common-header
        GIT_REPOSITORY git@svgithub01001.wago.local:BU-Automation/common-header.git
        GIT_TAG        4eebbd1d80c02d82318fb97c922b81600bf255d9 # v2.2.0
    )
    FetchContent_MakeAvailable(common-header)
    set(common_header libcommonheader)
endif()

# Common utilities: object library (shared by all libraries)
set(common_obj_target daemonutilscommon_obj)
file(GLOB_RECURSE common_sources src/common/**/*.cpp)
add_library(${common_obj_target} OBJECT ${common_sources})
target_link_libraries(${common_obj_target} PUBLIC ${common_header})
set_target_properties(${common_obj_target} PROPERTIES 
    EXPORT_COMPILE_COMMANDS ON
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
target_include_directories(${common_obj_target} PUBLIC inc)
target_include_directories(${common_obj_target} PRIVATE src/common)

# wago-logging: static library
set(wago_logging_lib_target wagologging)
set(wago_logging_lib_description "Library which provides utilities to setup logging.")
file(GLOB_RECURSE sources src/libwagologging/**/*.cpp src/libwagologging/*.cpp)
add_library(${wago_logging_lib_target} STATIC ${sources})
target_link_libraries(${wago_logging_lib_target} PUBLIC ${common_header} ${common_obj_target})
if(WITH_SYSTEMD_INTEGRATION)
    set(wago_logging_pc_req_public "${wago_logging_pc_req_public} libsystemd")
    target_link_libraries(${wago_logging_lib_target} PUBLIC ${systemd_libs})
    target_compile_definitions(${wago_logging_lib_target} PRIVATE 
        SYSTEMD_INTEGRATION=true
    )
endif()
set_target_properties(${wago_logging_lib_target} PROPERTIES EXPORT_COMPILE_COMMANDS ON)
set_target_properties(${wago_logging_lib_target} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${wago_logging_lib_target} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
target_include_directories(${wago_logging_lib_target} PUBLIC inc)
target_include_directories(${wago_logging_lib_target} PRIVATE src/common src/libwagologging)
install(TARGETS ${wago_logging_lib_target} ARCHIVE DESTINATION lib)
configure_file(lib${wago_logging_lib_target}.pc.in lib${wago_logging_lib_target}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${wago_logging_lib_target}.pc
    DESTINATION lib/pkgconfig)

# wago-optparsing: object library (for shared compilation of sources)
set(wago_optparsing_obj_target wagooptparsing_obj)
file(GLOB_RECURSE sources src/libwagooptparsing/**/*.cpp src/libwagooptparsing/*.cpp)
add_library(${wago_optparsing_obj_target} OBJECT ${sources})
target_link_libraries(${wago_optparsing_obj_target} PUBLIC ${common_header} ${common_obj_target})
set_target_properties(${wago_optparsing_obj_target} PROPERTIES 
    EXPORT_COMPILE_COMMANDS ON
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
target_compile_definitions(${wago_optparsing_obj_target} PRIVATE 
    WAGO_OPTPARSING_API=WC_API_EXPORT
)
target_include_directories(${wago_optparsing_obj_target} PUBLIC inc)
target_include_directories(${wago_optparsing_obj_target} PRIVATE src/common src/libwagooptparsing)

# wago-optparsing: shared library (for external use with controlled symbol visibility)
set(wago_optparsing_lib_target wagooptparsing)
set(wago_optparsing_lib_description "Library which provides utilities for cli options parsing.")
add_library(${wago_optparsing_lib_target} SHARED)
target_link_libraries(${wago_optparsing_lib_target} PUBLIC ${common_header} ${wago_optparsing_obj_target})
set_target_properties(${wago_optparsing_lib_target} PROPERTIES 
    EXPORT_COMPILE_COMMANDS ON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
target_include_directories(${wago_optparsing_lib_target} PUBLIC inc)
target_include_directories(${wago_optparsing_lib_target} PRIVATE src/common src/libwagooptparsing)
install(TARGETS ${wago_optparsing_lib_target} LIBRARY DESTINATION lib)
configure_file(lib${wago_optparsing_lib_target}.pc.in lib${wago_optparsing_lib_target}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${wago_optparsing_lib_target}.pc
    DESTINATION lib/pkgconfig)

# wago-privileges: static library
set(wago_privileges_lib_target wagoprivileges)
set(wago_privileges_lib_description "Library which provides utilities to drop privileges.")
file(GLOB_RECURSE sources src/libwagoprivileges/**/*.cpp src/libwagoprivileges/*.cpp)
add_library(${wago_privileges_lib_target} STATIC ${sources})
target_link_libraries(${wago_privileges_lib_target} PUBLIC ${common_header} ${common_obj_target})
set_target_properties(${wago_privileges_lib_target} PROPERTIES EXPORT_COMPILE_COMMANDS ON)
set_target_properties(${wago_privileges_lib_target} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${wago_privileges_lib_target} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
target_include_directories(${wago_privileges_lib_target} PUBLIC inc)
target_include_directories(${wago_privileges_lib_target} PRIVATE src/common src/libwagoprivileges)
install(TARGETS ${wago_privileges_lib_target} ARCHIVE DESTINATION lib)
configure_file(lib${wago_privileges_lib_target}.pc.in lib${wago_privileges_lib_target}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${wago_privileges_lib_target}.pc
    DESTINATION lib/pkgconfig)

# wtrace: object library (for shared compilation of sources)
set(wtrace_obj_target wtrace_obj)
file(GLOB_RECURSE sources src/libwtrace/**/*.cpp src/libwtrace/*.cpp)
add_library(${wtrace_obj_target} OBJECT ${sources})
target_link_libraries(${wtrace_obj_target} PUBLIC ${common_header} ${common_obj_target})
set_target_properties(${wtrace_obj_target} PROPERTIES 
    EXPORT_COMPILE_COMMANDS ON
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
target_compile_definitions(${wtrace_obj_target} PRIVATE 
    WTRACE_API=WC_API_EXPORT
)
target_include_directories(${wtrace_obj_target} PUBLIC inc)
target_include_directories(${wtrace_obj_target} PRIVATE src/common src/libwtrace)

# wtrace: shared library (for external use with controlled symbol visibility)
set(wtrace_lib_target wtrace)
set(wtrace_lib_description "Library to build trace functionality into applications for debug and analysis.")
add_library(${wtrace_lib_target} SHARED)
target_link_libraries(${wtrace_lib_target} PUBLIC ${common_header} ${wtrace_obj_target})
set_target_properties(${wtrace_lib_target} PROPERTIES 
    EXPORT_COMPILE_COMMANDS ON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
target_include_directories(${wtrace_lib_target} PUBLIC inc)
target_include_directories(${wtrace_lib_target} PRIVATE src/common src/libwtrace)
install(TARGETS ${wtrace_lib_target} LIBRARY DESTINATION lib)
configure_file(lib${wtrace_lib_target}.pc.in lib${wtrace_lib_target}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${wtrace_lib_target}.pc
    DESTINATION lib/pkgconfig)

# wago-crypto: object library (for shared compilation of sources)
find_package(OpenSSL REQUIRED)
set(wago_crypto_obj_target wagocrypto_obj)
file(GLOB_RECURSE sources src/libwagocrypto/**/*.cpp src/libwagocrypto/*.cpp)
add_library(${wago_crypto_obj_target} OBJECT ${sources})
target_link_libraries(${wago_crypto_obj_target} PUBLIC ${common_header} OpenSSL::SSL OpenSSL::Crypto ${common_obj_target})
set_target_properties(${wago_crypto_obj_target} PROPERTIES 
    EXPORT_COMPILE_COMMANDS ON
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
target_compile_definitions(${wago_crypto_obj_target} PRIVATE 
    WAGO_CRYPTO_API=WC_API_EXPORT
)
target_include_directories(${wago_crypto_obj_target} PUBLIC inc)
target_include_directories(${wago_crypto_obj_target} PRIVATE src/common src/libwagocrypto)

# wago-crypto: shared library (for external use with controlled symbol visibility)
set(wago_crypto_lib_target wagocrypto)
set(wago_crypto_lib_description "Library providing AES-256-GCM encryption for token management.")
set(wago_crypto_lib_target wagocrypto)
add_library(${wago_crypto_lib_target} SHARED)
target_link_libraries(${wago_crypto_lib_target} PUBLIC ${common_header} OpenSSL::SSL OpenSSL::Crypto ${wago_crypto_obj_target})
set_target_properties(${wago_crypto_lib_target} PROPERTIES 
    EXPORT_COMPILE_COMMANDS ON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME wagocrypto
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
target_compile_definitions(${wago_crypto_lib_target} PRIVATE 
    WAGO_CRYPTO_API=WC_API_EXPORT
)
target_include_directories(${wago_crypto_lib_target} PUBLIC inc)
target_include_directories(${wago_crypto_lib_target} PRIVATE src/common src/libwagocrypto)
install(TARGETS ${wago_crypto_lib_target} LIBRARY DESTINATION lib)

# Create shared lib pkg-config
set(wago_crypto_lib_description "${wago_crypto_lib_description}")
configure_file(lib${wago_crypto_lib_target}.pc.in lib${wago_crypto_lib_target}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${wago_crypto_lib_target}.pc
    DESTINATION lib/pkgconfig)

# install all headers
install(DIRECTORY inc/ DESTINATION include)

if(NOT(WITHOUT_TEST))
    enable_testing()
    find_package(GTest REQUIRED)
    
    file(GLOB_RECURSE test_sources CONFIGURE_DEPENDS test-src/common/*.cpp test-src/libwagologging/*.cpp test-src/libwagooptparsing/*.cpp test-src/libwtrace/*.cpp test-src/libwagocrypto/*.cpp)

    add_executable(${PROJECT_NAME}_test ${test_sources})
    set_target_properties(${PROJECT_NAME}_test PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
    target_link_libraries(${PROJECT_NAME}_test PRIVATE GTest::gtest_main GTest::gmock ${wago_logging_lib_target} ${wago_optparsing_obj_target} ${wago_privileges_lib_target} ${wtrace_obj_target} ${wago_crypto_obj_target})
    if(WITH_SYSTEMD_INTEGRATION)
        target_link_libraries(${PROJECT_NAME}_test PRIVATE ${systemd_libs})
        target_compile_definitions(${PROJECT_NAME}_test PRIVATE 
            SYSTEMD_INTEGRATION=true
        )
    endif()
    include(GoogleTest)
    gtest_discover_tests(${PROJECT_NAME}_test XML_OUTPUT_DIR "${CMAKE_BINARY_DIR}/test-results")
    target_include_directories(${PROJECT_NAME}_test PRIVATE inc test-inc src/common src/libwagologging src/libwagooptparsing src/libwtrace src/libwagocrypto)

    if(WITH_COVERAGE)
        list(APPEND CMAKE_PREFIX_PATH "cmake")
        find_package(CodeCoverage)
        # Enable GCC code coverage
        append_coverage_compiler_flags()
        # Define target `coverage`.
        setup_target_for_coverage_gcovr(
            NAME coverage
            EXECUTABLE ${wago_logging_lib_target}_test
            EXCLUDE "test-*/*" "inc/*" # do not count test sources or headers towards coverage
            )
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
