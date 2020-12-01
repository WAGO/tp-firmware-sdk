# -*-makefile-*-
#
# Copyright (C) 2020 by <elrest>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_START_MICROBROWSER) += start_microbrowser

#
# Paths and names
#
START_MICROBROWSER_VERSION	:= 1
START_MICROBROWSER_MD5		:=
START_MICROBROWSER		:= start_microbrowser
START_MICROBROWSER_URL		:= file://wago_intern/tools/TP600/$(START_MICROBROWSER)
START_MICROBROWSER_DIR		:= $(BUILDDIR)/$(START_MICROBROWSER)
START_MICROBROWSER_LICENSE	:= unknown

START_MICROBROWSER_PACKAGE_NAME := start_microbrowser_$(START_MICROBROWSER_VERSION)_$(PTXDIST_IPKG_ARCH_STRING)
START_MICROBROWSER_PLATFORMCONFIGPACKAGEDIR := $(PTXDIST_PLATFORMCONFIGDIR)/packages
# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

$(STATEDIR)/start_microbrowser.get:
	@$(call targetinfo)
	@$(call touch)


# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/start_microbrowser.extract:
	@$(call targetinfo)
	mkdir -p $(START_MICROBROWSER_DIR)

ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES	
	# WAGO_TOOLS_BUILD_VERSION_TRUNK | WAGO_TOOLS_BUILD_VERSION_RELEASE
	cp -rd $(PTXDIST_WORKSPACE)/wago_intern/tools/TP600/$(START_MICROBROWSER)/* $(START_MICROBROWSER_DIR)
endif

	@$(call touch)

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(STATEDIR)/start_microbrowser.extract.post:
	@$(call targetinfo)
	@$(call touch)
endif


# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

START_MICROBROWSER_PATH	:= PATH=$(CROSS_PATH)
START_MICROBROWSER_CONF_TOOL	:= NO
START_MICROBROWSER_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/start_microbrowser.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, START_MICROBROWSER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------
$(STATEDIR)/start_microbrowser.compile:
	@$(call targetinfo)

ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
	# WAGO_TOOLS_BUILD_VERSION_TRUNK | WAGO_TOOLS_BUILD_VERSION_RELEASE
	@$(call world/compile, START_MICROBROWSER)
endif

	@$(call touch)

# ----------------------------------------------------------------------------
# Install - Nothing
# ----------------------------------------------------------------------------
$(STATEDIR)/start_microbrowser.install:
	@$(call targetinfo)

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
#   # BSP mode: install ipk contents to packages/start_microbrowser
	@mkdir -p $(START_MICROBROWSER_PKGDIR) && \
    tar xvzf $(START_MICROBROWSER_PLATFORMCONFIGPACKAGEDIR)/$(START_MICROBROWSER_PACKAGE_NAME).tgz -C $(START_MICROBROWSER_PKGDIR)
else

#   # "normal" mode: call "make install"
	@$(call world/install, START_MICROBROWSER)


ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_RELEASE
#   # save install directory contents for BSP
	@cd $(START_MICROBROWSER_PKGDIR) && tar cvzf $(START_MICROBROWSER_PLATFORMCONFIGPACKAGEDIR)/$(START_MICROBROWSER_PACKAGE_NAME).tgz *
endif

endif

	@$(call touch)


# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/start_microbrowser.targetinstall:
	@$(call targetinfo)

	@$(call install_init, start_microbrowser)
	@$(call install_fixup, start_microbrowser,PRIORITY,optional)
	@$(call install_fixup, start_microbrowser,SECTION,base)
	@$(call install_fixup, start_microbrowser,AUTHOR,"<elrest>")
	@$(call install_fixup, start_microbrowser,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, start_microbrowser, 0, 0, 0755, -, /etc/config-tools/start_microbrowser)

	@$(call install_finish, start_microbrowser)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/start_microbrowser.clean:
	@$(call targetinfo)
	@-cd $(START_MICROBROWSER_DIR) && rm -f *.o $(START_MICROBROWSER)	
	@-cd $(START_MICROBROWSER_DIR) && \
		$(START_MICROBROWSER_MAKE_ENV) $(START_MICROBROWSER_PATH) $(MAKE) clean
	@$(call clean_pkg, START_MICROBROWSER)
	@$(call clean, $(START_MICROBROWSER_DIR))
	@rm -rf $(START_MICROBROWSER_DIR)

# vim: syntax=make
