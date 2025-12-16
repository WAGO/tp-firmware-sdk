#######################################################################################################################
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file is part of project auth-service (PTXdist package wago-auth-service).
#
# Copyright (c) 2023-2025 WAGO GmbH & Co. KG
#
# Contributors:
#   PEn: WAGO GmbH & Co. KG
#######################################################################################################################
# Makefile for PTXdist package wago-auth-service.
#
# For further information about the PTXdist project and license conditions
# see the PTXdist README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_WAGO_AUTH_SERVICE) += wago-auth-service

#
# Paths and names
#
WAGO_AUTH_SERVICE_BASE           := auth-service
WAGO_AUTH_SERVICE_VERSION        := 2.3.2
WAGO_AUTH_SERVICE_MD5            :=
WAGO_AUTH_SERVICE                := wago-$(WAGO_AUTH_SERVICE_BASE)
WAGO_AUTH_SERVICE_GIT_URL        := git@svgithub01001.wago.local:BU-Automation/auth-service
WAGO_AUTH_SERVICE_URL            := file://$(WAGO_AUTH_SERVICE_SRC_DIR)
WAGO_AUTH_SERVICE_GIT_BRANCH     := $(WAGO_AUTH_SERVICE_VERSION)
WAGO_AUTH_SERVICE_BUILDCONFIG    := Release
WAGO_AUTH_SERVICE_REPO_SRC_DIR   := $(PTXDIST_WORKSPACE)/repo_src
WAGO_AUTH_SERVICE_SRC_DIR        := $(WAGO_AUTH_SERVICE_REPO_SRC_DIR)/$(WAGO_AUTH_SERVICE)
WAGO_AUTH_SERVICE_DIR            := $(BUILDDIR)/$(WAGO_AUTH_SERVICE)/src
WAGO_AUTH_SERVICE_LICENSE        := MPL-2.0
WAGO_AUTH_SERVICE_BIN            := 

WAGO_AUTH_SERVICE_PACKAGE_NAME             := $(WAGO_AUTH_SERVICE_BASE)_$(WAGO_AUTH_SERVICE_VERSION)_$(PTXDIST_IPKG_ARCH_STRING)
WAGO_AUTH_SERVICE_PLATFORMCONFIGPACKAGEDIR := $(PTXDIST_PLATFORMCONFIGDIR)/packages

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------
ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(WAGO_AUTH_SERVICE_SRC_DIR):
	{ mkdir -p $(WAGO_AUTH_SERVICE_REPO_SRC_DIR) && cd $(WAGO_AUTH_SERVICE_REPO_SRC_DIR) && git clone $(WAGO_AUTH_SERVICE_GIT_URL) $(WAGO_AUTH_SERVICE_SRC_DIR); } \
	  || (rm -rf $(WAGO_AUTH_SERVICE_SRC_DIR) && false)

$(STATEDIR)/wago-auth-service.get: | $(WAGO_AUTH_SERVICE_SRC_DIR)
	{ cd $(WAGO_AUTH_SERVICE_SRC_DIR) && git checkout $(WAGO_AUTH_SERVICE_GIT_BRANCH); } \
	  || (rm -rf $(WAGO_AUTH_SERVICE_SRC_DIR) && false)
