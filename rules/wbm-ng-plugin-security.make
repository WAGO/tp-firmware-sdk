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
PACKAGES-$(PTXCONF_WBM_NG_PLUGIN_SECURITY) += wbm-ng-plugin-security

#
# Paths and names
#
WBM_NG_PLUGIN_SECURITY                := wbm-security
WBM_NG_PLUGIN_SECURITY_VERSION        := 1.1.0-rc.15616241069
WBM_NG_PLUGIN_SECURITY_SUFFIX         := tgz
WBM_NG_PLUGIN_SECURITY_ARCHIVE        := $(WBM_NG_PLUGIN_SECURITY)-$(WBM_NG_PLUGIN_SECURITY_VERSION).$(WBM_NG_PLUGIN_SECURITY_SUFFIX)
WBM_NG_PLUGIN_SECURITY_URL            := http://svsv01003/wago-ptxdist-src/$(WBM_NG_PLUGIN_SECURITY_ARCHIVE)
WBM_NG_PLUGIN_SECURITY_MD5            := a0b29cdeee8383f37002f23d9c18a28a

WBM_NG_PLUGIN_SECURITY_BUILDROOT_DIR  := $(BUILDDIR)/wbm-ng-plugin-security
WBM_NG_PLUGIN_SECURITY_SOURCE         := $(SRCDIR)/$(WBM_NG_PLUGIN_SECURITY_ARCHIVE)
WBM_NG_PLUGIN_SECURITY_DIR            := $(WBM_NG_PLUGIN_SECURITY_BUILDROOT_DIR)
WBM_NG_PLUGIN_SECURITY_LICENSE        := unknown
WBM_NG_PLUGIN_SECURITY_MAKE_ENV       :=
ifeq ($(PTXCONF_WBM),y)
WBM_NG_PLUGIN_SECURITY_TARGET_DIR     := /var/www/wbm-ng/plugins/$(WBM_NG_PLUGIN_SECURITY)
else
WBM_NG_PLUGIN_SECURITY_TARGET_DIR     := /var/www/wbm/plugins/$(WBM_NG_PLUGIN_SECURITY)
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

$(STATEDIR)/wbm-ng-plugin-security.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-security.compile:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-security.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-security.targetinstall:
	@$(call targetinfo)

	@$(call install_init, wbm-ng-plugin-security)
	@$(call install_fixup, wbm-ng-plugin-security, PRIORITY, optional)
	@$(call install_fixup, wbm-ng-plugin-security, SECTION, base)
	@$(call install_fixup, wbm-ng-plugin-security, AUTHOR,"Marius Hellmeier, WAGO Kontakttechnik GmbH \& Co. KG")
	@$(call install_fixup, wbm-ng-plugin-security, DESCRIPTION, missing)

	# create target directory itself
	@$(call install_copy, wbm-ng-plugin-security, 0, 0, 0755, $(WBM_NG_PLUGIN_SECURITY_TARGET_DIR))

	# loop over all files and subdirectories (deep)
	@cd $(WBM_NG_PLUGIN_SECURITY_DIR) && \
	for object in $$( find ./* -print ); do \
		if test -f $$object; then \
			$(call install_copy, wbm-ng-plugin-security, 0, 0, 0644, $(WBM_NG_PLUGIN_SECURITY_DIR)/$$object, $(WBM_NG_PLUGIN_SECURITY_TARGET_DIR)/$$object); \
		elif test -d $$object; then \
			$(call install_copy, wbm-ng-plugin-security, 0, 0, 0755, $(WBM_NG_PLUGIN_SECURITY_TARGET_DIR)/$$object); \
		fi; \
	done;


	@$(call install_finish, wbm-ng-plugin-security)
	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-security.clean:
	@$(call targetinfo)
	@rm -rf $(PTXCONF_SYSROOT_TARGET)$(WBM_NG_PLUGIN_SECURITY_TARGET_DIR)
	@$(call clean_pkg, WBM_NG_PLUGIN_SECURITY)
	@rm -rf $(WBM_NG_PLUGIN_SECURITY_BUILDROOT_DIR)

# vim: syntax=make
