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
PACKAGES-$(PTXCONF_WBM_NG) += wbm-ng

#
# Paths and names
#
WBM_NG                := wbm-pfc
WBM_NG_VERSION        := 2.2.0-rc.156162428539
WBM_NG_SUFFIX         := tgz
WBM_NG_ARCHIVE        := $(WBM_NG)-$(WBM_NG_VERSION).$(WBM_NG_SUFFIX)
WBM_NG_URL            := http://svsv01003/wago-ptxdist-src/$(WBM_NG_ARCHIVE)
WBM_NG_MD5            := 4ee8cc7217311b3a7a2272097165101b

WBM_NG_BUILDROOT_DIR  := $(BUILDDIR)/wbm-ng
WBM_NG_SOURCE         := $(SRCDIR)/$(WBM_NG_ARCHIVE)
WBM_NG_DIR            := $(WBM_NG_BUILDROOT_DIR)
ifeq ($(PTXCONF_WBM),y)
WBM_NG_TARGET_DIR     := /var/www/wbm-ng
else
WBM_NG_TARGET_DIR     := /var/www/wbm
endif
WBM_NG_LICENSE        := unknown
WBM_NG_MAKE_ENV       :=

# Generated blacklist: don't install plugins which are deactivated in ptxdist menuconfig
# The blacklist contains associations between defines and plugin/folder names included
# in the wbm-pfs default plugins inside the archive.
WBM_NG_PLUGINS_BLACKLIST :=
ifndef WBM_NG_FEATURE_EXAMPLE_A
	WBM_NG_PLUGINS_BLACKLIST += wbm-plugin-example-a
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

$(STATEDIR)/wbm-ng.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng.compile:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng.targetinstall:
	@$(call targetinfo)

	@$(call install_init, wbm-ng)
	@$(call install_fixup, wbm-ng, PRIORITY, optional)
	@$(call install_fixup, wbm-ng, SECTION, base)
	@$(call install_fixup, wbm-ng, AUTHOR,"Marius Hellmeier, WAGO Kontakttechnik GmbH \& Co. KG")
	@$(call install_fixup, wbm-ng, DESCRIPTION, missing)

	# create target directory itself
	@$(call install_copy, wbm-ng, 0, 0, 0755, $(WBM_NG_TARGET_DIR))

	# loop over all files and subdirectories (deep)
	# additionally apply a filter to the ./plugins folder, to filter out plugins named in the blacklist
	@cd $(WBM_NG_DIR) && \
	for object in $$( find $$( printf " -path ./plugins/%s -prune -o " $(WBM_NG_PLUGINS_BLACKLIST) ) -print ); do \
		if test -f $$object; then \
			$(call install_copy, wbm-ng, 0, 0, 0644, $(WBM_NG_DIR)/$$object, $(WBM_NG_TARGET_DIR)/$$object); \
		elif test -d $$object; then \
			$(call install_copy, wbm-ng, 0, 0, 0755, $(WBM_NG_TARGET_DIR)/$$object); \
		fi; \
	done;


	@$(call install_finish, wbm-ng)
	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng.clean:
	@$(call targetinfo)
	@rm -rf $(PTXCONF_SYSROOT_TARGET)$(WBM_NG_TARGET_DIR)
	@$(call clean_pkg, WBM_NG)
	@rm -rf $(WBM_NG_BUILDROOT_DIR)

# vim: syntax=make
