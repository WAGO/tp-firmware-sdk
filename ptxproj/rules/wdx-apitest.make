#######################################################################################################################
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2021-2025 WAGO GmbH & Co. KG
#
# Contributors:
#   PEn: WAGO GmbH & Co. KG
#######################################################################################################################
# Makefile for PTXdist package wdx-apitest.
#
# For further information about the PTXdist project and license conditions
# see the PTXdist README file.
#

#
# We provide this package
#
LAZY_PACKAGES-$(PTXCONF_WDX_APITEST) += wdx-apitest

#
# Paths and names
#
WDX_APITEST_BASE           := wdx-apitest
WDX_APITEST_VERSION        := $(WDX_LINUX_VERSION)
WDX_APITEST_MD5            :=
WDX_APITEST                := $(WDX_APITEST_BASE)
WDX_APITEST_URL            := file://$(WDX_APITEST_SRC_DIR)
WDX_APITEST_BUILDCONFIG    := $(WDX_LINUX_BUILDCONFIG)
WDX_APITEST_DIR            := $(BUILDDIR)/$(WDX_LINUX)/src
WDX_APITEST_LICENSE        := MIT
WDX_APITEST_BIN            := 

WDX_APITEST_PACKAGE_NAME             := $(WDX_APITEST_BASE)_$(WDX_APITEST_VERSION)_$(PTXDIST_IPKG_ARCH_STRING)
WDX_APITEST_PLATFORMCONFIGPACKAGEDIR := $(PTXDIST_PLATFORMCONFIGDIR)/packages

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------
$(STATEDIR)/wdx-apitest.get:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------
$(STATEDIR)/wdx-apitest.extract: 
	@$(call targetinfo)
	@$(call touch)

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(STATEDIR)/wdx-apitest.extract.post:
	@$(call targetinfo)
	@$(call touch)
endif

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------
$(STATEDIR)/wdx-apitest.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------
$(STATEDIR)/wdx-apitest.compile:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------
$(STATEDIR)/wdx-apitest.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------
$(STATEDIR)/wdx-apitest.targetinstall:
	@$(call targetinfo)

	@$(call install_init, wdx-apitest)
	@$(call install_fixup, wdx-apitest,PRIORITY,optional)
	@$(call install_fixup, wdx-apitest,SECTION,base)
	@$(call install_fixup, wdx-apitest,AUTHOR,"PEn - WAGO GmbH \& Co. KG")
	@$(call install_fixup, wdx-apitest,DESCRIPTION,missing)

	@$(call install_copy, wdx-apitest, 0, 0, 0755,                    /etc/wdx/wde/disabled)
	@$(call install_copy, wdx-apitest, 0, 0, 0755, \
	  $(BUILDDIR)/../packages/wdx-linux/usr/sbin/apitest,             /usr/sbin/apitest)
	@$(call install_copy, wdx-apitest, 0, 0, 0644, \
	  $(BUILDDIR)/../packages/wdx-linux/etc/wdx/wde/apitest.wdd.json, /etc/wdx/wde/disabled/apitest.wdd.json)
	@$(call install_copy, wdx-apitest, 0, 0, 0644, \
	  $(BUILDDIR)/../packages/wdx-linux/etc/wdx/wde/apitest.wdm.json, /etc/wdx/wde/disabled/apitest.wdm.json)

# busybox init
ifdef PTXCONF_INITMETHOD_BBINIT
	@$(call install_alternative, wdx-apitest, 0,   0, 0700,    /etc/init.d/apitest)
endif # PTXCONF_INITMETHOD_BBINIT

	@$(call install_finish, wdx-apitest)
	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------
$(STATEDIR)/wdx-apitest.clean:
	@$(call targetinfo)

# vim: syntax=make
