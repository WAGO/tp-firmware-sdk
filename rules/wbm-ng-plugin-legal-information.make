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
PACKAGES-$(PTXCONF_WBM_NG_PLUGIN_LEGAL_INFORMATION) += wbm-ng-plugin-legal-information

#
# Paths and names
#
WBM_NG_PLUGIN_LEGAL_INFORMATION                := wbm-legal-information
WBM_NG_PLUGIN_LEGAL_INFORMATION_VERSION        := 1.2.1-rc.156162455114
WBM_NG_PLUGIN_LEGAL_INFORMATION_SUFFIX         := tgz
WBM_NG_PLUGIN_LEGAL_INFORMATION_ARCHIVE        := $(WBM_NG_PLUGIN_LEGAL_INFORMATION)-$(WBM_NG_PLUGIN_LEGAL_INFORMATION_VERSION).$(WBM_NG_PLUGIN_LEGAL_INFORMATION_SUFFIX)
WBM_NG_PLUGIN_LEGAL_INFORMATION_URL            := http://svsv01003/wago-ptxdist-src/$(WBM_NG_PLUGIN_LEGAL_INFORMATION_ARCHIVE)
WBM_NG_PLUGIN_LEGAL_INFORMATION_MD5            := 30283cd9d706af9ecb0806f725e65940

WBM_NG_PLUGIN_LEGAL_INFORMATION_BUILDROOT_DIR  := $(BUILDDIR)/wbm-ng-plugin-legal-information
WBM_NG_PLUGIN_LEGAL_INFORMATION_SOURCE         := $(SRCDIR)/$(WBM_NG_PLUGIN_LEGAL_INFORMATION_ARCHIVE)
WBM_NG_PLUGIN_LEGAL_INFORMATION_DIR            := $(WBM_NG_PLUGIN_LEGAL_INFORMATION_BUILDROOT_DIR)
WBM_NG_PLUGIN_LEGAL_INFORMATION_LICENSE        := unknown
WBM_NG_PLUGIN_LEGAL_INFORMATION_MAKE_ENV       :=
ifeq ($(PTXCONF_WBM),y)
WBM_NG_PLUGIN_LEGAL_INFORMATION_TARGET_DIR     := /var/www/wbm-ng/plugins/$(WBM_NG_PLUGIN_LEGAL_INFORMATION)
else
WBM_NG_PLUGIN_LEGAL_INFORMATION_TARGET_DIR     := /var/www/wbm/plugins/$(WBM_NG_PLUGIN_LEGAL_INFORMATION)
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

$(STATEDIR)/wbm-ng-plugin-legal-information.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-legal-information.compile:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-legal-information.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-legal-information.targetinstall:
	@$(call targetinfo)

	@$(call install_init, wbm-ng-plugin-legal-information)
	@$(call install_fixup, wbm-ng-plugin-legal-information, PRIORITY, optional)
	@$(call install_fixup, wbm-ng-plugin-legal-information, SECTION, base)
	@$(call install_fixup, wbm-ng-plugin-legal-information, AUTHOR,"Marius Hellmeier, WAGO Kontakttechnik GmbH \& Co. KG")
	@$(call install_fixup, wbm-ng-plugin-legal-information, DESCRIPTION, missing)

	# create target directory itself
	@$(call install_copy, wbm-ng-plugin-legal-information, 0, 0, 0755, $(WBM_NG_PLUGIN_LEGAL_INFORMATION_TARGET_DIR))

	# loop over all files and subdirectories (deep)
	@cd $(WBM_NG_PLUGIN_LEGAL_INFORMATION_DIR) && \
	for object in $$( find ./* -print ); do \
		if test -f $$object; then \
			$(call install_copy, wbm-ng-plugin-legal-information, 0, 0, 0644, $(WBM_NG_PLUGIN_LEGAL_INFORMATION_DIR)/$$object, $(WBM_NG_PLUGIN_LEGAL_INFORMATION_TARGET_DIR)/$$object); \
		elif test -d $$object; then \
			$(call install_copy, wbm-ng-plugin-legal-information, 0, 0, 0755, $(WBM_NG_PLUGIN_LEGAL_INFORMATION_TARGET_DIR)/$$object); \
		fi; \
	done;

	@$(call install_finish, wbm-ng-plugin-legal-information)
	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-legal-information.clean:
	@$(call targetinfo)
	@rm -rf $(PTXCONF_SYSROOT_TARGET)$(WBM_NG_PLUGIN_LEGAL_INFORMATION_TARGET_DIR)
	@$(call clean_pkg, WBM_NG_PLUGIN_LEGAL_INFORMATION)
	@rm -rf $(WBM_NG_PLUGIN_LEGAL_INFORMATION_BUILDROOT_DIR)

# vim: syntax=make
