#######################################################################################################################
# Makefile for PTXdist package libswitchconfig
# This makefile uses the infrastructure provided by ct-build.

.SUFFIXES:  # Delete make's default suffix rules

.PHONY: default
default: all

#######################################################################################################################
# Overridable project configuration

PROJECT_ROOT ?= .

PTXDIST_PACKAGE ?= LIBSWITCHCONFIG
PTXDIST_PACKAGE_MK_FILE ?= $(PTXDIST_WORKSPACE)/rules/libswitchconfig.make

SRC_DIR = $(PROJECT_ROOT)/src

#######################################################################################################################
# Optional package/ platform feature configuration

-include $(SCRIPT_DIR)/platform_1.mk

#######################################################################################################################
# Custom toolchain configuration

LINT_RULE_FILES = \
	$(LINT_CONFIG_DIR)/lint-rules/pfc.lnt

#######################################################################################################################
# Build target configuration

MAIN_BUILDTARGETS += \
	libswitchconfig.a \
	libswitchconfig.so \

TEST_BUILDTARGETS += \
	alltests.elf \

BUILDTARGETS += \
	$(MAIN_BUILDTARGETS) \
	$(TEST_BUILDTARGETS)

INSTALL_TARGETS += \
	$(DESTDIR)/usr/lib/pkgconfig/libswitchconfig.pc \
	$(DESTDIR)/usr/lib/libswitchconfig.so.$(LIBSWITCHCONFIG_VERSION) \
	$(DESTDIR)/usr/include/switchconfig/switch_config_api.hpp

#######################################################################################################################
# Settings shared between build targets

SHARED_CCXXFLAGS += -fPIC
SHARED_CFLAGS    += $(call option_std,gnu11)
SHARED_CXXFLAGS  += $(call option_std,gnu++17)

SHARED_INCLUDES  += -I$(PROJECT_ROOT)/extern
SHARED_INCLUDES  += -I$(SRC_DIR)
SHARED_INCLUDES  += -I$(SRC_DIR)/util
SHARED_INCLUDES  += -isystem${SYSROOT}/usr/include
SHARED_INCLUDES  += -isystem${SYSROOT}/usr/include/glib-2.0

# $(call local_prerequisites,name)
local_prerequisites = $(call lib_buildtarget_raw,$($(1)_LIBS) $($(1)_PKG_CONFIG_LIBS),$($(1)_STATICALLYLINKED))


#######################################################################################################################
# Settings for build target libswitchconfig.a

libswitchconfig.a_INCLUDES         += $(SHARED_INCLUDES)
libswitchconfig.a_VERSION          = $(LIBSWITCHCONFIG_VERSION)
libswitchconfig.a_LIBS             +=
libswitchconfig.a_STATICALLYLINKED +=
libswitchconfig.a_PKG_CONFIGS      += glib-2.0
libswitchconfig.a_DISABLEDWARNINGS += inline
libswitchconfig.a_PREREQUISITES    += $(call local_prerequisites,libswitchconfig.a)
libswitchconfig.a_SOURCES          += $(call glob_r,$(addprefix $(SRC_DIR)/**/*.,$(SOURCE_FILE_EXTENSIONS)))
libswitchconfig.a_CPPFLAGS         += $(call uniq, $(libswitchconfig.a_INCLUDES))
libswitchconfig.a_CPPFLAGS         += $(call pkg_config_cppflags,$(libswitchconfig.a_PKG_CONFIGS))
libswitchconfig.a_CPPFLAGS         += -D'LIBSWITCHCONFIG_API=WC_API_EXPORT'
libswitchconfig.a_CCXXFLAGS        += $(SHARED_CCXXFLAGS)
libswitchconfig.a_CCXXFLAGS        += $(call option_disable_warning,$(libswitchconfig.a_DISABLEDWARNINGS))
libswitchconfig.a_CFLAGS           += $(SHARED_CFLAGS)
libswitchconfig.a_CFLAGS           += $(libswitchconfig.a_CCXXFLAGS)
libswitchconfig.a_CFLAGS           += $(call pkg_config_cflags,$(libswitchconfig.a_PKG_CONFIGS))
libswitchconfig.a_CXXFLAGS         += $(SHARED_CXXFLAGS)
libswitchconfig.a_CXXFLAGS         += $(libswitchconfig.a_CCXXFLAGS)
libswitchconfig.a_CXXFLAGS         += $(call pkg_config_cxxflags,$(libswitchconfig.a_PKG_CONFIGS))
libswitchconfig.a_LDFLAGS          += $(call option_lib,$(libswitchconfig.a_LIBS) $(libswitchconfig.a_PKG_CONFIG_LIBS),libswitchconfig.a)
libswitchconfig.a_LDFLAGS          += $(call pkg_config_ldflags,$(libswitchconfig.a_PKG_CONFIGS))
libswitchconfig.a_CLANG_TIDY_RULESET = $(CLANG_TIDY_CHECKS)
libswitchconfig.a_CLANG_TIDY_CHECKS += -google-runtime-references
libswitchconfig.a_CLANG_TIDY_CHECKS += -clang-diagnostic-c++98-c++11-c++14-compat

#######################################################################################################################
# Settings for build target libswitchconfig.so

