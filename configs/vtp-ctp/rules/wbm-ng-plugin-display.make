# -*-makefile-*-
#
# Copyright (C) 2017 by Marius Hellmeier (marius.hellmeier@wago.com), WAGO Kontakttechnik GmbH & Co. KG
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_WBM_NG_PLUGIN_DISPLAY) += wbm-ng-plugin-display

#
# Paths and names
#
WBM_NG_PLUGIN_DISPLAY                := wbm-display
WBM_NG_PLUGIN_DISPLAY_VERSION        := 1.2.1-rc.156162409115
WBM_NG_PLUGIN_DISPLAY_SUFFIX         := tgz
WBM_NG_PLUGIN_DISPLAY_ARCHIVE        := $(WBM_NG_PLUGIN_DISPLAY)-$(WBM_NG_PLUGIN_DISPLAY_VERSION).$(WBM_NG_PLUGIN_DISPLAY_SUFFIX)
WBM_NG_PLUGIN_DISPLAY_URL            := http://svsv01003/wago-ptxdist-src/$(WBM_NG_PLUGIN_DISPLAY_ARCHIVE)
WBM_NG_PLUGIN_DISPLAY_MD5            := e79580988035ddfcc2a64472f277ce6f

WBM_NG_PLUGIN_DISPLAY_BUILDROOT_DIR  := $(BUILDDIR)/wbm-ng-plugin-display
WBM_NG_PLUGIN_DISPLAY_SOURCE         := $(SRCDIR)/$(WBM_NG_PLUGIN_DISPLAY_ARCHIVE)
WBM_NG_PLUGIN_DISPLAY_DIR            := $(WBM_NG_PLUGIN_DISPLAY_BUILDROOT_DIR)
WBM_NG_PLUGIN_DISPLAY_LICENSE        := unknown
WBM_NG_PLUGIN_DISPLAY_MAKE_ENV       :=
ifeq ($(PTXCONF_WBM),y)
WBM_NG_PLUGIN_DISPLAY_TARGET_DIR     := /var/www/wbm-ng/plugins/$(WBM_NG_PLUGIN_DISPLAY)
else
WBM_NG_PLUGIN_DISPLAY_TARGET_DIR     := /var/www/wbm/plugins/$(WBM_NG_PLUGIN_DISPLAY)
endif

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

# use ptxdist default

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

# use ptxdist default

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-display.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-display.compile:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-display.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-display.targetinstall:
	@$(call targetinfo)

	@$(call install_init, wbm-ng-plugin-display)
	@$(call install_fixup, wbm-ng-plugin-display, PRIORITY, optional)
	@$(call install_fixup, wbm-ng-plugin-display, SECTION, base)
	@$(call install_fixup, wbm-ng-plugin-display, AUTHOR,"Marius Hellmeier, WAGO Kontakttechnik GmbH & Co. KG")
	@$(call install_fixup, wbm-ng-plugin-display, DESCRIPTION, missing)

	# create target directory itself
	@$(call install_copy, wbm-ng-plugin-display, 0, 0, 0755, $(WBM_NG_PLUGIN_DISPLAY_TARGET_DIR))

	# loop over all files and subdirectories (deep)
	@cd $(WBM_NG_PLUGIN_DISPLAY_DIR) && \
	for object in $$( find ./* -print ); do \
		if test -f $$object; then \
			$(call install_copy, wbm-ng-plugin-display, 0, 0, 0644, $(WBM_NG_PLUGIN_DISPLAY_DIR)/$$object, $(WBM_NG_PLUGIN_DISPLAY_TARGET_DIR)/$$object); \
		elif test -d $$object; then \
			$(call install_copy, wbm-ng-plugin-display, 0, 0, 0755, $(WBM_NG_PLUGIN_DISPLAY_TARGET_DIR)/$$object); \
		fi; \
	done;


	@$(call install_finish, wbm-ng-plugin-display)
	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-display.clean:
	@$(call targetinfo)
	@rm -rf $(PTXCONF_SYSROOT_TARGET)$(WBM_NG_PLUGIN_DISPLAY_TARGET_DIR)
	@$(call clean_pkg, WBM_NG_PLUGIN_DISPLAY)
	@rm -rf $(WBM_NG_PLUGIN_DISPLAY_BUILDROOT_DIR)

# vim: syntax=make
