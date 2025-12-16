#######################################################################################################################
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2022 WAGO GmbH & Co. KG
#
# Contributors:
#   MaHe: WAGO GmbH & Co. KG
#######################################################################################################################
# Makefile for PTXdist package wda-openapi.
#
# For further information about the PTXdist project and license conditions
# see the PTXdist README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_WDA_OPENAPI) += wda-openapi

#
# Paths and names
#
WDA_OPENAPI_VERSION        := 1.5.2
WDA_OPENAPI                := wda-openapi-$(WDA_OPENAPI_VERSION)
WDA_OPENAPI_URL            := $(call jfrog_template_to_url, WDA_OPENAPI)
WDA_OPENAPI_SUFFIX         := $(suffix $(WDA_OPENAPI_URL))
WDA_OPENAPI_SOURCE         := $(SRCDIR)/$(WDA_OPENAPI)$(WDA_OPENAPI_SUFFIX)
WDA_OPENAPI_MD5             = $(shell [ -f $(WDA_OPENAPI_MD5_FILE) ] && cat $(WDA_OPENAPI_MD5_FILE))
WDA_OPENAPI_MD5_FILE       := $(WDA_OPENAPI_SOURCE).md5
WDA_OPENAPI_ARTIFACT        = $(call jfrog_get_filename,$(WDA_OPENAPI_URL))

WDA_OPENAPI_DIR            := $(BUILDDIR)/wda-openapi
WDA_OPENAPI_TARGET_DIR     := /var/www/openapi

WDA_OPENAPI_PACKAGE_NAME := wda-openapi_$(WDA_OPENAPI_VERSION)_$(PTXDIST_IPKG_ARCH_STRING)
WDA_OPENAPI_PLATFORMCONFIGPACKAGEDIR := $(PTXDIST_PLATFORMCONFIGDIR)/packages

WDA_OPENAPI_LICENSE        := unknown
WDA_OPENAPI_MAKE_ENV       :=


# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

$(WDA_OPENAPI_SOURCE):
	@$(call targetinfo)
	$(call ptx/in-path, PTXDIST_PATH, scripts/wago/artifactory.sh) fetch \
        '$(WDA_OPENAPI_URL)' '$@' '$(WDA_OPENAPI_MD5_FILE)'

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

# use ptxdist default

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/wda-openapi.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/wda-openapi.compile:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wda-openapi.install:
	@$(call targetinfo)

ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
# "normal" mode: install static files into target directories

	@mkdir -p $(WDA_OPENAPI_PKGDIR)/$(WDA_OPENAPI_TARGET_DIR)
	@cp $(WDA_OPENAPI_DIR)/dist/wda.openapi.json $(WDA_OPENAPI_PKGDIR)$(WDA_OPENAPI_TARGET_DIR)/
ifdef PTXCONF_WDA_OPENAPI_HTML
	@cp $(WDA_OPENAPI_DIR)/dist/wda.openapi.html $(WDA_OPENAPI_PKGDIR)$(WDA_OPENAPI_TARGET_DIR)/
endif
	@chmod 644 $(WDA_OPENAPI_PKGDIR)$(WDA_OPENAPI_TARGET_DIR)/*

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_RELEASE
# save install directory to tgz for BSP mode
# avoid to install header files in BSP mode: for internal use only

	@mkdir -p $(WDA_OPENAPI_PLATFORMCONFIGPACKAGEDIR) && \
	  cd $(WDA_OPENAPI_PKGDIR) && tar cvzf $(WDA_OPENAPI_PLATFORMCONFIGPACKAGEDIR)/$(WDA_OPENAPI_PACKAGE_NAME).tgz *

endif

else
# BSP mode: install ipk contents to packages/<project name>
	@mkdir -p $(WDA_OPENAPI_PKGDIR) && \
	  tar xvzf $(WDA_OPENAPI_PLATFORMCONFIGPACKAGEDIR)/$(WDA_OPENAPI_PACKAGE_NAME).tgz -C $(WDA_OPENAPI_PKGDIR)

endif

	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wda-openapi.targetinstall:
	@$(call targetinfo)

	@$(call install_init, wda-openapi)
	@$(call install_fixup, wda-openapi, PRIORITY, optional)
	@$(call install_fixup, wda-openapi, SECTION, base)
	@$(call install_fixup, wda-openapi, AUTHOR,"Marius Hellmeier, WAGO GmbH \& Co. KG")
	@$(call install_fixup, wda-openapi, DESCRIPTION, missing)

	# install definition file and html file
	@$(call install_copy, wda-openapi, 0, 0, 0755, $(WDA_OPENAPI_TARGET_DIR))
	@$(call install_glob, wda-openapi, 0, 0, -, $(WDA_OPENAPI_TARGET_DIR), *)

	# add symlink to license file
	@$(call install_link, wda-openapi, /usr/share/licenses/wago.txt, $(WDA_OPENAPI_TARGET_DIR)/wda.license.txt)

	@$(call install_finish, wda-openapi)
	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/wda-openapi.clean:
	@$(call targetinfo)
	@rm -rf $(PTXCONF_SYSROOT_TARGET)$(WDA_OPENAPI_TARGET_DIR)
	@$(call clean_pkg, WDA_OPENAPI)
	@rm -rf $(WDA_OPENAPI_DIR)

# vim: syntax=make
