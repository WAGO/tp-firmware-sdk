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
PACKAGES-$(PTXCONF_WBM_NG_PLUGIN_IPK_UPLOADS) += wbm-ng-plugin-ipk-uploads

#
# Paths and names
#
WBM_NG_PLUGIN_IPK_UPLOADS                := wbm-ipk-uploads
WBM_NG_PLUGIN_IPK_UPLOADS_VERSION        := 1.0.1-rc.15616241019
WBM_NG_PLUGIN_IPK_UPLOADS_SUFFIX         := tgz
WBM_NG_PLUGIN_IPK_UPLOADS_ARCHIVE        := $(WBM_NG_PLUGIN_IPK_UPLOADS)-$(WBM_NG_PLUGIN_IPK_UPLOADS_VERSION).$(WBM_NG_PLUGIN_IPK_UPLOADS_SUFFIX)
WBM_NG_PLUGIN_IPK_UPLOADS_URL            := http://svsv01003/wago-ptxdist-src/$(WBM_NG_PLUGIN_IPK_UPLOADS_ARCHIVE)
WBM_NG_PLUGIN_IPK_UPLOADS_MD5            := a48f2fa1a2c51b4578ab42e66cdaf71c

WBM_NG_PLUGIN_IPK_UPLOADS_BUILDROOT_DIR  := $(BUILDDIR)/wbm-ng-plugin-ipk-uploads
WBM_NG_PLUGIN_IPK_UPLOADS_SOURCE         := $(SRCDIR)/$(WBM_NG_PLUGIN_IPK_UPLOADS_ARCHIVE)
WBM_NG_PLUGIN_IPK_UPLOADS_DIR            := $(WBM_NG_PLUGIN_IPK_UPLOADS_BUILDROOT_DIR)
WBM_NG_PLUGIN_IPK_UPLOADS_LICENSE        := unknown
WBM_NG_PLUGIN_IPK_UPLOADS_MAKE_ENV       :=
ifeq ($(PTXCONF_WBM),y)
WBM_NG_PLUGIN_IPK_UPLOADS_TARGET_DIR     := /var/www/wbm-ng/plugins/$(WBM_NG_PLUGIN_IPK_UPLOADS)
else
WBM_NG_PLUGIN_IPK_UPLOADS_TARGET_DIR     := /var/www/wbm/plugins/$(WBM_NG_PLUGIN_IPK_UPLOADS)
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

$(STATEDIR)/wbm-ng-plugin-ipk-uploads.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-ipk-uploads.compile:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-ipk-uploads.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-ipk-uploads.targetinstall:
	@$(call targetinfo)

	@$(call install_init, wbm-ng-plugin-ipk-uploads)
	@$(call install_fixup, wbm-ng-plugin-ipk-uploads, PRIORITY, optional)
	@$(call install_fixup, wbm-ng-plugin-ipk-uploads, SECTION, base)
	@$(call install_fixup, wbm-ng-plugin-ipk-uploads, AUTHOR,"Marius Hellmeier, WAGO Kontakttechnik GmbH \& Co. KG")
	@$(call install_fixup, wbm-ng-plugin-ipk-uploads, DESCRIPTION, missing)

	# create target directory itself
	@$(call install_copy, wbm-ng-plugin-ipk-uploads, 0, 0, 0755, $(WBM_NG_PLUGIN_IPK_UPLOADS_TARGET_DIR))

	# loop over all files and subdirectories (deep)
	@cd $(WBM_NG_PLUGIN_IPK_UPLOADS_DIR) && \
	for object in $$( find ./* -print ); do \
		if test -f $$object; then \
			$(call install_copy, wbm-ng-plugin-ipk-uploads, 0, 0, 0644, $(WBM_NG_PLUGIN_IPK_UPLOADS_DIR)/$$object, $(WBM_NG_PLUGIN_IPK_UPLOADS_TARGET_DIR)/$$object); \
		elif test -d $$object; then \
			$(call install_copy, wbm-ng-plugin-ipk-uploads, 0, 0, 0755, $(WBM_NG_PLUGIN_IPK_UPLOADS_TARGET_DIR)/$$object); \
		fi; \
	done;


	@$(call install_finish, wbm-ng-plugin-ipk-uploads)
	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-ipk-uploads.clean:
	@$(call targetinfo)
	@rm -rf $(PTXCONF_SYSROOT_TARGET)$(WBM_NG_PLUGIN_IPK_UPLOADS_TARGET_DIR)
	@$(call clean_pkg, WBM_NG_PLUGIN_IPK_UPLOADS)
	@rm -rf $(WBM_NG_PLUGIN_IPK_UPLOADS_BUILDROOT_DIR)

# vim: syntax=make
