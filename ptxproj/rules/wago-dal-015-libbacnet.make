# -*-makefile-*-
#
# Copyright (C) 2012 by <WAGO GmbH & Co. KG>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
#--- provided package --------------------------------------------------------
#
PACKAGES-$(PTXCONF_LIBBACNET) += libbacnet
#
#--- paths and names --------------------------------------------------------- 
#
LIBBACNET                   := libbacnet
LIBBACNET_VERSION        		:= 2.1.0
LIBBACNET_SO_VERSION        := $(LIBBACNET_VERSION)
LIBBACNET_FOLDER            := libbacnet_git

ifdef PTXCONF_LIBBACNET_SOURCE_DEV
LIBBACNET_GIT_URL           := git@svgithub01001.wago.local:BU-Automation/bacnet-ptxdist-libbacnet.git
endif


LIBBACNET_REL_PATH          := wago_intern/device/bacnet/$(LIBBACNET_FOLDER)
LIBBACNET_SRC_DIR           := $(PTXDIST_WORKSPACE)/$(LIBBACNET_REL_PATH)
LIBBACNET_ENV_VENDOR        := WAGO

ifdef PTXCONF_LIBBACNET_SOURCE_RELEASED
LIBBACNET_URL               := $(call jfrog_template_to_url, LIBBACNET)
else
ifdef PTXCONF_LIBBACNET_ARTIFACTORY_DEV
LIBBACNET_URL               := $(call jfrog_template_to_url, LIBBACNET_ARTIFACTORY_DEV)
else
LIBBACNET_URL               := file://$(LIBBACNET_SRC_DIR)
endif
endif
LIBBACNET_SUFFIX            := $(suffix $(LIBBACNET_URL))
LIBBACNET_MD5                = $(shell [ -f $(LIBBACNET_MD5_FILE) ] && cat $(LIBBACNET_MD5_FILE))
LIBBACNET_MD5_FILE          := wago_intern/artifactory_sources/$(LIBBACNET)$(LIBBACNET_SUFFIX).md5
LIBBACNET_ARTIFACT           = $(call jfrog_get_filename,$(LIBBACNET_URL))
ifdef PTXCONF_LIBBACNET_SOURCE_RELEASED
LIBBACNET_ARCHIVE           := $(LIBBACNET)-$(LIBBACNET_VERSION)$(LIBBACNET_SUFFIX)
endif
ifdef PTXCONF_LIBBACNET_ARTIFACTORY_DEV
LIBBACNET_ARCHIVE           := $(LIBBACNET)-$(LIBBACNET_VERSION)$(LIBBACNET_SUFFIX)
endif

LIBBACNET_BUILDROOT_DIR     := $(BUILDDIR)/$(LIBBACNET)
LIBBACNET_DIR               := $(LIBBACNET_BUILDROOT_DIR)/$(LIBBACNET)
LIBBACNET_LICENSE           := unknown

LIBBACNET_BUILDCONFIG       := Release
LIBBACNET_BUILD_DIR         := $(LIBBACNET_BUILDROOT_DIR)/bin/$(LIBBACNET_BUILDCONFIG)
LIBBACNET_BIN               := $(LIBBACNET).so.$(LIBBACNET_VERSION)
LIBBACNET_SO_NAME           := $(LIBBACNET).so
LIBBACNET_CONF_TOOL         := NO
LIBBACNET_MAKE_ENV          := $(CROSS_ENV) \
                               BUILDCONFIG=$(LIBBACNETSTACK_BUILDCONFIG) \
                               BIN_DIR=$(LIBBACNETSTACK_BUILD_DIR) \
                               TARGET_ARCH=$(PTXCONF_ARCH_STRING) \
                               ARM_ARCH_VERSION=7 \
                               SCRIPT_DIR=$(PTXDIST_SYSROOT_HOST)/lib/ct-build

LIBBACNET_PATH              := PATH=$(CROSS_PATH)
LIBBACNET_PACKAGE_NAME      := $(LIBBACNET)_$(LIBBACNET_VERSION)_$(PTXDIST_IPKG_ARCH_STRING)
LIBBACNET_PLATFORMCONFIGPACKAGEDIR := $(PTXDIST_PLATFORMCONFIGDIR)/packages


# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

ifdef PTXCONF_LIBBACNET_SOURCE_DEV

$(LIBBACNET_SRC_DIR):
	{ cd $(PTXDIST_WORKSPACE)/wago_intern && git clone $(LIBBACNET_GIT_URL) $(LIBBACNET_SRC_DIR); } \
    || rm -fr $(LIBBACNET_SRC_DIR)
ifdef PTXCONF_LIBBACNET_SOURCE_DEV_BRANCH
	{ cd $(LIBBACNET_SRC_DIR) && git checkout $(PTXCONF_LIBBACNET_SOURCE_DEV_BRANCH); } \
    || rm -fr $(LIBBACNET_SRC_DIR)
endif

$(STATEDIR)/libbacnet.get: | $(LIBBACNET_SRC_DIR)