libswitchconfig.so_INCLUDES         += $(SHARED_INCLUDES)
libswitchconfig.so_VERSION          = $(LIBSWITCHCONFIG_VERSION)
libswitchconfig.so_LIBS             +=
libswitchconfig.so_STATICALLYLINKED +=
libswitchconfig.so_PKG_CONFIGS      += glib-2.0
libswitchconfig.so_DISABLEDWARNINGS += inline
libswitchconfig.so_PREREQUISITES    += $(call local_prerequisites,libswitchconfig.so)
libswitchconfig.so_SOURCES          += $(call glob_r,$(addprefix $(SRC_DIR)/**/*.,$(SOURCE_FILE_EXTENSIONS)))
libswitchconfig.so_CPPFLAGS         += $(libswitchconfig.a_INCLUDES)
libswitchconfig.so_CPPFLAGS         += $(call pkg_config_cppflags,$(libswitchconfig.so_PKG_CONFIGS))
libswitchconfig.so_CCXXFLAGS        += $(OPTION_PIC)
libswitchconfig.so_CCXXFLAGS        += $(SHARED_CCXXFLAGS)
libswitchconfig.so_CCXXFLAGS        += $(call option_disable_warning,$(libswitchconfig.so_DISABLEDWARNINGS))
libswitchconfig.so_CFLAGS           += $(SHARED_CFLAGS)
libswitchconfig.so_CFLAGS           += $(libswitchconfig.so_CCXXFLAGS)
libswitchconfig.so_CFLAGS           += $(call pkg_config_cflags,$(libswitchconfig.so_PKG_CONFIGS))
libswitchconfig.so_CXXFLAGS         += $(SHARED_CXXFLAGS)
libswitchconfig.so_CXXFLAGS         += $(libswitchconfig.so_CCXXFLAGS)
libswitchconfig.so_CXXFLAGS         += $(call pkg_config_cxxflags,$(libswitchconfig.so_PKG_CONFIGS))
libswitchconfig.so_LDFLAGS          += $(call option_lib,$(libswitchconfig.so_LIBS) $(libswitchconfig.so_PKG_CONFIG_LIBS),libswitchconfig.so)
libswitchconfig.so_LDFLAGS          += $(call pkg_config_ldflags,$(libswitchconfig.so_PKG_CONFIGS))
libswitchconfig.so_CLANG_TIDY_RULESET = $(CLANG_TIDY_CHECKS)
libswitchconfig.so_CLANG_TIDY_CHECKS += -google-runtime-references
libswitchconfig.so_CLANG_TIDY_CHECKS += -clang-diagnostic-c++98-c++11-c++14-compat

#######################################################################################################################
# Settings for build target alltests.elf

alltests.elf_LIBS             += gmock_main gmock gtest switchconfig
alltests.elf_STATICALLYLINKED += gmock_main gmock gtest switchconfig
alltests.elf_PKG_CONFIGS      += glib-2.0
alltests.elf_DISABLEDWARNINGS += inline
alltests.elf_PREREQUISITES    += $(call local_prerequisites,alltests.elf)
alltests.elf_SOURCES          += $(call glob_r,$(addprefix $(PROJECT_ROOT)/test-src/**/*.,$(SOURCE_FILE_EXTENSIONS)))
alltests.elf_CPPFLAGS         += $(SHARED_INCLUDES)
alltests.elf_CPPFLAGS         += -I$(SRC_DIR)
alltests.elf_CPPFLAGS         += -I$(SRC_DIR)/libswitchconfig
alltests.elf_CPPFLAGS         += $(call pkg_config_cppflags,$(alltests.elf_PKG_CONFIGS))
alltests.elf_CCXXFLAGS        += $(SHARED_CCXXFLAGS)
alltests.elf_CCXXFLAGS        += $(call option_disable_warning,$(alltests.elf_DISABLEDWARNINGS))
alltests.elf_CFLAGS           += $(SHARED_CFLAGS)
alltests.elf_CFLAGS           += $(alltests.elf_CCXXFLAGS)
alltests.elf_CFLAGS           += $(call pkg_config_cflags,$(alltests.elf_PKG_CONFIGS))
alltests.elf_CXXFLAGS         += $(SHARED_CXXFLAGS)
alltests.elf_CXXFLAGS         += $(alltests.elf_CCXXFLAGS)
alltests.elf_CXXFLAGS         += $(call pkg_config_cxxflags,$(alltests.elf_PKG_CONFIGS))
alltests.elf_LDFLAGS          += $(call option_lib,$(alltests.elf_LIBS),alltests.elf)
alltests.elf_LDFLAGS          += $(call pkg_config_ldflags,$(alltests.elf_PKG_CONFIGS))
alltests.elf_CLANG_TIDY_RULESET = $(CLANG_TIDY_CHECKS)
alltests.elf_CLANG_TIDY_CHECKS += -google-runtime-references
alltests.elf_CLANG_TIDY_CHECKS += -clang-diagnostic-c++98-c++11-c++14-compat
alltests.elf_CLANG_TIDY_CHECKS += $(GTEST_CLANG_TIDY_CHECKS)

#######################################################################################################################
# Build infrastructure

include $(SCRIPT_DIR)/buildclang_1.mk
-include $(PTXDIST_WORKSPACE)/../wago_intern/debugging/scripts/valgrind-target.mk


#######################################################################################################################
# Custom rules

HDR_DIR=$(DESTDIR)/usr/include/switchconfig

$(DESTDIR)/usr/lib/pkgconfig/%.pc: $(PROJECT_ROOT)/%.pc | $(DESTDIR)/usr/lib/pkgconfig
	cp $< $@

$(DESTDIR)/usr/bin/%: $(BIN_DIR)/%.elf | $(DESTDIR)/usr/bin
	cp $< $@

$(HDR_DIR)/%.hpp: $(PROJECT_ROOT)/extern/%.hpp | $(HDR_DIR)
	cp $< $@

$(HDR_DIR):
	mkdir -p $@

#######################################################################################################################
# Bootstrapping

$(SCRIPT_DIR)/%.mk:
	$(error build scripts unavailable ($@) - set script dir (SCRIPT_DIR=$(SCRIPT_DIR)))

