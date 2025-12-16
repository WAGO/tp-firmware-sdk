cmake_minimum_required(VERSION 3.24)

# determine cmake project version from VERSION file
file(STRINGS "VERSION" PROJECT_VERSION_FROM_FILE)
if(PROJECT_VERSION_FROM_FILE MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)(-(.+))?$")
    set(PROJECT_VERSION_NUMBERS "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}") 
    set(PROJECT_VERSION_REVISION "${CMAKE_MATCH_5}")
    set(FULL_PROJECT_VERSION "${PROJECT_VERSION_FROM_FILE}") 
else()
    message(ERROR "`VERSION` does not match semver format")
endif()

project(wdx-linux
    VERSION ${PROJECT_VERSION_NUMBERS}
    DESCRIPTION "WDx Linux")

# Default C/C++ options
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(WITHOUT_TEST "Disable unit test" OFF)
option(WITH_CPPTEST "Enable Parasoft C/C++test target" OFF)
option(WITH_COVERAGE "Enable code coverage" OFF)
option(BUILD_DOC "Build documentation" OFF)
option(WITH_SYSTEMD_INTEGRATION "Build paramd with systemd integration" OFF)
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

find_package(nlohmann_json REQUIRED)
find_package(Boost COMPONENTS asio)
if (NOT boost_FOUND)
    message("Boost was not found. Assuming libboost_system is installed.")
    set(boost_asio boost_system)
endif()
find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(FCGI REQUIRED fcgi)
if(WITH_SYSTEMD_INTEGRATION)
    pkg_check_modules(SYSTEMD REQUIRED libsystemd)
    set(systemd_libs ${SYSTEMD_LIBRARIES})
endif()
set(fcgi_libs ${FCGI_LIBRARIES})

if (NOT(FETCH_DEPENDENCIES))
    pkg_check_modules(WDX_CORE REQUIRED wdxcore)
    pkg_check_modules(WDX_WDA REQUIRED wdxwda)
    pkg_check_modules(WDX_TEST REQUIRED wdxtest)
    pkg_check_modules(WC_LIB REQUIRED libcommonheader) # populates WC_LIB_LIBRARIES
    pkg_check_modules(WAGO_LOGGING REQUIRED libwagologging)
    pkg_check_modules(WAGO_OPTPARSING REQUIRED libwagooptparsing)
    pkg_check_modules(WAGO_PRIVILEGES REQUIRED libwagoprivileges)
    pkg_check_modules(WAGO_CRYPTO REQUIRED libwagocrypto)
    pkg_check_modules(WAGO_TRACE REQUIRED libwtrace)
    set(wdxcore_libs ${WDX_CORE_LIBRARIES})
    set(wdxwda_libs ${WDX_WDA_LIBRARIES})
    set(wdxtest_libs ${WDX_TEST_LIBRARIES})
    set(common_header ${WC_LIB_LIBRARIES})
    set(wago_logging_libs ${WAGO_LOGGING_LIBRARIES})
    set(wago_optparsing_libs ${WAGO_OPTPARSING_LIBRARIES})
    set(wago_privileges_libs ${WAGO_PRIVILEGES_LIBRARIES})
    set(wago_crypto_libs ${WAGO_CRYPTO_LIBRARIES})
    set(wago_trace_libs ${WAGO_TRACE_LIBRARIES})

