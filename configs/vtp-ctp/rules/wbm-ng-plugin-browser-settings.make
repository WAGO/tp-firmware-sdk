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
PACKAGES-$(PTXCONF_WBM_NG_PLUGIN_BROWSER_SETTINGS) += wbm-ng-plugin-browser-settings

#
# Paths and names
#
WBM_NG_PLUGIN_BROWSER_SETTINGS                := wbm-browser-settings
WBM_NG_PLUGIN_BROWSER_SETTINGS_VERSION        := 1.2.0-rc.156162409413
WBM_NG_PLUGIN_BROWSER_SETTINGS_SUFFIX         := tgz
WBM_NG_PLUGIN_BROWSER_SETTINGS_ARCHIVE        := $(WBM_NG_PLUGIN_BROWSER_SETTINGS)-$(WBM_NG_PLUGIN_BROWSER_SETTINGS_VERSION).$(WBM_NG_PLUGIN_BROWSER_SETTINGS_SUFFIX)
WBM_NG_PLUGIN_BROWSER_SETTINGS_URL            := http://svsv01003/wago-ptxdist-src/$(WBM_NG_PLUGIN_BROWSER_SETTINGS_ARCHIVE)
WBM_NG_PLUGIN_BROWSER_SETTINGS_MD5            := ce73214208d5b39db9f54dd117d47957

WBM_NG_PLUGIN_BROWSER_SETTINGS_BUILDROOT_DIR  := $(BUILDDIR)/wbm-ng-plugin-browser-settings
WBM_NG_PLUGIN_BROWSER_SETTINGS_SOURCE         := $(SRCDIR)/$(WBM_NG_PLUGIN_BROWSER_SETTINGS_ARCHIVE)
WBM_NG_PLUGIN_BROWSER_SETTINGS_DIR            := $(WBM_NG_PLUGIN_BROWSER_SETTINGS_BUILDROOT_DIR)
WBM_NG_PLUGIN_BROWSER_SETTINGS_LICENSE        := unknown
WBM_NG_PLUGIN_BROWSER_SETTINGS_MAKE_ENV       :=
ifeq ($(PTXCONF_WBM),y)
WBM_NG_PLUGIN_BROWSER_SETTINGS_TARGET_DIR     := /var/www/wbm-ng/plugins/$(WBM_NG_PLUGIN_BROWSER_SETTINGS)
else
WBM_NG_PLUGIN_BROWSER_SETTINGS_TARGET_DIR     := /var/www/wbm/plugins/$(WBM_NG_PLUGIN_BROWSER_SETTINGS)
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

$(STATEDIR)/wbm-ng-plugin-browser-settings.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-browser-settings.compile:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-browser-settings.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-browser-settings.targetinstall:
	@$(call targetinfo)

	@$(call install_init, wbm-ng-plugin-browser-settings)
	@$(call install_fixup, wbm-ng-plugin-browser-settings, PRIORITY, optional)
	@$(call install_fixup, wbm-ng-plugin-browser-settings, SECTION, base)
	@$(call install_fixup, wbm-ng-plugin-browser-settings, AUTHOR,"Marius Hellmeier, WAGO Kontakttechnik GmbH & Co. KG")
	@$(call install_fixup, wbm-ng-plugin-browser-settings, DESCRIPTION, missing)

	# create target directory itself
	@$(call install_copy, wbm-ng-plugin-browser-settings, 0, 0, 0755, $(WBM_NG_PLUGIN_BROWSER_SETTINGS_TARGET_DIR))

	# loop over all files and subdirectories (deep)
	@cd $(WBM_NG_PLUGIN_BROWSER_SETTINGS_DIR) && \
	for object in $$( find ./* -print ); do \
		if test -f $$object; then \
			$(call install_copy, wbm-ng-plugin-browser-settings, 0, 0, 0644, $(WBM_NG_PLUGIN_BROWSER_SETTINGS_DIR)/$$object, $(WBM_NG_PLUGIN_BROWSER_SETTINGS_TARGET_DIR)/$$object); \
		elif test -d $$object; then \
			$(call install_copy, wbm-ng-plugin-browser-settings, 0, 0, 0755, $(WBM_NG_PLUGIN_BROWSER_SETTINGS_TARGET_DIR)/$$object); \
		fi; \
	done;


	@$(call install_finish, wbm-ng-plugin-browser-settings)
	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-browser-settings.clean:
	@$(call targetinfo)
	@rm -rf $(PTXCONF_SYSROOT_TARGET)$(WBM_NG_PLUGIN_BROWSER_SETTINGS_TARGET_DIR)
	@$(call clean_pkg, WBM_NG_PLUGIN_BROWSER_SETTINGS)
	@rm -rf $(WBM_NG_PLUGIN_BROWSER_SETTINGS_BUILDROOT_DIR)

# vim: syntax=make
