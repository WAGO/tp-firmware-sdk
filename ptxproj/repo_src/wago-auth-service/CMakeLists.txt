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

project(auth-service
    VERSION ${PROJECT_VERSION_NUMBERS}
    DESCRIPTION "WAGO auth service")

# Default C/C++ options
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(WITHOUT_TEST "Disable unit test" OFF)
option(WITH_CPPTEST "Enable Parasoft C/C++test target" OFF)
option(WITH_COVERAGE "Enable code coverage" OFF)
option(BUILD_DOC "Build documentation" OFF)
option(WITH_SYSTEMD_INTEGRATION "Build authd with systemd integration" OFF)
option(ENFORCE_SET_ADMIN_PASSWORD "Compile with initial password setup for admin" ON)
option(WDX_USER_MODEL "Set WDx feature which should be used e.g. to change a users's password")
    set(wdx_user_model_options PasswordManagement AccountManagement)
    list(FIND wdx_user_model_options ${WDX_USER_MODEL} index)
    if(index EQUAL -1)
        message(FATAL_ERROR "WDX_USER_MODEL must be one of: ${wdx_user_model_options}")
    endif()
option(HTML_TEMPLATE_LOCATION "Set where HTML template files should be installed")
if(HTML_TEMPLATE_LOCATION STREQUAL OFF)
    message(FATAL_ERROR "HTML_TEMPLATE_LOCATION must have a target path value")
endif()
option(WWW_LOCATION "Set www (webserver root) folder location is.")
if(WWW_LOCATION STREQUAL OFF)
    message(FATAL_ERROR "WWW_LOCATION must have a target path value")
endif()
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

# System dependencies
find_package(nlohmann_json)
find_package(CURL)
find_package(OpenSSL)
find_package(PkgConfig)
pkg_check_modules(FCGI REQUIRED fcgi)
pkg_check_modules(PAM REQUIRED pam)
if(WITH_SYSTEMD_INTEGRATION)
    pkg_check_modules(SYSTEMD REQUIRED systemd)
    set(systemd_libs ${SYSTEMD_LIBRARIES})
endif()
set(fcgi_libs ${FCGI_LIBRARIES})
set(pam_libs ${PAM_LIBRARIES})

if (NOT(FETCH_DEPENDENCIES))
    pkg_check_modules(WDX_CORE REQUIRED wdxcore)
    pkg_check_modules(WDX_TEST REQUIRED wdxtest)
    pkg_check_modules(WDX_LINUXOS_COM REQUIRED libwdxlinuxoscom)
    pkg_check_modules(WC_LIB REQUIRED libcommonheader)
    pkg_check_modules(WAGO_LOGGING REQUIRED libwagologging)
    pkg_check_modules(WAGO_OPTPARSING REQUIRED libwagooptparsing)
    pkg_check_modules(WAGO_PRIVILEGES REQUIRED libwagoprivileges)
    pkg_check_modules(WAGO_CRYPTO REQUIRED libwagocrypto)
    set(wdxcore_libs ${WDX_CORE_LIBRARIES})
    set(wdxtest_libs ${WDX_TEST_LIBRARIES})
    set(wdxlinuxoscom_libs ${WDX_LINUXOS_COM_LIBRARIES})
    set(common_header ${WC_LIB_LIBRARIES})
    set(wago_logging_libs ${WAGO_LOGGING_LIBRARIES})
    set(wago_optparsing_libs ${WAGO_OPTPARSING_LIBRARIES})
    set(wago_privileges_libs ${WAGO_PRIVILEGES_LIBRARIES})
    set(wago_crypto_libs ${WAGO_CRYPTO_LIBRARIES})