else()
    include(FetchContent)
    FetchContent_Declare(
        wdx-core
        GIT_REPOSITORY git@svgithub01001.wago.local:BU-Automation/wdx-core.git
        GIT_TAG        d7d47899bda4b0375e7d37bd45dc609d1c263a76 # v2.0.0-beta.3
        DOWNLOAD_EXTRACT_TIMESTAMP true
        USES_TERMINAL_DOWNLOAD true
    )
    FetchContent_Declare(
        common-header
        GIT_REPOSITORY git@svgithub01001.wago.local:BU-Automation/common-header.git
        GIT_TAG        4f53583e476acc563ef6a4612ade76a89e810239 # v2.3.0
        DOWNLOAD_EXTRACT_TIMESTAMP true
        USES_TERMINAL_DOWNLOAD true
    )
    FetchContent_Declare(
        daemon-utils
        GIT_REPOSITORY git@svgithub01001.wago.local:BU-Automation/daemon-utils.git
        GIT_TAG        d854ee352cf3f29c6260c52cc96fc167e0e6a59c # v2.5.0
        DOWNLOAD_EXTRACT_TIMESTAMP true
        USES_TERMINAL_DOWNLOAD true
    )
    FetchContent_MakeAvailable(wdx-core common-header daemon-utils)
    set(common_header libcommonheader)
    set(wdxcore_libs wdxcore)
    set(wdxwda_libs wdxwda)
    set(wdxtest_libs wdxtest)
    set(wago_logging_libs wagologging)
    set(wago_optparsing_libs wagooptparsing)
    set(wago_privileges_libs wagoprivileges)
    set(wago_trace_libs wtrace)
    set(wago_crypto_libs wagocrypto)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Common utilities: object library (shared by all libraries)
