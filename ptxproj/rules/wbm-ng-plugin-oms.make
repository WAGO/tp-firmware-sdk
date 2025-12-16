# -*-makefile-*-
#
# Copyright (C) 2025 WAGO GmbH & Co. KG
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_WBM_NG_PLUGIN_OMS) += wbm-ng-plugin-oms

#
# Paths and names
#
WBM_NG_PLUGIN_OMS_VERSION        := 1.0.0
WBM_NG_PLUGIN_OMS                := wbm-oms-$(WBM_NG_PLUGIN_OMS_VERSION)
WBM_NG_PLUGIN_OMS_URL            := $(call jfrog_template_to_url, WBM_NG_PLUGIN_OMS)
WBM_NG_PLUGIN_OMS_SUFFIX         := $(suffix $(WBM_NG_PLUGIN_OMS_URL))
WBM_NG_PLUGIN_OMS_SOURCE         := $(SRCDIR)/$(WBM_NG_PLUGIN_OMS)$(WBM_NG_PLUGIN_OMS_SUFFIX)
WBM_NG_PLUGIN_OMS_MD5             = $(shell [ -f $(WBM_NG_PLUGIN_OMS_MD5_FILE) ] && cat $(WBM_NG_PLUGIN_OMS_MD5_FILE))
WBM_NG_PLUGIN_OMS_MD5_FILE       := $(WBM_NG_PLUGIN_OMS_SOURCE).md5
WBM_NG_PLUGIN_OMS_ARTIFACT        = $(call jfrog_get_filename,$(WBM_NG_PLUGIN_OMS_URL))
WBM_NG_PLUGIN_OMS_BUILDROOT_DIR  := $(BUILDDIR)/wbm-ng-plugin-oms
WBM_NG_PLUGIN_OMS_DIR            := $(WBM_NG_PLUGIN_OMS_BUILDROOT_DIR)
WBM_NG_PLUGIN_OMS_LICENSE        := unknown
WBM_NG_PLUGIN_OMS_MAKE_ENV       :=
ifeq ($(PTXCONF_WBM),y)
	WBM_NG_PLUGIN_OMS_TARGET_DIR     := /var/www/wbm-ng/plugins/wbm-oms
else
	WBM_NG_PLUGIN_OMS_TARGET_DIR     := /var/www/wbm/plugins/wbm-oms
endif

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

$(WBM_NG_PLUGIN_OMS_SOURCE):
	@$(call targetinfo)
	$(call ptx/in-path, PTXDIST_PATH, scripts/wago/artifactory.sh) fetch \
        '$(WBM_NG_PLUGIN_OMS_URL)' '$@' '$(WBM_NG_PLUGIN_OMS_MD5_FILE)'

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

# use ptxdist default

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-oms.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-oms.compile:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-oms.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-oms.targetinstall:
	@$(call targetinfo)

ifdef PTXCONF_OMS_DISABLE_CONTROLS
	@$(call install_init, wbm-ng-plugin-oms)
	@$(call install_fixup, wbm-ng-plugin-oms, PRIORITY, optional)
	@$(call install_fixup, wbm-ng-plugin-oms, SECTION, base)
	@$(call install_fixup, wbm-ng-plugin-oms, AUTHOR,"WAGO GmbH \& Co. KG")
	@$(call install_fixup, wbm-ng-plugin-oms, DESCRIPTION, missing)

	# create target directory itself
	@$(call install_copy, wbm-ng-plugin-oms, 0, 0, 0755, $(WBM_NG_PLUGIN_OMS_TARGET_DIR))

	# loop over all files and subdirectories (deep)
	@cd $(WBM_NG_PLUGIN_OMS_DIR) && \
	for object in $$( find ./* \( -path './series' -o -path './.ptxdist*' -o -path './.pc*' \) -prune -o -print ); do \
		if test -f $$object -a ! -h $$object; then \
			$(call install_copy, wbm-ng-plugin-oms, 0, 0, 0644, $(WBM_NG_PLUGIN_OMS_DIR)/$$object, $(WBM_NG_PLUGIN_OMS_TARGET_DIR)/$$object); \
		elif test -d $$object -a ! -h $$object; then \
			$(call install_copy, wbm-ng-plugin-oms, 0, 0, 0755, $(WBM_NG_PLUGIN_OMS_TARGET_DIR)/$$object); \
		fi; \
	done;
endif

	@$(call install_finish, wbm-ng-plugin-oms)
	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-oms.clean:
	@$(call targetinfo)
	@rm -rf $(PTXCONF_SYSROOT_TARGET)$(WBM_NG_PLUGIN_OMS_TARGET_DIR)
	@$(call clean_pkg, WBM_NG_PLUGIN_OMS)
	@rm -rf $(WBM_NG_PLUGIN_OMS_BUILDROOT_DIR)

# vim: syntax=make
