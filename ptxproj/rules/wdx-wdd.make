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
# Makefile for PTXdist package wdx-wdd.
#
# For further information about the PTXdist project and license conditions
# see the PTXdist README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_WDX_WDD) += wdx-wdd

#
# Paths and names
#
WDX_WDD_BASE           := wdx-wdd
WDX_WDD_VERSION        := 2.0.0
WDX_WDD_MD5            :=
WDX_WDD                := wago-$(WDX_WDD_BASE)-$(WDX_WDD_VERSION)
WDX_WDD_BUILDCONFIG    := Release
WDX_WDD_SRC_DIR        := $(call ptx/in-path, PTXDIST_PATH, wago_intern/$(WDX_WDD_BASE))
WDX_WDD_BUILDROOT_DIR  := $(BUILDDIR)/$(WDX_WDD)
WDX_WDD_DIR            := $(WDX_WDD_BUILDROOT_DIR)/src
WDX_WDD_BUILD_DIR      := $(WDX_WDD_BUILDROOT_DIR)/bin
WDX_WDD_INCLUDE_DIR    := /usr/include
WDX_WDD_LICENSE        := WAGO
WDX_WDD_BIN            := 

WDX_WDD_SCRIPT_DIR     := $(WDX_WDD_SRC_DIR)/script
WDX_WDD_TEMPLATE_DIR   := $(WDX_WDD_SRC_DIR)/wdd/template/$(PTXCONF_PLATFORM)
WDX_WDD_FRAGMENT_DIR   := $(WDX_WDD_SRC_DIR)/wdd/fragment
WDX_WDD_OUTPUT_DIR     := $(WDX_WDD_BUILDROOT_DIR)/wdd

WDX_WDD_PACKAGE_NAME             := $(WDX_WDD_BASE)_$(WDX_WDD_VERSION)_$(PTXDIST_IPKG_ARCH_STRING)
WDX_WDD_PLATFORMCONFIGPACKAGEDIR := $(PTXDIST_PLATFORMCONFIGDIR)/packages

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------
$(STATEDIR)/wdx-wdd.get:
	@$(call touch)

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------
$(STATEDIR)/wdx-wdd.extract:
	@$(call targetinfo)
	@mkdir -p $(WDX_WDD_DIR)
ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
	# copy local wdx source directory
	@cp -r $(WDX_WDD_SRC_DIR)/* $(WDX_WDD_DIR);
endif
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------
WDX_WDD_PATH      := PATH=$(CROSS_PATH)
WDX_WDD_CONF_TOOL := NO

$(STATEDIR)/wdx-wdd.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------
$(STATEDIR)/wdx-wdd.compile:
	@$(call targetinfo)
ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
	@mkdir -p $(WDX_WDD_BUILDROOT_DIR)/wdd && \
	  $(HOSTPYTHON3) $(WDX_WDD_SCRIPT_DIR)/generate_wdd.py -v -p \
	    -t "$(WDX_WDD_TEMPLATE_DIR)" \
	    -f "$(WDX_WDD_FRAGMENT_DIR)" \
	    -o "$(WDX_WDD_OUTPUT_DIR)"
endif
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------
$(STATEDIR)/wdx-wdd.install:
	@$(call targetinfo)

ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
# "normal" mode: install model files into target directories
	@mkdir -p $(WDX_WDD_PKGDIR)/$(PTXCONF_WDX_WDD_DIR) && \
	  cp $(WDX_WDD_BUILDROOT_DIR)/wdd/*.wdd.json $(WDX_WDD_PKGDIR)$(PTXCONF_WDX_WDD_DIR)/ && \
	  chmod 644 $(WDX_WDD_PKGDIR)$(PTXCONF_WDX_WDD_DIR)/*.wdd.json

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_RELEASE
# save install directory to tgz for BSP mode
# avoid to install header files in BSP mode: for internal use only
	@mkdir -p $(WDX_WDD_PLATFORMCONFIGPACKAGEDIR) && \
	  cd $(WDX_WDD_PKGDIR) && tar cvzf $(WDX_WDD_PLATFORMCONFIGPACKAGEDIR)/$(WDX_WDD_PACKAGE_NAME).tgz --exclude=usr/include/* *
endif

else
# BSP mode: install ipk contents to packages/<project name>
	@mkdir -p $(WDX_WDD_PKGDIR) && \
	  tar xvzf $(WDX_WDD_PLATFORMCONFIGPACKAGEDIR)/$(WDX_WDD_PACKAGE_NAME).tgz -C $(WDX_WDD_PKGDIR)

endif

	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------
$(STATEDIR)/wdx-wdd.targetinstall:
	@$(call targetinfo)
	@$(call install_init, wdx-wdd)
	@$(call install_fixup, wdx-wdd,PRIORITY,optional)
	@$(call install_fixup, wdx-wdd,SECTION,base)
	@$(call install_fixup, wdx-wdd,AUTHOR,"MaHe - WAGO GmbH \& Co. KG")
	@$(call install_fixup, wdx-wdd,DESCRIPTION,missing)

	# copy WDDs to target directories
	@$(call install_copy, wdx-wdd, 0, 0, 0755, $(PTXCONF_WDX_WDD_DIR))
	@$(call install_glob, wdx-wdd, 0, 0, -,    $(PTXCONF_WDX_WDD_DIR), *.wdd.json)

	@$(call install_finish, wdx-wdd)
	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/wdx-wdd.clean:
	@$(call targetinfo)
	rm -rf $(PTXCONF_SYSROOT_TARGET)$(PTXCONF_WDX_WDD_DIR)
	@$(call clean_pkg, WDX_WDD)
	@rm -rf $(WDX_WDD_BUILDROOT_DIR)

# vim: syntax=make
