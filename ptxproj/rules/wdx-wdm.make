#######################################################################################################################
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2021-2025 WAGO GmbH & Co. KG
#
# Contributors:
#   MaHe: WAGO GmbH & Co. KG
#   PEn:  WAGO GmbH & Co. KG
#######################################################################################################################
# Makefile for PTXdist package wdx-wdm.
#
# For further information about the PTXdist project and license conditions
# see the PTXdist README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_WDX_WDM) += wdx-wdm

#
# Paths and names
#
WDX_WDM_BASE           := wdx-wdm
WDX_WDM_VERSION        := 1.24.0
WDX_WDM                := $(WDX_WDM_BASE)-$(WDX_WDM_VERSION)
WDX_WDM_SUFFIX         := tar.gz
WDX_WDM_BUILDCONFIG    := Release
WDX_WDM_DIR            := $(BUILDDIR)/$(WDX_WDM)
WDX_WDM_BUILD_DIR      := $(WDX_WDM_DIR)/bin
WDX_WDM_INCLUDE_DIR    := /usr/include
WDX_WDM_LICENSE        := MPL-2.0
WDX_WDM_BIN            :=

WDX_WDM_PACKAGE_NAME             := $(WDX_WDM_BASE)_$(WDX_WDM_VERSION)_$(PTXDIST_IPKG_ARCH_STRING)
WDX_WDM_PLATFORMCONFIGPACKAGEDIR := $(PTXDIST_PLATFORMCONFIGDIR)/packages

# WDx artifacts
WDX_WDM_URL            := $(call jfrog_template_to_url, WDX_WDM)
WDX_WDM_SUFFIX         := $(suffix $(WDX_WDM_URL))
WDX_WDM_MD5             = $(shell [ -f $(WDX_WDM_MD5_FILE) ] && cat $(WDX_WDM_MD5_FILE))
WDX_WDM_SOURCE_BASEDIR := wago_intern/artifactory_sources
WDX_WDM_MD5_FILE       := $(WDX_WDM_SOURCE_BASEDIR)/$(WDX_WDM)$(WDX_WDM_SUFFIX).md5
WDX_WDM_ARTIFACT        = $(call jfrog_get_filename,$(WDX_WDM_URL))

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------
ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(STATEDIR)/wdx-wdm.get:
	# download WDx files from artifactory
	$(call ptx/in-path, PTXDIST_PATH, scripts/wago/artifactory.sh) fetch \
	  '$(WDX_WDM_URL)' $(WDX_WDM_SOURCE_BASEDIR)/$(WDX_WDM)$(WDX_WDM_SUFFIX) '$(WDX_WDM_MD5_FILE)'
	@$(call touch)

else
$(STATEDIR)/wdx-wdm.get:
	@$(call touch)

endif

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------
$(STATEDIR)/wdx-wdm.extract:
	@$(call targetinfo)
	@mkdir -p $(WDX_WDM_DIR)
ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
	# extract downloaded artifacts
	@unzip -o $(WDX_WDM_SOURCE_BASEDIR)/$(WDX_WDM)$(WDX_WDM_SUFFIX) \
	  -d $(WDX_WDM_DIR)
endif
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------
WDX_WDM_PATH      := PATH=$(CROSS_PATH)
WDX_WDM_CONF_TOOL := NO

$(STATEDIR)/wdx-wdm.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------
$(STATEDIR)/wdx-wdm.compile:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------
$(STATEDIR)/wdx-wdm.install:
	@$(call targetinfo)

ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
# "normal" mode: install model files into target directories
	@mkdir -p $(WDX_WDM_PKGDIR)/$(PTXCONF_WDX_WDM_DIR) && \
	  cp $(WDX_WDM_DIR)/*.wdm.json $(WDX_WDM_PKGDIR)$(PTXCONF_WDX_WDM_DIR)/ && \
	  chmod 644 $(WDX_WDM_PKGDIR)$(PTXCONF_WDX_WDM_DIR)/*.wdm.json

ifdef PTXCONF_WDX_WDM_CPP_HEADERS
	@mkdir -p $(WDX_WDM_PKGDIR)/$(WDX_WDM_INCLUDE_DIR) && \
	  cp -r $(WDX_WDM_DIR)/include/* $(WDX_WDM_PKGDIR)$(WDX_WDM_INCLUDE_DIR)/
endif

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_RELEASE
# save install directory to tgz for BSP mode
# avoid to install header files in BSP mode: for internal use only
	@mkdir -p $(WDX_WDM_PLATFORMCONFIGPACKAGEDIR) && \
	  cd $(WDX_WDM_PKGDIR) && tar cvzf $(WDX_WDM_PLATFORMCONFIGPACKAGEDIR)/$(WDX_WDM_PACKAGE_NAME).tgz --exclude=usr/include/* *
endif

else
# BSP mode: install ipk contents to packages/<project name>
	@mkdir -p $(WDX_WDM_PKGDIR) && \
	  tar xvzf $(WDX_WDM_PLATFORMCONFIGPACKAGEDIR)/$(WDX_WDM_PACKAGE_NAME).tgz -C $(WDX_WDM_PKGDIR)

endif

	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------
$(STATEDIR)/wdx-wdm.targetinstall:
	@$(call targetinfo)
	@$(call install_init, wdx-wdm)
	@$(call install_fixup, wdx-wdm,PRIORITY,optional)
	@$(call install_fixup, wdx-wdm,SECTION,base)
	@$(call install_fixup, wdx-wdm,AUTHOR,"MaHe - WAGO GmbH \& Co. KG")
	@$(call install_fixup, wdx-wdm,DESCRIPTION,missing)

	# copy WDM to target directories
	@$(call install_copy, wdx-wdm, 0, 0, 0755, $(PTXCONF_WDX_WDM_DIR))
	@$(call install_glob, wdx-wdm, 0, 0, -,    $(PTXCONF_WDX_WDM_DIR), *.wdm.json)

	# create WDE target directory
	@$(call install_copy, wdx-wdm, 0, 0, 0755, $(PTXCONF_WDX_WDE_DIR))

	@$(call install_finish, wdx-wdm)
	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------
$(STATEDIR)/wdx-wdm.clean:
	@$(call targetinfo)
	rm -rf $(PTXCONF_SYSROOT_TARGET)$(PTXCONF_WDX_WDM_DIR)
	@$(call clean_pkg, WDX_WDM)
	@rm -rf $(WDX_WDM_DIR)

# vim: syntax=make