set(common_obj_target wdxlinuxcommon_obj)
file(GLOB_RECURSE common_sources src/common/**/*.cpp src/common/*.cpp)
add_library(${common_obj_target} OBJECT ${common_sources})
target_link_libraries(${common_obj_target} PUBLIC ${wdxcore_libs} ${common_header} ${wago_privileges_libs})
if(WITH_SYSTEMD_INTEGRATION)
    target_link_libraries(${common_obj_target} PRIVATE ${systemd_libs})
    target_compile_definitions(${common_obj_target} PRIVATE 
        SYSTEMD_INTEGRATION=true
    )
endif()
set_target_properties(${common_obj_target} PROPERTIES 
    EXPORT_COMPILE_COMMANDS ON
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
target_include_directories(${common_obj_target} PUBLIC inc)
target_include_directories(${common_obj_target} PRIVATE src/common)

# libwdxlinuxoscom: object library (for shared compilation of sources)
set(wdxlinuxoscom_obj_target wdxlinuxoscom_obj)
file(GLOB_RECURSE sources src/libwdxlinuxoscom/**/*.cpp src/libwdxlinuxoscom/*.cpp)
add_library(${wdxlinuxoscom_obj_target} OBJECT ${sources})
set(wdxlinuxoscom_pc_req_public "${wdxlinuxoscom_pc_req_public} wdxcore")
target_link_libraries(${wdxlinuxoscom_obj_target} PUBLIC ${wdxcore_libs} ${common_header} ${common_obj_target})
target_link_libraries(${wdxlinuxoscom_obj_target} PRIVATE ${wago_trace_libs} ${boost_asio} nlohmann_json::nlohmann_json)
if(WITH_SYSTEMD_INTEGRATION)
    target_link_libraries(${wdxlinuxoscom_obj_target} PUBLIC ${systemd_libs})
    target_compile_definitions(${wdxlinuxoscom_obj_target} PRIVATE 
        SYSTEMD_INTEGRATION=true
    )
endif()
set_target_properties(${wdxlinuxoscom_obj_target} PROPERTIES 
    EXPORT_COMPILE_COMMANDS ON
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
target_compile_definitions(${wdxlinuxoscom_obj_target} PRIVATE 
    WDXLINUXOSCOM_API=WC_API_EXPORT
)
target_include_directories(${wdxlinuxoscom_obj_target} PUBLIC inc)
target_include_directories(${wdxlinuxoscom_obj_target} PRIVATE src/common src/libwdxlinuxoscom)

# libwdxlinuxoscom: shared library (for external use with controlled symbol visibility)
set(wdxlinuxoscom_lib_target wdxlinuxoscom)
set(wdxlinuxoscom_lib_description "Library for interprocess communication for WDx.")
add_library(${wdxlinuxoscom_lib_target} SHARED)
target_link_libraries(${wdxlinuxoscom_lib_target} PUBLIC ${wdxcore_libs} ${common_header} ${wdxlinuxoscom_obj_target} ${common_obj_target} ${wago_trace_libs})
target_link_libraries(${wdxlinuxoscom_lib_target} PRIVATE ${wago_privileges_libs} ${boost_asio} nlohmann_json::nlohmann_json)
if(WITH_SYSTEMD_INTEGRATION)
    set(wdxlinuxoscom_pc_req_public "${wdxlinuxoscom_pc_req_public} libsystemd")
    target_link_libraries(${wdxlinuxoscom_lib_target} PUBLIC ${systemd_libs})
    target_compile_definitions(${wdxlinuxoscom_lib_target} PRIVATE 
        SYSTEMD_INTEGRATION=true
    )
endif()
set_target_properties(${wdxlinuxoscom_lib_target} PROPERTIES 
    EXPORT_COMPILE_COMMANDS ON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
target_include_directories(${wdxlinuxoscom_lib_target} PUBLIC inc)
target_include_directories(${wdxlinuxoscom_lib_target} PRIVATE src/common src/libwdxlinuxoscom)
install(TARGETS ${wdxlinuxoscom_lib_target} LIBRARY DESTINATION lib)
configure_file(lib${wdxlinuxoscom_lib_target}.pc.in lib${wdxlinuxoscom_lib_target}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${wdxlinuxoscom_lib_target}.pc
    DESTINATION lib/pkgconfig)

# libwdxlinuxosclient: static library
set(wdxlinuxosclient_lib_target wdxlinuxosclient)
set(wdxlinuxosclient_lib_description "Library for easy creation of parameter provider client processes.")
file(GLOB_RECURSE sources src/libwdxlinuxosclient/**/*.cpp src/libwdxlinuxosclient/*.cpp)
add_library(${wdxlinuxosclient_lib_target} STATIC ${sources})
set(wdxlinuxosclient_pc_req_public "${wdxlinuxosclient_pc_req_public} libwdxlinuxoscom libwagooptparsing libwtrace libwagologging libwagoprivileges")
target_link_libraries(${wdxlinuxosclient_lib_target} PUBLIC ${wdxlinuxoscom_lib_target} ${wago_optparsing_libs} ${wago_trace_libs} ${wago_logging_libs} ${wago_privileges_libs} Threads::Threads ${common_obj_target})
if(WITH_SYSTEMD_INTEGRATION)
    target_compile_definitions(${wdxlinuxosclient_lib_target} PRIVATE
        SYSTEMD_INTEGRATION=true
    )
endif()
set_target_properties(${wdxlinuxosclient_lib_target} PROPERTIES EXPORT_COMPILE_COMMANDS ON)
set_target_properties(${wdxlinuxosclient_lib_target} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${wdxlinuxosclient_lib_target} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
target_include_directories(${wdxlinuxosclient_lib_target} PUBLIC inc)
target_include_directories(${wdxlinuxosclient_lib_target} PRIVATE src/common src/libwdxlinuxosclient)
install(TARGETS ${wdxlinuxosclient_lib_target} ARCHIVE DESTINATION lib)
configure_file(lib${wdxlinuxosclient_lib_target}.pc.in lib${wdxlinuxosclient_lib_target}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${wdxlinuxosclient_lib_target}.pc
    DESTINATION lib/pkgconfig)

# libwdxlinuxosfile: static library
set(wdxlinuxosfile_lib_target wdxlinuxosfile)
set(wdxlinuxosfile_lib_description "Library which provides utilities for testing parameter service components, such as providers.")
file(GLOB_RECURSE sources src/libwdxlinuxosfile/**/*.cpp src/libwdxlinuxosfile/*.cpp)
add_library(${wdxlinuxosfile_lib_target} STATIC ${sources})
set(wdxlinuxosfile_pc_req_public "${wdxlinuxosfile_pc_req_public} wdxcore libwagoprivileges")
target_link_libraries(${wdxlinuxosfile_lib_target} PUBLIC ${wdxcore_libs} ${wago_privileges_libs} ${common_header} ${common_obj_target})
set_target_properties(${wdxlinuxosfile_lib_target} PROPERTIES EXPORT_COMPILE_COMMANDS ON)
set_target_properties(${wdxlinuxosfile_lib_target} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${wdxlinuxosfile_lib_target} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
target_include_directories(${wdxlinuxosfile_lib_target} PUBLIC inc)
target_include_directories(${wdxlinuxosfile_lib_target} PRIVATE src/common src/libwdxlinuxosfile)
install(TARGETS ${wdxlinuxosfile_lib_target} ARCHIVE DESTINATION lib)
configure_file(lib${wdxlinuxosfile_lib_target}.pc.in lib${wdxlinuxosfile_lib_target}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${wdxlinuxosfile_lib_target}.pc
    DESTINATION lib/pkgconfig)

# libwdxlinuxosserv: static library
set(wdxlinuxosserv_lib_target wdxlinuxosserv)
set(wdxlinuxosserv_lib_description "Library for creation of parameter service processes.")
file(GLOB_RECURSE sources src/libwdxlinuxosserv/**/*.cpp src/libwdxlinuxosserv/*.cpp)
add_library(${wdxlinuxosserv_lib_target} STATIC ${sources})
set(wdxlinuxosserv_pc_req_public "${wdxlinuxosserv_pc_req_public} wdxcore wdxwda libwdxlinuxoscom libwagoprivileges libwagocrypto fcgi")
target_link_libraries(${wdxlinuxosserv_lib_target} PUBLIC ${wdxlinuxoscom_lib_target} ${wdxwda_libs} ${wdxcore_libs} ${common_header} ${fcgi_libs} ${wago_privileges_libs} ${wago_crypto_libs} CURL::libcurl ${wago_trace_libs} ${common_obj_target})
target_link_libraries(${wdxlinuxosserv_lib_target} PRIVATE nlohmann_json::nlohmann_json)
if(WITH_SYSTEMD_INTEGRATION)
    set(wdxlinuxosserv_pc_req_public "${wdxlinuxosserv_pc_req_public} libsystemd")
    target_link_libraries(${wdxlinuxosserv_lib_target} PUBLIC ${systemd_libs})
    target_compile_definitions(${wdxlinuxosserv_lib_target} PRIVATE 
        SYSTEMD_INTEGRATION=true
    )
endif()
set_target_properties(${wdxlinuxosserv_lib_target} PROPERTIES EXPORT_COMPILE_COMMANDS ON)
set_target_properties(${wdxlinuxosserv_lib_target} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${wdxlinuxosserv_lib_target} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
target_compile_definitions(${wdxlinuxosserv_lib_target} PRIVATE 
    WDM_DIR="/etc/wdx/wdm"
    WDD_DIR="/etc/wdx/wdd"
    WDE_DIR="/etc/wdx/wde"
    WDXSERV_VERSION=${FULL_PROJECT_VERSION}
    WDXSERV_MAJOR=${PROJECT_VERSION_MAJOR}
    WDXSERV_MINOR=${PROJECT_VERSION_MINOR}
    WDXSERV_BUGFIX=${PROJECT_VERSION_PATCH}
    WDXSERV_REVISION=${PROJECT_VERSION_REVISION}
)
target_include_directories(${wdxlinuxosserv_lib_target} PUBLIC inc)
target_include_directories(${wdxlinuxosserv_lib_target} PRIVATE src/common src/libwdxlinuxosserv ${FCGI_INCLUDE_DIR}) # include fcgi header
install(TARGETS ${wdxlinuxosserv_lib_target} ARCHIVE DESTINATION lib)
configure_file(lib${wdxlinuxosserv_lib_target}.pc.in lib${wdxlinuxosserv_lib_target}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${wdxlinuxosserv_lib_target}.pc
    DESTINATION lib/pkgconfig)

# paramd
file(GLOB_RECURSE paramd_sources CONFIGURE_DEPENDS src/paramd/*.cpp
                                                   src/paramd/**/*.cpp)
