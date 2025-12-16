# -*-makefile-*-
#
# Copyright (C) 2017 by <PEn>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_LIBCOMMONHEADER) += libcommonheader

#
# Paths and names
#
LIBCOMMONHEADER_BASE           := common-header
LIBCOMMONHEADER_VERSION        := 2.3.0
LIBCOMMONHEADER_MD5            :=
LIBCOMMONHEADER                := $(LIBCOMMONHEADER_BASE)
LIBCOMMONHEADER_GIT_URL        := git@svgithub01001.wago.local:BU-Automation/common-header
LIBCOMMONHEADER_GIT_BRANCH     := $(LIBCOMMONHEADER_VERSION)
LIBCOMMONHEADER_BUILDCONFIG    := Release
LIBCOMMONHEADER_REPO_SRC_DIR   := $(PTXDIST_WORKSPACE)/repo_src
LIBCOMMONHEADER_SRC_DIR        := $(LIBCOMMONHEADER_REPO_SRC_DIR)/$(LIBCOMMONHEADER)
LIBCOMMONHEADER_DIR            := $(BUILDDIR)/$(LIBCOMMONHEADER)
LIBCOMMONHEADER_LICENSE        := MPL-2.0
LIBCOMMONHEADER_BIN            := 
LIBCOMMONHEADER_MAKE_ENV       := $(CROSS_ENV)
LIBCOMMONHEADER_URL            := file://$(LIBCOMMONHEADER_SRC_DIR)

LIBCOMMONHEADER_PACKAGE_NAME             := $(LIBCOMMONHEADER_BASE)_$(LIBCOMMONHEADER_VERSION)_$(PTXDIST_IPKG_ARCH_STRING)
LIBCOMMONHEADER_PLATFORMCONFIGPACKAGEDIR := $(PTXDIST_PLATFORMCONFIGDIR)/packages

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(LIBCOMMONHEADER_SRC_DIR):
	{ mkdir -p $(LIBCOMMONHEADER_REPO_SRC_DIR) && cd $(LIBCOMMONHEADER_REPO_SRC_DIR) && git clone $(LIBCOMMONHEADER_GIT_URL) $(LIBCOMMONHEADER_SRC_DIR); } \
	  || (rm -fr $(LIBCOMMONHEADER_SRC_DIR) && false)

$(STATEDIR)/libcommonheader.get: | $(LIBCOMMONHEADER_SRC_DIR)
	{ cd $(LIBCOMMONHEADER_SRC_DIR) && git checkout $(LIBCOMMONHEADER_GIT_BRANCH); } \
	  || (rm -fr $(LIBCOMMONHEADER_SRC_DIR) && false)
else
$(STATEDIR)/libcommonheader.get:
endif
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/libcommonheader.extract: 
	@$(call targetinfo)
ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
	@$(call clean, LIBCOMMONHEADER_DIR)
	@mkdir -p $(LIBCOMMONHEADER_DIR) && cp -r $(LIBCOMMONHEADER_SRC_DIR)/* $(LIBCOMMONHEADER_DIR)
	@$(call patchin, LIBCOMMONHEADER)
endif
	@$(call touch)

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(STATEDIR)/libcommonheader.extract.post:
	@$(call targetinfo)
	@$(call touch)
endif

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

LIBCOMMONHEADER_CONF_TOOL	:= cmake
LIBCOMMONHEADER_CONF_OPT  := $(CROSS_CMAKE_USR) -D WITHOUT_TEST=ON -D CMAKE_BUILD_TYPE=$(LIBCOMMONHEADER_BUILDCONFIG)
LIBCOMMONHEADER_MAKE_ENV  := $(CROSS_ENV)

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(STATEDIR)/libcommonheader.prepare:
	@$(call targetinfo)
	@$(call touch)
endif

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(STATEDIR)/libcommonheader.compile:
	@$(call targetinfo)
	@$(call touch)
endif

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/libcommonheader.install:
	@$(call targetinfo)

ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
# "normal" mode: call "install"
	@$(call world/install, LIBCOMMONHEADER)

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_RELEASE
# save install directory to tgz for BSP mode
	@mkdir -p $(LIBCOMMONHEADER_PLATFORMCONFIGPACKAGEDIR) && \
	  cd $(LIBCOMMONHEADER_PKGDIR) && tar cvzf $(LIBCOMMONHEADER_PLATFORMCONFIGPACKAGEDIR)/$(LIBCOMMONHEADER_PACKAGE_NAME).tgz *
endif

else
# BSP mode: install ipk contents to packages/<project name>
	@mkdir -p $(LIBCOMMONHEADER_PKGDIR) && \
	  tar xvzf $(LIBCOMMONHEADER_PLATFORMCONFIGPACKAGEDIR)/$(LIBCOMMONHEADER_PACKAGE_NAME).tgz -C $(LIBCOMMONHEADER_PKGDIR)

endif

	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/libcommonheader.targetinstall:
	@$(call targetinfo)

	@$(call install_init, libcommonheader)
	@$(call install_fixup, libcommonheader,PRIORITY,optional)
	@$(call install_fixup, libcommonheader,SECTION,base)
	@$(call install_fixup, libcommonheader,AUTHOR,"<PEn>")
	@$(call install_fixup, libcommonheader,DESCRIPTION,missing)

	@$(call install_finish, libcommonheader)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/libcommonheader.clean:
	@$(call targetinfo)
	rm -rf $(PTXCONF_SYSROOT_TARGET)/usr/include/wc/
	@$(call clean, LIBCOMMONHEADER)
	@$(call clean_pkg, LIBCOMMONHEADER)

# vim: syntax=make
