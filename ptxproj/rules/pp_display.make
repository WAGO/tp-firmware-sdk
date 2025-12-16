# -*-makefile-*-
#
# Copyright (C) 2021 by WAGO GmbH \& Co. KG
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_PP_DISPLAY) += pp_display

#
# Paths and names
#
PP_DISPLAY_VERSION        := 1.0.0
PP_DISPLAY_MD5            :=
PP_DISPLAY_BASE           := pp_display
PP_DISPLAY                := pp_display-$(PP_DISPLAY_VERSION)
PP_DISPLAY_BUILDCONFIG    := Release
PP_DISPLAY_SRC_DIR        := $(PTXDIST_WORKSPACE)/wago_intern/$(PP_DISPLAY_BASE)
PP_DISPLAY_BUILDROOT_DIR  := $(BUILDDIR)/$(PP_DISPLAY)
PP_DISPLAY_DIR            := $(PP_DISPLAY_BUILDROOT_DIR)/src
PP_DISPLAY_BIN_DIR        := $(PP_DISPLAY_BUILDROOT_DIR)/bin/$(PP_DISPLAY_BUILDCONFIG)
PP_DISPLAY_LICENSE        := WAGO
PP_DISPLAY_MAKE_ENV       := $(CROSS_ENV) \
BUILDCONFIG=$(PP_DISPLAY_BUILDCONFIG) \
BIN_DIR=$(PP_DISPLAY_BIN_DIR) \
SCRIPT_DIR=$(PTXDIST_SYSROOT_HOST)/usr/lib/ct-build

PP_DISPLAY_PACKAGE_NAME             := $(PP_DISPLAY_BASE)_$(PP_DISPLAY_VERSION)_$(PTXDIST_IPKG_ARCH_STRING)
PP_DISPLAY_PLATFORMCONFIGPACKAGEDIR := $(PTXDIST_PLATFORMCONFIGDIR)/packages

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------


$(STATEDIR)/pp_display.extract: 
	@$(call targetinfo)
	@mkdir -p $(PP_DISPLAY_BUILDROOT_DIR)
ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
	@if [ ! -L $(PP_DISPLAY_DIR) ]; then \
	  ln -s $(PP_DISPLAY_SRC_DIR) $(PP_DISPLAY_DIR); \
	fi
endif
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

PP_DISPLAY_CONF_TOOL	:= NO

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(STATEDIR)/pp_display.prepare:
	@$(call targetinfo)
	@$(call touch)
endif

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(STATEDIR)/pp_display.compile:
	@$(call targetinfo)
	@$(call touch)
endif

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/pp_display.install:
	@$(call targetinfo)
	
ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
# "normal" mode: call "install"
	@$(call world/install, PP_DISPLAY)

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_RELEASE
# save install directory to tgz for BSP mode
	@mkdir -p $(PP_DISPLAY_PLATFORMCONFIGPACKAGEDIR) && \
	  cd $(PP_DISPLAY_PKGDIR) && tar cvzf $(PP_DISPLAY_PLATFORMCONFIGPACKAGEDIR)/$(PP_DISPLAY_PACKAGE_NAME).tgz *
endif

else
# BSP mode: install ipk contents to packages/<project name>
	@mkdir -p $(PP_DISPLAY_PKGDIR) && \
	  tar xvzf $(PP_DISPLAY_PLATFORMCONFIGPACKAGEDIR)/$(PP_DISPLAY_PACKAGE_NAME).tgz -C $(PP_DISPLAY_PKGDIR)

endif

	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/pp_display.targetinstall:
	@$(call targetinfo)

	@$(call install_init, pp_display)
	@$(call install_fixup, pp_display,PRIORITY,optional)
	@$(call install_fixup, pp_display,SECTION,base)
	@$(call install_fixup, pp_display,AUTHOR,"PEn - WAGO GmbH \& Co. KG")
	@$(call install_fixup, pp_display,DESCRIPTION,missing)

ifdef PTXCONF_PP_DISPLAY_DAEMON
	@$(call install_copy, pp_display, 0, 0, 0750, -, /usr/sbin/pp_display)

# busybox init
ifdef PTXCONF_INITMETHOD_BBINIT
ifdef PTXCONF_PP_DISPLAY_DAEMON
	@$(call install_copy, pp_display, 0, 0, 0700, -, /etc/init.d/pp_display)

ifneq ($(call remove_quotes, $(PTXCONF_PP_DISPLAY_DAEMON_BBINIT_LINK)),)
	@$(call install_link, pp_display, \
	  /etc/init.d/pp_display, \
	  /etc/rc.d/$(PTXCONF_PP_DISPLAY_DAEMON_BBINIT_LINK))
endif
endif # ifdef PTXCONF_PP_DISPLAY_DAEMON
endif # PTXCONF_INITMETHOD_BBINIT

endif

	@$(call install_finish, pp_display)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/pp_display.clean:
	@$(call targetinfo)
	@if [ -d $(PP_DISPLAY_DIR) ]; then \
	  $(PP_DISPLAY_MAKE_ENV) $(PP_DISPLAY_PATH) $(MAKE) $(MFLAGS) -C $(PP_DISPLAY_DIR) clean; \
	fi
	@$(call clean_pkg, PP_DISPLAY)
	@rm -rf $(PP_DISPLAY_BUILDROOT_DIR)

# vim: syntax=make
