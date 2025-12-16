#######################################################################################################################
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2024-2025 WAGO GmbH & Co. KG
#
# Contributors:
#   PEn:  WAGO GmbH & Co. KG
#   MaHe: WAGO GmbH & Co. KG
#######################################################################################################################
# Makefile for PTXdist package wago-daemon-utils.
#
# For further information about the PTXdist project and license conditions
# see the PTXdist README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_WAGO_DAEMON_UTILS) += wago-daemon-utils

#
# Paths and names
#
WAGO_DAEMON_UTILS_BASE           := daemon-utils
WAGO_DAEMON_UTILS_VERSION        := 2.5.0
WAGO_DAEMON_UTILS_MD5            :=
WAGO_DAEMON_UTILS                := $(WAGO_DAEMON_UTILS_BASE)
WAGO_DAEMON_UTILS_GIT_URL        := git@svgithub01001.wago.local:BU-Automation/daemon-utils
WAGO_DAEMON_UTILS_URL            := file://$(WAGO_DAEMON_UTILS_SRC_DIR)
WAGO_DAEMON_UTILS_GIT_BRANCH     := $(WAGO_DAEMON_UTILS_VERSION)
WAGO_DAEMON_UTILS_BUILDCONFIG    := Release
WAGO_DAEMON_UTILS_REPO_SRC_DIR   := $(PTXDIST_WORKSPACE)/repo_src
WAGO_DAEMON_UTILS_SRC_DIR        := $(WAGO_DAEMON_UTILS_REPO_SRC_DIR)/$(WAGO_DAEMON_UTILS)
WAGO_DAEMON_UTILS_DIR            := $(BUILDDIR)/$(WAGO_DAEMON_UTILS)
WAGO_DAEMON_UTILS_LICENSE        := MPL-2.0
WAGO_DAEMON_UTILS_BIN            := 

WAGO_DAEMON_UTILS_PACKAGE_NAME             := $(WAGO_DAEMON_UTILS_BASE)_$(WAGO_DAEMON_UTILS_VERSION)_$(PTXDIST_IPKG_ARCH_STRING)
WAGO_DAEMON_UTILS_PLATFORMCONFIGPACKAGEDIR := $(PTXDIST_PLATFORMCONFIGDIR)/packages

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(WAGO_DAEMON_UTILS_SRC_DIR):
	{ mkdir -p $(WAGO_DAEMON_UTILS_REPO_SRC_DIR) && cd $(WAGO_DAEMON_UTILS_REPO_SRC_DIR) && git clone $(WAGO_DAEMON_UTILS_GIT_URL) $(WAGO_DAEMON_UTILS_SRC_DIR); } \
	  || (rm -fr $(WAGO_DAEMON_UTILS_SRC_DIR) && false)

$(STATEDIR)/wago-daemon-utils.get: | $(WAGO_DAEMON_UTILS_SRC_DIR)
	{ cd $(WAGO_DAEMON_UTILS_SRC_DIR) && git checkout $(WAGO_DAEMON_UTILS_GIT_BRANCH); } \
	  || (rm -fr $(WAGO_DAEMON_UTILS_SRC_DIR) && false)
else
$(STATEDIR)/wago-daemon-utils.get:
endif
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/wago-daemon-utils.extract: 
	@$(call targetinfo)
ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
	@$(call clean, WAGO_DAEMON_UTILS_DIR)
	@mkdir -p $(WAGO_DAEMON_UTILS_DIR) && cp -r $(WAGO_DAEMON_UTILS_SRC_DIR)/* $(WAGO_DAEMON_UTILS_DIR)
	@$(call patchin, WAGO_DAEMON_UTILS)
endif
	@$(call touch)

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(STATEDIR)/wago-daemon-utils.extract.post:
	@$(call targetinfo)
	@$(call touch)
endif


# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

WAGO_DAEMON_UTILS_PATH      := PATH=$(CROSS_PATH)
WAGO_DAEMON_UTILS_CONF_TOOL	:= cmake
WAGO_DAEMON_UTILS_CONF_OPT  := $(CROSS_CMAKE_USR) -D CMAKE_BUILD_TYPE=$(WAGO_DAEMON_UTILS_BUILDCONFIG) -D WITHOUT_TEST:BOOL=ON
WAGO_DAEMON_UTILS_MAKE_ENV  := $(CROSS_ENV)

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(STATEDIR)/wago-daemon-utils.prepare:
	@$(call targetinfo)
	@$(call touch)
endif

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(STATEDIR)/wago-daemon-utils.compile:
	@$(call targetinfo)
	@$(call touch)
endif

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wago-daemon-utils.install:
	@$(call targetinfo)

ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
# "normal" mode: call "install"
	@$(call world/install, WAGO_DAEMON_UTILS)

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_RELEASE
# save install directory contents for BSP
	@mkdir -p $(WAGO_DAEMON_UTILS_PLATFORMCONFIGPACKAGEDIR) && \
	  cd $(WAGO_DAEMON_UTILS_PKGDIR) && tar cvzf $(WAGO_DAEMON_UTILS_PLATFORMCONFIGPACKAGEDIR)/$(WAGO_DAEMON_UTILS_PACKAGE_NAME).tgz *
endif

else
# BSP mode: install ipk contents to packages/<project name>
	@mkdir -p $(WAGO_DAEMON_UTILS_PKGDIR) && \
	  tar xvzf $(WAGO_DAEMON_UTILS_PLATFORMCONFIGPACKAGEDIR)/$(WAGO_DAEMON_UTILS_PACKAGE_NAME).tgz -C $(WAGO_DAEMON_UTILS_PKGDIR)
endif

	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wago-daemon-utils.targetinstall:
	@$(call targetinfo)

	@$(call install_init, wago-daemon-utils)
	@$(call install_fixup, wago-daemon-utils,PRIORITY,optional)
	@$(call install_fixup, wago-daemon-utils,SECTION,base)
	@$(call install_fixup, wago-daemon-utils,AUTHOR,"PEn - WAGO GmbH \& Co. KG")
	@$(call install_fixup, wago-daemon-utils,DESCRIPTION,missing)

ifdef PTXCONF_WAGO_OPTPARSING
	@$(call install_lib, wago-daemon-utils, 0, 0, 0644, libwagooptparsing)
endif

ifdef PTXCONF_WAGO_TRACE
	@$(call install_lib, wago-daemon-utils, 0, 0, 0644, libwtrace)
endif

ifdef PTXCONF_WAGO_CRYPTO
	@$(call install_lib, wago-daemon-utils, 0, 0, 0644, libwagocrypto)
endif

	@$(call install_finish, wago-daemon-utils)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/wago-daemon-utils.clean:
	@$(call targetinfo)
	rm -rf $(PTXCONF_SYSROOT_TARGET)/usr/include/wago/logging/
	rm -rf $(PTXCONF_SYSROOT_TARGET)/usr/include/wago/optparsing/
	rm -rf $(PTXCONF_SYSROOT_TARGET)/usr/include/wago/privileges/
	rm -rf $(PTXCONF_SYSROOT_TARGET)/usr/include/wago/wtrace/
	rm -rf $(PTXCONF_SYSROOT_TARGET)/usr/include/wago/crypto/
	@$(call clean, WAGO_DAEMON_UTILS)
	@$(call clean_pkg, WAGO_DAEMON_UTILS)

# vim: syntax=make
