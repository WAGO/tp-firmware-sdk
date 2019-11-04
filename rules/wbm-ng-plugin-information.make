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
PACKAGES-$(PTXCONF_WBM_NG_PLUGIN_INFORMATION) += wbm-ng-plugin-information

#
# Paths and names
#
WBM_NG_PLUGIN_INFORMATION                := wbm-information
WBM_NG_PLUGIN_INFORMATION_VERSION        := 2.0.1-rc.156162408816
WBM_NG_PLUGIN_INFORMATION_SUFFIX         := tgz
WBM_NG_PLUGIN_INFORMATION_ARCHIVE        := $(WBM_NG_PLUGIN_INFORMATION)-$(WBM_NG_PLUGIN_INFORMATION_VERSION).$(WBM_NG_PLUGIN_INFORMATION_SUFFIX)
WBM_NG_PLUGIN_INFORMATION_URL            := http://svsv01003/wago-ptxdist-src/$(WBM_NG_PLUGIN_INFORMATION_ARCHIVE)
WBM_NG_PLUGIN_INFORMATION_MD5            := 10c1fd1d27542aeadafedcf738fe4250

WBM_NG_PLUGIN_INFORMATION_BUILDROOT_DIR  := $(BUILDDIR)/wbm-ng-plugin-information
WBM_NG_PLUGIN_INFORMATION_SOURCE         := $(SRCDIR)/$(WBM_NG_PLUGIN_INFORMATION_ARCHIVE)
WBM_NG_PLUGIN_INFORMATION_DIR            := $(WBM_NG_PLUGIN_INFORMATION_BUILDROOT_DIR)
WBM_NG_PLUGIN_INFORMATION_LICENSE        := unknown
WBM_NG_PLUGIN_INFORMATION_MAKE_ENV       :=
ifeq ($(PTXCONF_WBM),y)
WBM_NG_PLUGIN_INFORMATION_TARGET_DIR     := /var/www/wbm-ng/plugins/$(WBM_NG_PLUGIN_INFORMATION)
else
WBM_NG_PLUGIN_INFORMATION_TARGET_DIR     := /var/www/wbm/plugins/$(WBM_NG_PLUGIN_INFORMATION)
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

$(STATEDIR)/wbm-ng-plugin-information.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-information.compile:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-information.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-information.targetinstall:
	@$(call targetinfo)

	@$(call install_init, wbm-ng-plugin-information)
	@$(call install_fixup, wbm-ng-plugin-information, PRIORITY, optional)
	@$(call install_fixup, wbm-ng-plugin-information, SECTION, base)
	@$(call install_fixup, wbm-ng-plugin-information, AUTHOR,"Marius Hellmeier, WAGO Kontakttechnik GmbH \& Co. KG")
	@$(call install_fixup, wbm-ng-plugin-information, DESCRIPTION, missing)

	# create target directory itself
	@$(call install_copy, wbm-ng-plugin-information, 0, 0, 0755, $(WBM_NG_PLUGIN_INFORMATION_TARGET_DIR))

	# loop over all files and subdirectories (deep)
	@cd $(WBM_NG_PLUGIN_INFORMATION_DIR) && \
	for object in $$( find ./* -print ); do \
		if test -f $$object; then \
			$(call install_copy, wbm-ng-plugin-information, 0, 0, 0644, $(WBM_NG_PLUGIN_INFORMATION_DIR)/$$object, $(WBM_NG_PLUGIN_INFORMATION_TARGET_DIR)/$$object); \
		elif test -d $$object; then \
			$(call install_copy, wbm-ng-plugin-information, 0, 0, 0755, $(WBM_NG_PLUGIN_INFORMATION_TARGET_DIR)/$$object); \
		fi; \
	done;


	@$(call install_finish, wbm-ng-plugin-information)
	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-information.clean:
	@$(call targetinfo)
	@rm -rf $(PTXCONF_SYSROOT_TARGET)$(WBM_NG_PLUGIN_INFORMATION_TARGET_DIR)
	@$(call clean_pkg, WBM_NG_PLUGIN_INFORMATION)
	@rm -rf $(WBM_NG_PLUGIN_INFORMATION_BUILDROOT_DIR)

# vim: syntax=make
