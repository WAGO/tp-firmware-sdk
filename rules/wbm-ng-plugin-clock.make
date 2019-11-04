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
PACKAGES-$(PTXCONF_WBM_NG_PLUGIN_CLOCK) += wbm-ng-plugin-clock

#
# Paths and names
#
WBM_NG_PLUGIN_CLOCK                := wbm-clock
WBM_NG_PLUGIN_CLOCK_VERSION        := 1.0.1-rc.156162410411
WBM_NG_PLUGIN_CLOCK_SUFFIX         := tgz
WBM_NG_PLUGIN_CLOCK_ARCHIVE        := $(WBM_NG_PLUGIN_CLOCK)-$(WBM_NG_PLUGIN_CLOCK_VERSION).$(WBM_NG_PLUGIN_CLOCK_SUFFIX)
WBM_NG_PLUGIN_CLOCK_URL            := http://svsv01003/wago-ptxdist-src/$(WBM_NG_PLUGIN_CLOCK_ARCHIVE)
WBM_NG_PLUGIN_CLOCK_MD5            := 63c4e9d22126501819ec9bdda7ef14d1

WBM_NG_PLUGIN_CLOCK_BUILDROOT_DIR  := $(BUILDDIR)/wbm-ng-plugin-clock
WBM_NG_PLUGIN_CLOCK_SOURCE         := $(SRCDIR)/$(WBM_NG_PLUGIN_CLOCK_ARCHIVE)
WBM_NG_PLUGIN_CLOCK_DIR            := $(WBM_NG_PLUGIN_CLOCK_BUILDROOT_DIR)
WBM_NG_PLUGIN_CLOCK_LICENSE        := unknown
WBM_NG_PLUGIN_CLOCK_MAKE_ENV       :=
ifeq ($(PTXCONF_WBM),y)
WBM_NG_PLUGIN_CLOCK_TARGET_DIR     := /var/www/wbm-ng/plugins/$(WBM_NG_PLUGIN_CLOCK)
else
WBM_NG_PLUGIN_CLOCK_TARGET_DIR     := /var/www/wbm/plugins/$(WBM_NG_PLUGIN_CLOCK)
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

$(STATEDIR)/wbm-ng-plugin-clock.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-clock.compile:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-clock.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-clock.targetinstall:
	@$(call targetinfo)

	@$(call install_init, wbm-ng-plugin-clock)
	@$(call install_fixup, wbm-ng-plugin-clock, PRIORITY, optional)
	@$(call install_fixup, wbm-ng-plugin-clock, SECTION, base)
	@$(call install_fixup, wbm-ng-plugin-clock, AUTHOR,"Marius Hellmeier, WAGO Kontakttechnik GmbH \& Co. KG")
	@$(call install_fixup, wbm-ng-plugin-clock, DESCRIPTION, missing)

	# create target directory itself
	@$(call install_copy, wbm-ng-plugin-clock, 0, 0, 0755, $(WBM_NG_PLUGIN_CLOCK_TARGET_DIR))

	# loop over all files and subdirectories (deep)
	@cd $(WBM_NG_PLUGIN_CLOCK_DIR) && \
	for object in $$( find ./* -print ); do \
		if test -f $$object; then \
			$(call install_copy, wbm-ng-plugin-clock, 0, 0, 0644, $(WBM_NG_PLUGIN_CLOCK_DIR)/$$object, $(WBM_NG_PLUGIN_CLOCK_TARGET_DIR)/$$object); \
		elif test -d $$object; then \
			$(call install_copy, wbm-ng-plugin-clock, 0, 0, 0755, $(WBM_NG_PLUGIN_CLOCK_TARGET_DIR)/$$object); \
		fi; \
	done;


	@$(call install_finish, wbm-ng-plugin-clock)
	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-clock.clean:
	@$(call targetinfo)
	@rm -rf $(PTXCONF_SYSROOT_TARGET)$(WBM_NG_PLUGIN_CLOCK_TARGET_DIR)
	@$(call clean_pkg, WBM_NG_PLUGIN_CLOCK)
	@rm -rf $(WBM_NG_PLUGIN_CLOCK_BUILDROOT_DIR)

# vim: syntax=make
