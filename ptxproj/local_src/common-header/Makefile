#######################################################################################################################
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file is part of project common-header (PTXdist package libcommonheader).
#
# Copyright (c) 2017-2022 WAGO GmbH & Co. KG
#
# Contributors:
#   JB: WAGO GmbH & Co. KG
#   PEn: WAGO GmbH & Co. KG
#######################################################################################################################
# Makefile for PTXdist package libcommonheader
# This makefile uses the infrastructure provided by ct-build.


.SUFFIXES:

.PHONY: default doc
default: all 

#######################################################################################################################
# Overridable project configuration

#ENABLE_VERBOSE = T

PROJECT_ROOT ?= .

PTXDIST_PACKAGE ?= libcommonheader
PTXDIST_PACKAGE_MK_FILE ?= $(PTXDIST_WORKSPACE)/rules/wago-libs-029-libwc.make

SRC_DIR = $(PROJECT_ROOT)/src

CFLAGS += $(call option_std,gnu99)
CXXFLAGS += $(call option_std,gnu++14)

#######################################################################################################################
# Optional package/ platform feature configuration

-include $(SCRIPT_DIR)/platform_1.mk

#######################################################################################################################
# Custom toolchain configuration

LINT_RULE_FILES = \
$(LINT_CONFIG_DIR)/lint-rules/pfc.lnt

#######################################################################################################################
# Build target configuration

$(DISABLE_NOT_PTXDIST)MAIN_BUILDTARGETS += 

$(DISABLE_NOT_PTXDIST)TEST_BUILDTARGETS += alltests.elf

BUILDTARGETS += $(MAIN_BUILDTARGETS) $(TEST_BUILDTARGETS)

$(DISABLE_NOT_PTXDIST)INSTALL_TARGETS += 

#######################################################################################################################
# Settings for build target alltests.elf

alltests.elf_STATICALLYLINKED += gmock_main gmock gtest
alltests.elf_LIBS += gmock_main gmock gtest
alltests.elf_PKG_CONFIGS += oslinux
alltests.elf_DISABLEDWARNINGS += 
alltests.elf_PREREQUISITES += $(call lib_buildtarget,$(alltests.elf_LIBS),alltests.elf)
alltests.elf_CPPFLAGS += -I$(PROJECT_ROOT)/inc
alltests.elf_CPPFLAGS += $(call pkg_config_cppflags,$(alltests.elf_PKG_CONFIGS))
#alltests.elf_CFLAGS += $(call option_std,gnu99)
alltests.elf_CXXFLAGS += $(call option_disable_warning,$(alltests.elf_DISABLEDWARNINGS))
alltests.elf_CXXFLAGS += $(call pkg_config_cxxflags,$(alltests.elf_PKG_CONFIGS))
#alltests.elf_CXXFLAGS += $(call option_std,gnu++14)
alltests.elf_LDFLAGS += $(call option_lib,$(alltests.elf_LIBS),alltests.elf)
alltests.elf_LDFLAGS += $(call pkg_config_ldflags,$(alltests.elf_PKG_CONFIGS))
alltests.elf_SOURCES += $(call fglob_r,$(PROJECT_ROOT)/test-src,$(SOURCE_FILE_EXTENSIONS))

#######################################################################################################################
# Build infrastructure

-include $(SCRIPT_DIR)/buildclang_1.mk
-include $(PTXDIST_WORKSPACE)/../wago_intern/debugging/scripts/valgrind-target.mk

#######################################################################################################################
# Custom rules

doc:
	# explicitly use line buffering for doxygen to avoid stdout being interrupted by stderr mid-line
	rm -rf out/doc/ && stdbuf --output=L doxygen doc/doxygen.config


#######################################################################################################################
# Bootstrapping

$(SCRIPT_DIR)/%.mk:
	$(error build scripts unavailable - set script dir (SCRIPT_DIR=$(SCRIPT_DIR)), checkout or import projects)