else()
    include(FetchContent)

    FetchContent_Declare(
        wdx-core
        GIT_REPOSITORY git@svgithub01001.wago.local:BU-Automation/wdx-core.git
        GIT_TAG        4488307c92934a479f28873f5c7dcdfb1180350e # v2.0.0-alpha.11
        DOWNLOAD_EXTRACT_TIMESTAMP true
        USES_TERMINAL_DOWNLOAD true
    )
    FetchContent_Declare(
        wdx-linux
        GIT_REPOSITORY git@svgithub01001.wago.local:BU-Automation/wdx-linux.git
        GIT_TAG        b710b690010f19ecdfa1b030d200c6d55b4b512f # v2.0.0-alpha.15
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
    FetchContent_MakeAvailable(wdx-core wdx-linux common-header daemon-utils)
    set(common_header libcommonheader)
    set(wdxcore_libs wdxcore)
    set(wdxtest_libs wdxtest)
    set(wdxlinuxoscom_libs wdxlinuxoscom)
    set(wago_logging_libs wagologging)
    set(wago_optparsing_libs wagooptparsing)
    set(wago_privileges_libs wagoprivileges)
    set(wago_crypto_libs wagocrypto)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# libauthserv: static library
set(IMPLEMENTED_AUTH_API_VERSION 1.2.0)
set(authserv_lib_target authserv)
set(wago_logging_lib_description "Library for creation of parameter service processes.")
file(GLOB_RECURSE sources src/libauthserv/*.cpp
                          src/libauthserv/**/*.cpp
                          src/common/*.cpp
                          src/common/**/*.cpp)
add_library(${authserv_lib_target} STATIC ${sources})
set(authserv_pc_req_public "${authserv_pc_req_public} wdxcore libwdxlinuxoscom libwagoprivileges libwagocrypto openssl fcgi pam")
target_link_libraries(${authserv_lib_target} PUBLIC ${wdxcore_libs} ${wdxlinuxoscom_libs} ${wago_privileges_libs} ${wago_crypto_libs} OpenSSL::SSL OpenSSL::Crypto ${fcgi_libs} ${pam_libs})
target_link_libraries(${authserv_lib_target} PRIVATE nlohmann_json::nlohmann_json)
target_compile_definitions(${authserv_lib_target} PRIVATE 
    AUTH_API_VERSION=\"${IMPLEMENTED_AUTH_API_VERSION}\"
    AUTHSERV_VERSION=${FULL_PROJECT_VERSION}
    WDX_USER_MODEL_ACCOUNTMANAGEMENT=$<IF:$<STREQUAL:${WDX_USER_MODEL},AccountManagement>,true,false>
    WDX_USER_MODEL_PASSWORDMANAGEMENT=$<IF:$<STREQUAL:${WDX_USER_MODEL},PasswordManagement>,true,false>
    HTML_TEMPLATE_LOCATION=\"${HTML_TEMPLATE_LOCATION}\"
)
if(WITH_SYSTEMD_INTEGRATION)
    set(authserv_pc_req_public "${authserv_pc_req_public} libsystemd")
    target_link_libraries(${authserv_lib_target} PUBLIC ${systemd_libs})
    target_compile_definitions(${authserv_lib_target} PRIVATE 
        SYSTEMD_INTEGRATION=true
    )
endif()
if (ENFORCE_SET_ADMIN_PASSWORD)
    target_compile_definitions(${authserv_lib_target} PRIVATE 
        ENFORCE_SET_ADMIN_PASSWORD=true
    )
endif()
set_target_properties(${authserv_lib_target} PROPERTIES EXPORT_COMPILE_COMMANDS ON)
set_target_properties(${authserv_lib_target} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${authserv_lib_target} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
target_include_directories(${authserv_lib_target} PUBLIC inc)
target_include_directories(${authserv_lib_target} PRIVATE src/libauthserv src/common ${FCGI_INCLUDE_DIR}) # include fcgi header
install(TARGETS ${authserv_lib_target} ARCHIVE DESTINATION lib)
configure_file(lib${authserv_lib_target}.pc.in lib${authserv_lib_target}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${authserv_lib_target}.pc
    DESTINATION lib/pkgconfig)

# authd
file(GLOB_RECURSE authd_sources CONFIGURE_DEPENDS src/authd/*.cpp
                                                  src/authd/**/*.cpp
                                                  src/common/*.cpp
                                                  src/common/**/*.cpp)
add_executable(authd ${authd_sources})
target_link_libraries(authd PRIVATE ${authserv_lib_target} ${wdxlinuxoscom_libs} ${wago_logging_libs} ${wago_privileges_libs} ${wago_optparsing_libs} ${pam_libs} Threads::Threads)
set_target_properties(authd PROPERTIES EXPORT_COMPILE_COMMANDS ON)
if(WITH_SYSTEMD_INTEGRATION)
    target_link_libraries(authd PRIVATE ${systemd_libs})
    target_compile_definitions(authd PRIVATE 
        AUTHSERV_VERSION=${FULL_PROJECT_VERSION}
        SYSTEMD_INTEGRATION=true
        LEGACY_DAEMON=false
    )
else()
    target_compile_definitions(authd PRIVATE 
        AUTHSERV_VERSION=${FULL_PROJECT_VERSION}
        SYSTEMD_INTEGRATION=false
        LEGACY_DAEMON=true
    )
endif()
target_include_directories(authd PRIVATE inc src/authd src/common ${FCGI_INCLUDE_DIR})
install(TARGETS authd RUNTIME DESTINATION sbin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config/authd/authd.conf DESTINATION /etc/authd)
install(DIRECTORY DESTINATION /etc/authd/clients)
install(DIRECTORY DESTINATION /etc/authd/resource_servers)

# Install HTML-Templates
file(GLOB html_templates CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/www/auth/*.template)
install(DIRECTORY DESTINATION ${HTML_TEMPLATE_LOCATION})
install(FILES ${html_templates} DESTINATION ${HTML_TEMPLATE_LOCATION})

# Install JavaScript, CSS and Images
file(GLOB www_assets     CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/www/auth/*.js
                                           ${CMAKE_CURRENT_SOURCE_DIR}/www/auth/*.css)
install(DIRECTORY DESTINATION ${WWW_LOCATION})
install(FILES ${www_assets} DESTINATION ${WWW_LOCATION})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/www/auth/images DESTINATION ${WWW_LOCATION})

if(NOT(WITHOUT_TEST))
    enable_testing()
    find_package(GTest REQUIRED)

    include(GoogleTest)

    file(GLOB authserv_lib_test_sources CONFIGURE_DEPENDS test-src/common/*.cpp test-src/mocks/*.cpp test-src/libauthserv/*.cpp)
    add_executable(${authserv_lib_target}_test ${authserv_lib_test_sources})
    set_target_properties(${authserv_lib_target}_test PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
    target_include_directories(${authserv_lib_target}_test PRIVATE inc test-inc src/common src/libauthserv ${PAM_INCLUDE_DIR} ${FCGI_INCLUDE_DIR})
    target_link_libraries(${authserv_lib_target}_test GTest::gtest_main GTest::gmock wdxtest wdxcore wdxlinuxoscom ${authserv_lib_target})
    target_compile_definitions(${authserv_lib_target}_test PRIVATE 
        AUTH_API_VERSION=\"${IMPLEMENTED_AUTH_API_VERSION}\"
        AUTHSERV_VERSION=${FULL_PROJECT_VERSION}
        HTML_TEMPLATE_LOCATION=\"${HTML_TEMPLATE_LOCATION}\"
    )
    if(WITH_SYSTEMD_INTEGRATION)
        target_compile_definitions(${authserv_lib_target}_test PRIVATE 
            SYSTEMD_INTEGRATION=true
        )
    endif()
    gtest_discover_tests(${authserv_lib_target}_test XML_OUTPUT_DIR "${CMAKE_BINARY_DIR}/test-results")

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
            authserv_test
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
        add_custom_target(memcheck_auth_service
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
    #         EXECUTABLE ${authserv_lib_target}_test
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
