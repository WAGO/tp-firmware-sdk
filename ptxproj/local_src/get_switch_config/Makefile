#######################################################################################################################
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2024 WAGO GmbH & Co. KG
########################################################################################################################
# Makefile for PTXdist package get_switch_config
# This makefile uses the infrastructure provided by ct-build.

.SUFFIXES:  # Delete make's default suffix rules

.PHONY: default
default: all

#######################################################################################################################
# Overridable project configuration

PROJECT_ROOT ?= .

PTXDIST_PACKAGE ?= GET_SWITCH_CONFIG
PTXDIST_PACKAGE_MK_FILE ?= $(PTXDIST_WORKSPACE)/rules/get_switch_config.make

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
	get_switch_config.elf

TEST_BUILDTARGETS += \
	alltests.elf

BUILDTARGETS += \
	$(MAIN_BUILDTARGETS) \
	$(TEST_BUILDTARGETS)

INSTALL_TARGETS += \
	$(DESTDIR)/etc/config-tools/get_switch_config

#######################################################################################################################
# Settings shared between build targets

SHARED_CCXXFLAGS += -fPIC
SHARED_CCXXFLAGS += -fvisibility=hidden
SHARED_CFLAGS    += $(call option_std,gnu99)
SHARED_CXXFLAGS  += $(call option_std,gnu++17)
SHARED_CXXFLAGS  += -fvisibility-inlines-hidden

SHARED_INCLUDES  += -isystem$(SYSROOT)/usr/include

# $(call local_prerequisites,name)
local_prerequisites = $(call lib_buildtarget_raw,$($(1)_LIBS) $($(1)_PKG_CONFIG_LIBS),$($(1)_STATICALLYLINKED))


#######################################################################################################################
# Settings for build target alltests.elf

alltests.elf_LIBS             += gmock_main gmock gtest
alltests.elf_STATICALLYLINKED += gmock_main gmock gtest
alltests.elf_PKG_CONFIGS      += libswitchconfig
alltests.elf_DISABLEDWARNINGS +=
alltests.elf_PREREQUISITES    += $(call local_prerequisites,alltests.elf)
alltests.elf_SOURCES          += $(call glob_r,$(addprefix $(PROJECT_ROOT)/test-src/**/*.,$(SOURCE_FILE_EXTENSIONS)))
alltests.elf_SOURCES          += $(call glob_r,$(addprefix $(SRC_DIR)/internal/**/*.,$(SOURCE_FILE_EXTENSIONS)))
alltests.elf_CPPFLAGS         += -I$(SRC_DIR)/internal
alltests.elf_CPPFLAGS         += $(call pkg_config_cppflags,$(alltests.elf_PKG_CONFIGS))
alltests.elf_CPPFLAGS         += $(SHARED_INCLUDES)
alltests.elf_CCXXFLAGS        += $(SHARED_CCXXFLAGS)
alltests.elf_CCXXFLAGS        += $(call option_disable_warning,$(alltests.elf_DISABLEDWARNINGS))
alltests.elf_CFLAGS           += $(SHARED_CFLAGS)
alltests.elf_CFLAGS           += $(alltests.elf_CCXXFLAGS)
alltests.elf_CFLAGS           += $(call pkg_config_cflags,$(alltests.elf_PKG_CONFIGS))
alltests.elf_CXXFLAGS         += $(SHARED_CXXFLAGS)
alltests.elf_CXXFLAGS         += $(alltests.elf_CCXXFLAGS)
alltests.elf_CXXFLAGS         += $(call pkg_config_cxxflags,$(alltests.elf_PKG_CONFIGS))
alltests.elf_LDFLAGS          += $(call option_lib,$(alltests.elf_LIBS),alltests.elf)
alltests.elf_LDFLAGS          += $(call pkg_config_ldflags,$(alltests.elf_PKG_CONFIGS), --static)
alltests.elf_CLANG_TIDY_RULESET = $(CLANG_TIDY_CHECKS)
alltests.elf_CLANG_TIDY_CHECKS += -google-runtime-references
alltests.elf_CLANG_TIDY_CHECKS += -clang-diagnostic-c++98-c++11-c++14-compat
alltests.elf_CLANG_TIDY_CHECKS += $(GTEST_CLANG_TIDY_CHECKS)

#######################################################################################################################
# Settings for build target get_switch_config.elf

get_switch_config.elf_LIBS             += 
get_switch_config.elf_STATICALLYLINKED += 
get_switch_config.elf_PKG_CONFIGS      += libswitchconfig
get_switch_config.elf_DISABLEDWARNINGS +=
get_switch_config.elf_PREREQUISITES    += $(call local_prerequisites,get_switch_config.elf)
get_switch_config.elf_SOURCES          += $(call glob_r,$(addprefix $(SRC_DIR)/**/*.,$(SOURCE_FILE_EXTENSIONS)))
get_switch_config.elf_CPPFLAGS         += -I$(SRC_DIR)/internal
get_switch_config.elf_CPPFLAGS         += $(SHARED_INCLUDES)
get_switch_config.elf_CPPFLAGS         += $(call pkg_config_cppflags,$(get_switch_config.elf_PKG_CONFIGS))
get_switch_config.elf_CCXXFLAGS        += $(SHARED_CCXXFLAGS)
get_switch_config.elf_CCXXFLAGS        += $(call option_disable_warning,$(get_switch_config.elf_DISABLEDWARNINGS))
get_switch_config.elf_CFLAGS           += $(SHARED_CFLAGS)
get_switch_config.elf_CFLAGS           += $(get_switch_config.elf_CCXXFLAGS)
get_switch_config.elf_CFLAGS           += $(call pkg_config_cflags,$(get_switch_config.elf_PKG_CONFIGS))
get_switch_config.elf_CXXFLAGS         += $(SHARED_CXXFLAGS)
get_switch_config.elf_CXXFLAGS         += $(get_switch_config.elf_CCXXFLAGS)
get_switch_config.elf_CXXFLAGS         += $(call pkg_config_cxxflags,$(get_switch_config.elf_PKG_CONFIGS))
get_switch_config.elf_LDFLAGS          += $(call option_lib,$(get_switch_config.elf_LIBS),get_switch_config.elf)
get_switch_config.elf_LDFLAGS          += $(call pkg_config_ldflags,$(get_switch_config.elf_PKG_CONFIGS))
get_switch_config.elf_CLANG_TIDY_RULESET = $(CLANG_TIDY_CHECKS)
get_switch_config.elf_CLANG_TIDY_CHECKS += -google-runtime-references
get_switch_config.elf_CLANG_TIDY_CHECKS += -clang-diagnostic-c++98-c++11-c++14-compat

#######################################################################################################################
# Build infrastructure

include $(SCRIPT_DIR)/buildclang_1.mk
-include $(PTXDIST_WORKSPACE)/../wago_intern/debugging/scripts/valgrind-target.mk


#######################################################################################################################
# Custom rules

HDR_DIR=$(DESTDIR)/usr/include/get_switch_config

$(DESTDIR)/etc/config-tools/%: $(BIN_DIR)/%.elf | $(DESTDIR)/etc/config-tools
	cp $< $@

$(DESTDIR)/etc/config-tools: $(DESTDIR)
	mkdir -p $(DESTDIR)/etc/config-tools

#######################################################################################################################
# Bootstrapping

$(SCRIPT_DIR)/%.mk:
	$(error build scripts unavailable ($@) - set script dir (SCRIPT_DIR=$(SCRIPT_DIR)))