add_executable(paramd ${paramd_sources})
target_link_libraries(paramd PRIVATE ${wdxlinuxoscom_lib_target} ${wdxlinuxosserv_lib_target} ${wago_trace_libs} ${wago_logging_libs} ${wago_privileges_libs} ${wago_optparsing_libs} Threads::Threads)
set_target_properties(paramd PROPERTIES EXPORT_COMPILE_COMMANDS ON)
if(WITH_SYSTEMD_INTEGRATION)
    target_link_libraries(paramd PRIVATE ${systemd_libs})
    target_compile_definitions(paramd PRIVATE 
        SYSTEMD_INTEGRATION=true
        LEGACY_DAEMON=false
    )
endif()
target_include_directories(paramd PRIVATE inc src/common src/paramd)
install(TARGETS paramd RUNTIME DESTINATION sbin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config/paramd/paramd.conf DESTINATION /etc/paramd)

# apitest client
file(GLOB_RECURSE apitest_sources CONFIGURE_DEPENDS src/apitest/*.cpp)
add_executable(apitest ${apitest_sources})
target_link_libraries(apitest PRIVATE ${wdxlinuxosfile_lib_target} ${wdxlinuxosclient_lib_target})
set_target_properties(apitest PROPERTIES EXPORT_COMPILE_COMMANDS ON)
target_include_directories(apitest PRIVATE inc src/common src/apitest)
install(TARGETS apitest RUNTIME DESTINATION sbin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config/apitest/apitest.wdm.json DESTINATION /etc/wdx/wde)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config/apitest/apitest.wdd.json DESTINATION /etc/wdx/wde)

# ASIO test server
file(GLOB_RECURSE asio_test_server_sources CONFIGURE_DEPENDS src/asio-testapp/testserver.cpp
                                                             src/common/*.cpp
                                                             src/libwdxlinuxoscom/common/*.cpp
                                                             src/libwdxlinuxoscom/asio/*.cpp
                                                             src/libwdxlinuxoscom/system_abstraction/*.cpp
                                                             src/libwdxlinuxoscom/exception.cpp)
add_executable(asio_test_server ${asio_test_server_sources})
set_target_properties(asio_test_server PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
target_link_libraries(asio_test_server PRIVATE ${wago_privileges_libs} ${wdxcore_libs} ${boost_asio} ${common_header} Threads::Threads)
target_include_directories(asio_test_server PRIVATE inc src/common src/asio-testapp src/libwdxlinuxoscom)

# ASIO test client
file(GLOB_RECURSE asio_test_client_sources CONFIGURE_DEPENDS src/asio-testapp/testclient.cpp
                                                             src/common/*.cpp
                                                             src/libwdxlinuxoscom/common/*.cpp
                                                             src/libwdxlinuxoscom/asio/*.cpp
                                                             src/libwdxlinuxoscom/system_abstraction/*.cpp
                                                             src/libwdxlinuxoscom/exception.cpp)
add_executable(asio_test_client ${asio_test_client_sources})
set_target_properties(asio_test_client PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
target_link_libraries(asio_test_client PRIVATE ${wago_privileges_libs} ${wdxcore_libs} ${boost_asio} ${common_header} Threads::Threads)
target_include_directories(asio_test_client PRIVATE inc src/common src/asio-testapp src/libwdxlinuxoscom)

# Example backend client
file(GLOB_RECURSE example_backend_client_sources CONFIGURE_DEPENDS src/examples/clients/backend_client/*.cpp
                                                                   src/examples/providers/*.cpp)
add_executable(example_backend_client ${example_backend_client_sources})
set_target_properties(example_backend_client PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
target_link_libraries(example_backend_client PRIVATE ${wdxlinuxosclient_lib_target})
target_include_directories(example_backend_client PRIVATE inc src/examples)

# Example file API client
file(GLOB_RECURSE example_file_api_client_sources CONFIGURE_DEPENDS src/examples/clients/file_api_client/*.cpp
                                                                    src/examples/providers/*.cpp)
add_executable(example_file_api_client ${example_file_api_client_sources})
set_target_properties(example_file_api_client PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
target_link_libraries(example_file_api_client PRIVATE ${wdxlinuxosclient_lib_target})
target_include_directories(example_file_api_client PRIVATE inc src/examples)

# Example frontend client
file(GLOB_RECURSE example_frontend_client_sources CONFIGURE_DEPENDS src/examples/clients/frontend_client/*.cpp
                                                                    src/examples/providers/*.cpp)
add_executable(example_frontend_client ${example_frontend_client_sources})
set_target_properties(example_frontend_client PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
target_link_libraries(example_frontend_client PRIVATE ${wdxlinuxosclient_lib_target})
target_include_directories(example_frontend_client PRIVATE inc src/examples)

# Example homeless provider client
file(GLOB_RECURSE example_homeless_provider_client_sources CONFIGURE_DEPENDS src/examples/clients/homeless_provider_client/*.cpp
                                                                             src/examples/providers/*.cpp)
add_executable(example_homeless_provider_client ${example_homeless_provider_client_sources})
set_target_properties(example_homeless_provider_client PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
target_link_libraries(example_homeless_provider_client PRIVATE ${wdxlinuxosclient_lib_target})
target_include_directories(example_homeless_provider_client PRIVATE inc src/examples)

# install all headers
install(DIRECTORY inc/ DESTINATION include)

if(NOT(WITHOUT_TEST))
    enable_testing()
    find_package(GTest REQUIRED)

    include(GoogleTest)

    file(GLOB client_lib_test_sources CONFIGURE_DEPENDS test-src/common/*.cpp test-src/libwdxlinuxosclient/*.cpp)
    add_executable(${wdxlinuxosclient_lib_target}_test ${client_lib_test_sources})
    set_target_properties(${wdxlinuxosclient_lib_target}_test PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
    target_include_directories(${wdxlinuxosclient_lib_target}_test PRIVATE inc test-inc src/common src/libwdxlinuxosclient)
    target_link_libraries(${wdxlinuxosclient_lib_target}_test PRIVATE GTest::gtest_main GTest::gmock ${wdxtest_libs} ${wdxlinuxosclient_lib_target})
    if(WITH_SYSTEMD_INTEGRATION)
        target_compile_definitions(${wdxlinuxosclient_lib_target}_test PRIVATE 
            SYSTEMD_INTEGRATION=true
        )
    endif()
    gtest_discover_tests(${wdxlinuxosclient_lib_target}_test XML_OUTPUT_DIR "${CMAKE_BINARY_DIR}/test-results")

    file(GLOB com_lib_test_sources CONFIGURE_DEPENDS test-src/common/*.cpp test-src/libwdxlinuxoscom/*.cpp)
    add_executable(${wdxlinuxoscom_lib_target}_test ${com_lib_test_sources})
    set_target_properties(${wdxlinuxoscom_lib_target}_test PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
    target_include_directories(${wdxlinuxoscom_lib_target}_test PRIVATE inc test-inc src/common src/libwdxlinuxoscom)
    target_link_libraries(${wdxlinuxoscom_lib_target}_test PRIVATE GTest::gtest_main GTest::gmock ${boost_asio} ${wdxtest_libs} ${wdxlinuxoscom_obj_target} ${common_obj_target})
    gtest_discover_tests(${wdxlinuxoscom_lib_target}_test XML_OUTPUT_DIR "${CMAKE_BINARY_DIR}/test-results")

    file(GLOB file_lib_test_sources CONFIGURE_DEPENDS test-src/common/*.cpp test-src/libwdxlinuxosfile/*.cpp)
    add_executable(${wdxlinuxosfile_lib_target}_test ${file_lib_test_sources})
    set_target_properties(${wdxlinuxosfile_lib_target}_test PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
    target_include_directories(${wdxlinuxosfile_lib_target}_test PRIVATE inc test-inc src/common src/libwdxlinuxosfile)
    target_link_libraries(${wdxlinuxosfile_lib_target}_test PRIVATE GTest::gtest_main GTest::gmock ${wdxtest_libs} ${wdxlinuxosfile_lib_target})
    gtest_discover_tests(${wdxlinuxosfile_lib_target}_test XML_OUTPUT_DIR "${CMAKE_BINARY_DIR}/test-results")

    file(GLOB serv_lib_test_sources CONFIGURE_DEPENDS test-src/common/*.cpp test-src/libwdxlinuxosserv/*.cpp test-src/mocks/*.cpp)
    add_executable(${wdxlinuxosserv_lib_target}_test ${serv_lib_test_sources})
    set_target_properties(${wdxlinuxosserv_lib_target}_test PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
    target_include_directories(${wdxlinuxosserv_lib_target}_test PRIVATE inc test-inc src/common src/libwdxlinuxosserv ${FCGI_INCLUDE_DIR})
    target_link_libraries(${wdxlinuxosserv_lib_target}_test PRIVATE GTest::gtest_main GTest::gmock ${wdxtest_libs} ${wdxlinuxosserv_lib_target})
    if(WITH_SYSTEMD_INTEGRATION)
        target_compile_definitions(${wdxlinuxosserv_lib_target}_test PRIVATE 
            SYSTEMD_INTEGRATION=true
        )
    endif()
    gtest_discover_tests(${wdxlinuxosserv_lib_target}_test XML_OUTPUT_DIR "${CMAKE_BINARY_DIR}/test-results")

    file(GLOB paramd_test_sources CONFIGURE_DEPENDS test-src/common/*.cpp test-src/paramd/*.cpp src/paramd/modules/*.cpp)
    add_executable(paramd_test ${paramd_test_sources})
    set_target_properties(paramd_test PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
    target_include_directories(paramd_test PRIVATE inc test-inc src/common src/paramd)
    target_link_libraries(paramd_test PRIVATE GTest::gtest_main GTest::gmock ${wdxtest_libs} ${wdxlinuxosserv_lib_target} ${wago_trace_libs} ${wago_optparsing_libs})
    gtest_discover_tests(paramd_test XML_OUTPUT_DIR "${CMAKE_BINARY_DIR}/test-results")

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
            ${wdxlinuxosclient_lib_target}_test
            ${wdxlinuxoscom_lib_target}_test
            ${wdxlinuxosfile_lib_target}_test
            ${wdxlinuxosserv_lib_target}_test
            paramd_test
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
        add_custom_target(memcheck_wdx_linux
            COMMAND ${CMAKE_COMMAND} -E echo "All memcheck tasks completed successfully!"
            DEPENDS ${MEMCHECK_INDIVIDUAL_TARGETS}
            COMMENT "Running valgrind memcheck on all test executables"
            EXCLUDE_FROM_ALL TRUE
        )
    else()
        message(WARNING "valgrind not found, memcheck target will not be available")
    endif()

    # FIXME: Repair coverage
    # if(WITH_COVERAGE)
    #     list(APPEND CMAKE_PREFIX_PATH "cmake")
    #     find_package(CodeCoverage)
    #     # Enable GCC code coverage
    #     append_coverage_compiler_flags()
    #     # Define target `coverage`.
    #     setup_target_for_coverage_gcovr(
    #         NAME coverage
    #         EXECUTABLE ${wdxlinuxosclient_lib_target}_test
    #         EXCLUDE "test-*/*" "inc/*" # do not count test sources or headers towards coverage
    #         )
    # endif()

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