else
$(STATEDIR)/wago-auth-service.get:
endif
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------
$(STATEDIR)/wago-auth-service.extract:
	@$(call targetinfo)
ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
	@$(call clean, WAGO_AUTH_SERVICE_DIR)
	@mkdir -p $(WAGO_AUTH_SERVICE_DIR) && cp -r $(WAGO_AUTH_SERVICE_SRC_DIR)/* $(WAGO_AUTH_SERVICE_DIR)
	@$(call patchin, WAGO_AUTH_SERVICE)
endif
	@$(call touch)

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(STATEDIR)/wago-auth-service.extract.post:
	@$(call targetinfo)
	@$(call touch)
endif

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------
WAGO_AUTH_SERVICE_PATH      := PATH=$(CROSS_PATH)
WAGO_AUTH_SERVICE_CONF_TOOL := cmake
WAGO_AUTH_SERVICE_CONF_OPT  := $(CROSS_CMAKE_USR) -D CMAKE_BUILD_TYPE=$(WAGO_AUTH_SERVICE_BUILDCONFIG) \
												  -D WITHOUT_TEST:BOOL=ON \
												  -D WDX_USER_MODEL=PasswordManagement \
												  -D HTML_TEMPLATE_LOCATION=/var/www/auth \
												  -D WWW_LOCATION=/var/www/auth
WAGO_AUTH_SERVICE_MAKE_ENV  := $(CROSS_ENV)

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(STATEDIR)/wago-auth-service.prepare:
	@$(call targetinfo)
	@$(call touch)
endif

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------
ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(STATEDIR)/wago-auth-service.compile:
	@$(call targetinfo)
	@$(call touch)
endif

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------
$(STATEDIR)/wago-auth-service.install:
	@$(call targetinfo)

ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
# "normal" mode: call "install"
	@$(call world/install, WAGO_AUTH_SERVICE)

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_RELEASE
# save install directory contents for BSP
	@mkdir -p $(WAGO_AUTH_SERVICE_PLATFORMCONFIGPACKAGEDIR) && \
	  cd $(WAGO_AUTH_SERVICE_PKGDIR) && tar cvzf $(WAGO_AUTH_SERVICE_PLATFORMCONFIGPACKAGEDIR)/$(WAGO_AUTH_SERVICE_PACKAGE_NAME).tgz *
endif

else
# BSP mode: install ipk contents to packages/<project name>
	@mkdir -p $(WAGO_AUTH_SERVICE_PKGDIR) && \
	  tar xvzf $(WAGO_AUTH_SERVICE_PLATFORMCONFIGPACKAGEDIR)/$(WAGO_AUTH_SERVICE_PACKAGE_NAME).tgz -C $(WAGO_AUTH_SERVICE_PKGDIR)
endif

	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------
$(STATEDIR)/wago-auth-service.targetinstall:
	@$(call targetinfo)

	@$(call install_init, wago-auth-service)
	@$(call install_fixup, wago-auth-service,PRIORITY,optional)
	@$(call install_fixup, wago-auth-service,SECTION,base)
	@$(call install_fixup, wago-auth-service,AUTHOR,"PEn - WAGO GmbH \& Co. KG")
	@$(call install_fixup, wago-auth-service,DESCRIPTION,missing)

ifdef PTXCONF_WAGO_AUTH_SERVICE_LIB
#	@$(call install_lib, wago-auth-service, 0, 0, 0755, libauthserv)
	@$(call install_alternative, wago-auth-service, 0, 0, 0644, /etc/pam.d/authd)
endif

ifdef PTXCONF_WAGO_AUTH_SERVICE_LIGHTTPD_INTEGRATION
	@$(call install_alternative, wago-auth-service, 0, 0, 0600, /etc/lighttpd/apps.confd/auth_service.conf)
endif

ifdef PTXCONF_WAGO_AUTH_SERVICE_DAEMON
	@$(call install_copy,        wago-auth-service, 0,   0, 0750, -, /usr/sbin/authd)
	@$(call install_copy,        wago-auth-service, 0, 124, 0770,    /etc/authd)
	@$(call install_alternative, wago-auth-service, 0, 124, 0660,    /etc/authd/authd.conf)
	@$(call install_copy,        wago-auth-service, 0, 124, 0770,    /etc/authd/clients)
	@$(call install_alternative, wago-auth-service, 0, 124, 0660,    /etc/authd/clients/wdmrdesk.conf)
	@$(call install_copy,        wago-auth-service, 0, 124, 0770,    /etc/authd/resource_servers)
	@$(call install_alternative, wago-auth-service, 0,   0, 0750,    /etc/config-tools/backup-restore/auth_service)

	# Install login web-page
	@$(call install_copy, wago-auth-service, 0, 124, 0775,    /var/www/auth)
	@$(call install_copy, wago-auth-service, 0, 124, 0664, -, /var/www/auth/auth.js)
	@$(call install_copy, wago-auth-service, 0, 124, 0664, -, /var/www/auth/auth.css)
	@$(call install_copy, wago-auth-service, 0, 124, 0660, -, /var/www/auth/password_setup.html.template)
	@$(call install_copy, wago-auth-service, 0, 124, 0660, -, /var/www/auth/login.html.template)
	@$(call install_copy, wago-auth-service, 0, 124, 0664, -, /var/www/auth/login.js)
	@$(call install_copy, wago-auth-service, 0, 124, 0660, -, /var/www/auth/password_change.html.template)
	@$(call install_copy, wago-auth-service, 0, 124, 0664, -, /var/www/auth/password_change.js)
	@$(call install_copy, wago-auth-service, 0, 124, 0664, -, /var/www/auth/confirmation.html.template)
	@$(call install_copy, wago-auth-service, 0, 124, 0775,    /var/www/auth/images)
	@$(call install_copy, wago-auth-service, 0, 124, 0664, -, /var/www/auth/images/favicon.ico)
	@$(call install_copy, wago-auth-service, 0, 124, 0664, -, /var/www/auth/images/spinner.gif)
	@$(call install_copy, wago-auth-service, 0, 124, 0664, -, /var/www/auth/images/wago-logo.svg)
	@$(call install_copy, wago-auth-service, 0, 124, 0664, -, /var/www/auth/images/warning.svg)
	@$(call install_copy, wago-auth-service, 0, 124, 0664, -, /var/www/auth/images/info.svg)
	@$(call install_copy, wago-auth-service, 0, 124, 0664, -, /var/www/auth/images/chevron-left.svg)
	@$(call install_copy, wago-auth-service, 0, 124, 0664, -, /var/www/auth/images/chevron-right.svg)
	@$(call install_copy, wago-auth-service, 0, 124, 0664, -, /var/www/auth/images/btn-close.svg)
	@$(call install_copy, wago-auth-service, 0, 124, 0664, -, /var/www/auth/images/check.svg)


# busybox init
ifdef PTXCONF_INITMETHOD_BBINIT
ifdef PTXCONF_WAGO_AUTH_SERVICE_DAEMON_STARTSCRIPT
	@$(call install_alternative, wago-auth-service, 0, 0, 0755, /etc/init.d/authd)

ifneq ($(call remove_quotes, $(PTXCONF_WAGO_AUTH_SERVICE_DAEMON_BBINIT_LINK)),)
	@$(call install_link, wago-auth-service, \
	  /etc/init.d/authd, \
	  /etc/rc.d/$(PTXCONF_WAGO_AUTH_SERVICE_DAEMON_BBINIT_LINK))
endif
endif # PTXCONF_WAGO_AUTH_SERVICE_DAEMON_STARTSCRIPT
endif # PTXCONF_INITMETHOD_BBINIT
endif # PTXCONF_WAGO_AUTH_SERVICE_DAEMON

	@$(call install_finish, wago-auth-service)
	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------
# Use default

# vim: syntax=make
