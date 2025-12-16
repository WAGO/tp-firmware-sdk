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
# Makefile for PTXdist package wdx-linux.
#
# For further information about the PTXdist project and license conditions
# see the PTXdist README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_WDX_LINUX) += wdx-linux

#
# Paths and names
#
WDX_LINUX_BASE           := wdx-linux
WDX_LINUX_VERSION        := 2.0.9
WDX_LINUX_MD5            :=
WDX_LINUX                := $(WDX_LINUX_BASE)
WDX_LINUX_GIT_URL        := git@svgithub01001.wago.local:BU-Automation/wdx-linux
WDX_LINUX_URL            := file://$(WDX_LINUX_SRC_DIR)
WDX_LINUX_GIT_BRANCH     := $(WDX_LINUX_VERSION)
WDX_LINUX_BUILDCONFIG    := Release
WDX_LINUX_REPO_SRC_DIR   := $(PTXDIST_WORKSPACE)/repo_src
WDX_LINUX_SRC_DIR        := $(WDX_LINUX_REPO_SRC_DIR)/$(WDX_LINUX)
WDX_LINUX_DIR            := $(BUILDDIR)/$(WDX_LINUX)/src
WDX_LINUX_LICENSE        := MPL-2.0
WDX_LINUX_BIN            := 

WDX_LINUX_PACKAGE_NAME             := $(WDX_LINUX_BASE)_$(WDX_LINUX_VERSION)_$(PTXDIST_IPKG_ARCH_STRING)
WDX_LINUX_PLATFORMCONFIGPACKAGEDIR := $(PTXDIST_PLATFORMCONFIGDIR)/packages

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------
ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(WDX_LINUX_SRC_DIR):
	{ mkdir -p $(WDX_LINUX_REPO_SRC_DIR) && cd $(WDX_LINUX_REPO_SRC_DIR) && git clone $(WDX_LINUX_GIT_URL) $(WDX_LINUX_SRC_DIR); } \
	  || (rm -rf $(WDX_LINUX_SRC_DIR) && false)

$(STATEDIR)/wdx-linux.get: | $(WDX_LINUX_SRC_DIR)
	{ cd $(WDX_LINUX_SRC_DIR) && git fetch -pP && git checkout $(WDX_LINUX_GIT_BRANCH); } \
	  || (rm -rf $(WDX_LINUX_SRC_DIR) && false)