endif

ifdef PTXCONF_LIBBACNET_SOURCE_RELEASED
$(STATEDIR)/libbacnet.get:
	@$(call targetinfo)

ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
	$(call ptx/in-path, PTXDIST_PATH, scripts/wago/artifactory.sh) fetch \
    '$(LIBBACNET_URL)' wago_intern/artifactory_sources/$(LIBBACNET_ARCHIVE) '$(LIBBACNET_MD5_FILE)'
endif

	@$(call touch)
endif

ifdef PTXCONF_LIBBACNET_ARTIFACTORY_DEV
$(STATEDIR)/libbacnet.get:
	@$(call targetinfo)

ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
	$(call ptx/in-path, PTXDIST_PATH, scripts/wago/artifactory.sh) fetch \
    '$(LIBBACNET_URL)' wago_intern/artifactory_sources/$(LIBBACNET_ARCHIVE) '$(LIBBACNET_MD5_FILE)'
endif

	@$(call touch)
endif

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/libbacnet.extract:
	@$(call targetinfo)
	@mkdir -p $(LIBBACNET_BUILDROOT_DIR)
ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
ifdef PTXCONF_LIBBACNET_SOURCE_RELEASED
	@mkdir -p $(LIBBACNET_DIR)
	@tar xvf wago_intern/artifactory_sources/$(LIBBACNET_ARCHIVE) -C $(LIBBACNET_DIR) --strip-components=1
	@$(call patchin, LIBBACNET)
endif
ifdef PTXCONF_LIBBACNET_ARTIFACTORY_DEV
	@mkdir -p $(LIBBACNET_DIR)
	@tar xvf wago_intern/artifactory_sources/$(LIBBACNET_ARCHIVE) -C $(LIBBACNET_DIR) --strip-components=1
	@$(call patchin, LIBBACNET)
else
ifndef PTXCONF_LIBBACNET_SOURCE_RELEASED
	@if [ ! -L $(LIBBACNET_DIR) ]; then \
	  	ln -s $(LIBBACNET_SRC_DIR) $(LIBBACNET_DIR); \
	fi
endif
endif
endif
	@$(call touch)

# ----------------------------------------------------------------------------
# Extract.post
# ----------------------------------------------------------------------------

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(STATEDIR)/libbacnet.extract.post:
	@$(call targetinfo)
	@$(call touch)
endif

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/libbacnet.prepare:
	@$(call targetinfo)
ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
	@$(call world/prepare, LIBBACNET)
endif
	@$(call touch)


# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/libbacnet.compile:
	@$(call targetinfo)

ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
	@$(call world/compile, LIBBACNET)
endif
	@$(call touch)


# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/libbacnet.install:
	@$(call targetinfo)

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
#   BSP mode: install by extracting tgz file
	@mkdir -p $(LIBBACNET_PKGDIR) && \
  	tar xvzf $(LIBBACNET_PLATFORMCONFIGPACKAGEDIR)/$(LIBBACNET_PACKAGE_NAME).tgz -C $(LIBBACNET_PKGDIR)
else	
# 	normal mode, call "make install"	
	
	@$(call world/install, LIBBACNET)
	
ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_RELEASE
#   # save install directory to tgz for BSP mode
	@mkdir -p $(LIBBACNET_PLATFORMCONFIGPACKAGEDIR)
	@cd $(LIBBACNET_PKGDIR) && tar cvzf $(LIBBACNET_PLATFORMCONFIGPACKAGEDIR)/$(LIBBACNET_PACKAGE_NAME).tgz *
endif
endif
	@$(call touch)


# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/libbacnet.targetinstall:
	@$(call targetinfo)
	@$(call install_init, libbacnet)

	@$(call install_fixup, libbacnet,PRIORITY,optional)
	@$(call install_fixup, libbacnet,SECTION,base)
	@$(call install_fixup, libbacnet,AUTHOR,"Rolf Schulenburg <rolf.schulenburg@wago.com>")
	@$(call install_fixup, libbacnet,DESCRIPTION,"BACnet device for the PFC controller" )
	
	@$(call install_lib, libbacnet, 0, 0, 0644, libbacnet)
	@$(call install_link, libbacnet, ../$(LIBBACNET_BIN), /usr/lib/dal/$(LIBBACNET_SO_NAME))
	
	@$(call install_finish, libbacnet)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

PTX_ROOT=$(PTXCONF_SYSROOT_TARGET)/../root
PTX_DAL=$(PTXCONF_SYSROOT_TARGET)/usr/lib/dal

$(STATEDIR)/libbacnet.clean:
	@$(call targetinfo)
	@if [ -d $(LIBBACNET_DIR) ]; then \
		$(LIBBACNET_MAKE_ENV) $(LIBBACNET_PATH) $(MAKE) $(MFLAGS) -C $(LIBBACNET_DIR) clean; \
	fi
	@$(call clean_pkg, LIBBACNET)
	@rm -rf $(LIBBACNET_BUILDROOT_DIR)

# vim: syntax=make

