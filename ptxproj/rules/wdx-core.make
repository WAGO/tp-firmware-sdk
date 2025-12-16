#######################################################################################################################
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2019-2025 WAGO GmbH & Co. KG
#
# Contributors:
#   MP:  WAGO GmbH & Co. KG
#   PEn: WAGO GmbH & Co. KG
#######################################################################################################################
# Makefile for PTXdist package wdx-core.
#
# For further information about the PTXdist project and license conditions
# see the PTXdist README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_WDX_CORE) += wdx-core

#
# Paths and names
#
WDX_CORE_BASE           := wdx-core
WDX_CORE_VERSION        := 2.2.4
WDX_CORE_MD5            :=
WDX_CORE                := $(WDX_CORE_BASE)
WDX_CORE_GIT_URL        := git@svgithub01001.wago.local:BU-Automation/wdx-core
WDX_CORE_URL            := file://$(WDX_CORE_SRC_DIR)
WDX_CORE_GIT_BRANCH     := $(WDX_CORE_VERSION)
WDX_CORE_BUILDCONFIG    := Release
WDX_CORE_REPO_SRC_DIR   := $(PTXDIST_WORKSPACE)/repo_src
WDX_CORE_SRC_DIR        := $(WDX_CORE_REPO_SRC_DIR)/$(WDX_CORE)
WDX_CORE_DIR            := $(BUILDDIR)/$(WDX_CORE)/src
WDX_CORE_LICENSE        := MPL-2.0
WDX_CORE_BIN            := 

WDX_CORE_PACKAGE_NAME             := $(WDX_CORE)_$(WDX_CORE_VERSION)_$(PTXDIST_IPKG_ARCH_STRING)
WDX_CORE_PLATFORMCONFIGPACKAGEDIR := $(PTXDIST_PLATFORMCONFIGDIR)/packages

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------
ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(WDX_CORE_SRC_DIR):
	{ mkdir -p $(WDX_CORE_REPO_SRC_DIR) && cd $(WDX_CORE_REPO_SRC_DIR) && git clone $(WDX_CORE_GIT_URL) $(WDX_CORE_SRC_DIR); } \
	  || (rm -fr $(WDX_CORE_SRC_DIR) && false)

$(STATEDIR)/wdx-core.get: | $(WDX_CORE_SRC_DIR)
	{ cd $(WDX_CORE_SRC_DIR) && git fetch -pP && git checkout $(WDX_CORE_GIT_BRANCH); } \
	  || (rm -fr $(WDX_CORE_SRC_DIR) && false)
else
$(STATEDIR)/wdx-core.get:
endif
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------
$(STATEDIR)/wdx-core.extract: 
	@$(call targetinfo)
ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
	@$(call clean, WDX_CORE_DIR)
	@mkdir -p $(WDX_CORE_DIR) && cp -r $(WDX_CORE_SRC_DIR)/* $(WDX_CORE_DIR)
	@$(call patchin, WDX_CORE)
endif
	@$(call touch)

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(STATEDIR)/wdx-core.extract.post:
	@$(call targetinfo)
	@$(call touch)
endif

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------
WDX_CORE_PATH      := PATH=$(CROSS_PATH)
WDX_CORE_CONF_TOOL := cmake
WDX_CORE_CONF_OPT  := $(CROSS_CMAKE_USR) -D CMAKE_BUILD_TYPE=$(WDX_CORE_BUILDCONFIG) -D WITHOUT_TEST:BOOL=ON -D ADMIN_WITH_ALL_PERMISSIONS=ON -D WDA_PARAMETER_PROVIDER=ON -D WDA_ENABLE_SUBDEVICES=OFF
WDX_CORE_MAKE_ENV  := $(CROSS_ENV)

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(STATEDIR)/wdx-core.prepare:
	@$(call targetinfo)
	@$(call touch)
endif

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------
ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(STATEDIR)/wdx-core.compile:
	@$(call targetinfo)
	@$(call touch)
endif

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------
$(STATEDIR)/wdx-core.install:
	@$(call targetinfo)

ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
# "normal" mode: call "install"
	@$(call world/install, WDX_CORE)

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_RELEASE
# save install directory contents for BSP
	@mkdir -p $(WDX_CORE_PLATFORMCONFIGPACKAGEDIR) && \
	  cd $(WDX_CORE_PKGDIR) && tar cvzf $(WDX_CORE_PLATFORMCONFIGPACKAGEDIR)/$(WDX_CORE_PACKAGE_NAME).tgz *
endif

else
# BSP mode: install ipk contents to packages/<project name>
	@mkdir -p $(WDX_CORE_PKGDIR) && \
	  tar xvzf $(WDX_CORE_PLATFORMCONFIGPACKAGEDIR)/$(WDX_CORE_PACKAGE_NAME).tgz -C $(WDX_CORE_PKGDIR)
endif

	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------
$(STATEDIR)/wdx-core.targetinstall:
	@$(call targetinfo)

	@$(call install_init, wdx-core)
	@$(call install_fixup, wdx-core,PRIORITY,optional)
	@$(call install_fixup, wdx-core,SECTION,base)
	@$(call install_fixup, wdx-core,AUTHOR,"PEn - WAGO GmbH \& Co. KG")
	@$(call install_fixup, wdx-core,DESCRIPTION,missing)

	@$(call install_finish, wdx-core)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------
# Use default

# vim: syntax=make