else
$(STATEDIR)/wdx-linux.get:
endif
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------
$(STATEDIR)/wdx-linux.extract: 
	@$(call targetinfo)
ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
	@$(call clean, WDX_LINUX_DIR)
	@mkdir -p $(WDX_LINUX_DIR) && cp -r $(WDX_LINUX_SRC_DIR)/* $(WDX_LINUX_DIR)
	@$(call patchin, WDX_LINUX)
endif
	@$(call touch)

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(STATEDIR)/wdx-linux.extract.post:
	@$(call targetinfo)
	@$(call touch)
endif

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------
WDX_LINUX_PATH      := PATH=$(CROSS_PATH)
WDX_LINUX_CONF_TOOL := cmake
WDX_LINUX_CONF_OPT  := $(CROSS_CMAKE_USR) -D CMAKE_BUILD_TYPE=$(WDX_LINUX_BUILDCONFIG) -D WITHOUT_TEST:BOOL=ON
WDX_LINUX_MAKE_ENV  := $(CROSS_ENV)

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(STATEDIR)/wdx-linux.prepare:
	@$(call targetinfo)
	@$(call touch)
endif

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------
ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(STATEDIR)/wdx-linux.compile:
	@$(call targetinfo)
	@$(call touch)
endif

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------
$(STATEDIR)/wdx-linux.install:
	@$(call targetinfo)

ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
# "normal" mode: call "install"
	@$(call world/install, WDX_LINUX)

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_RELEASE
# save install directory contents for BSP
	@mkdir -p $(WDX_LINUX_PLATFORMCONFIGPACKAGEDIR) && \
	  cd $(WDX_LINUX_PKGDIR) && tar cvzf $(WDX_LINUX_PLATFORMCONFIGPACKAGEDIR)/$(WDX_LINUX_PACKAGE_NAME).tgz *
endif

else
# BSP mode: install ipk contents to packages/<project name>
	@mkdir -p $(WDX_LINUX_PKGDIR) && \
	  tar xvzf $(WDX_LINUX_PLATFORMCONFIGPACKAGEDIR)/$(WDX_LINUX_PACKAGE_NAME).tgz -C $(WDX_LINUX_PKGDIR)
endif

	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------
$(STATEDIR)/wdx-linux.targetinstall:
	@$(call targetinfo)

	@$(call install_init, wdx-linux)
	@$(call install_fixup, wdx-linux,PRIORITY,optional)
	@$(call install_fixup, wdx-linux,SECTION,base)
	@$(call install_fixup, wdx-linux,AUTHOR,"PEn - WAGO GmbH \& Co. KG")
	@$(call install_fixup, wdx-linux,DESCRIPTION,missing)

ifdef PTXCONF_WDX_LINUX_COM_LIB
	@$(call install_lib, wdx-linux, 0, 0, 0644, libwdxlinuxoscom)
endif

ifdef PTXCONF_WDX_LINUX_SERVICE_DAEMON
	@$(call install_copy,        wdx-linux, 0,   0, 0750, -, /usr/sbin/paramd)
	@$(call install_copy,        wdx-linux, 0, 121, 0770,    /etc/paramd)
	@$(call install_alternative, wdx-linux, 0, 121, 0660,    /etc/paramd/paramd.conf)
	@$(call install_copy,        wdx-linux, 0, 124, 0770,    /etc/authd)
	@$(call install_copy,        wdx-linux, 0, 124, 0770,    /etc/authd/clients)
	@$(call install_copy,        wdx-linux, 0, 124, 0770,    /etc/authd/resource_servers)
	@$(call install_alternative, wdx-linux, 0, 124, 0660,    /etc/authd/clients/paramd.conf)
	@$(call install_alternative, wdx-linux, 0, 124, 0660,    /etc/authd/resource_servers/wda.conf)
	@$(call install_alternative, wdx-linux, 0,   0, 0750,    /etc/config-tools/get_wda)
	@$(call install_alternative, wdx-linux, 0,   0, 0750,    /etc/config-tools/config_wda)
	@$(call install_alternative, wdx-linux, 0,   0, 0750,    /etc/config-tools/backup-restore/parameter_service)

# Install sudoers.d file
	@$(call install_alternative, wdx-linux, 0,   0, 0444,    /etc/sudoers.d/config_wda)

# busybox init
ifdef PTXCONF_INITMETHOD_BBINIT
ifdef PTXCONF_WDX_LINUX_SERVICE_DAEMON_STARTSCRIPT
	@$(call install_alternative, wdx-linux, 0,   0, 0755,    /etc/init.d/paramd)

ifneq ($(call remove_quotes, $(PTXCONF_WDX_LINUX_SERVICE_DAEMON_BBINIT_LINK)),)
	@$(call install_link, wdx-linux, \
	  /etc/init.d/paramd, \
	  /etc/rc.d/$(PTXCONF_WDX_LINUX_SERVICE_DAEMON_BBINIT_LINK))
endif
endif # PTXCONF_WDX_LINUX_SERVICE_DAEMON_STARTSCRIPT
endif # PTXCONF_INITMETHOD_BBINIT
endif # PTXCONF_WDX_LINUX_SERVICE_DAEMON

ifdef PTXCONF_WDX_LINUX_SERVICE_LIGHTTPD_INTEGRATION
	@$(call install_alternative, wdx-linux, 0,   0, 0600,    /etc/lighttpd/apps.confd/param_service.conf)
	@$(call install_link, wdx-linux, ../../snippets/cors_policies/all_origins_and_null.conf, \
		/etc/lighttpd/apps.confd/param_service/cors_policy.conf)
endif

	@$(call install_finish, wdx-linux)
	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------
# Use default

# vim: syntax=make
