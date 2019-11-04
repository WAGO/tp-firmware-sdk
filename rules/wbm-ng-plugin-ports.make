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
PACKAGES-$(PTXCONF_WBM_NG_PLUGIN_PORTS) += wbm-ng-plugin-ports

#
# Paths and names
#
WBM_NG_PLUGIN_PORTS                := wbm-ports
WBM_NG_PLUGIN_PORTS_VERSION        := 1.4.0-rc.156162409713
WBM_NG_PLUGIN_PORTS_SUFFIX         := tgz
WBM_NG_PLUGIN_PORTS_ARCHIVE        := $(WBM_NG_PLUGIN_PORTS)-$(WBM_NG_PLUGIN_PORTS_VERSION).$(WBM_NG_PLUGIN_PORTS_SUFFIX)
WBM_NG_PLUGIN_PORTS_URL            := http://svsv01003/wago-ptxdist-src/$(WBM_NG_PLUGIN_PORTS_ARCHIVE)
WBM_NG_PLUGIN_PORTS_MD5            := 2fa2e128918b8f76416586cc0e55dfef

WBM_NG_PLUGIN_PORTS_BUILDROOT_DIR  := $(BUILDDIR)/wbm-ng-plugin-ports
WBM_NG_PLUGIN_PORTS_SOURCE         := $(SRCDIR)/$(WBM_NG_PLUGIN_PORTS_ARCHIVE)
WBM_NG_PLUGIN_PORTS_DIR            := $(WBM_NG_PLUGIN_PORTS_BUILDROOT_DIR)
WBM_NG_PLUGIN_PORTS_LICENSE        := unknown
WBM_NG_PLUGIN_PORTS_MAKE_ENV       :=
ifeq ($(PTXCONF_WBM),y)
WBM_NG_PLUGIN_PORTS_TARGET_DIR     := /var/www/wbm-ng/plugins/$(WBM_NG_PLUGIN_PORTS)
else
WBM_NG_PLUGIN_PORTS_TARGET_DIR     := /var/www/wbm/plugins/$(WBM_NG_PLUGIN_PORTS)
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

$(STATEDIR)/wbm-ng-plugin-ports.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-ports.compile:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-ports.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-ports.targetinstall:
	@$(call targetinfo)

	@$(call install_init, wbm-ng-plugin-ports)
	@$(call install_fixup, wbm-ng-plugin-ports, PRIORITY, optional)
	@$(call install_fixup, wbm-ng-plugin-ports, SECTION, base)
	@$(call install_fixup, wbm-ng-plugin-ports, AUTHOR,"Marius Hellmeier, WAGO Kontakttechnik GmbH \& Co. KG")
	@$(call install_fixup, wbm-ng-plugin-ports, DESCRIPTION, missing)

	# create target directory itself
	@$(call install_copy, wbm-ng-plugin-ports, 0, 0, 0755, $(WBM_NG_PLUGIN_PORTS_TARGET_DIR))

	# loop over all files and subdirectories (deep)
	@cd $(WBM_NG_PLUGIN_PORTS_DIR) && \
	for object in $$( find ./* -print ); do \
		if test -f $$object; then \
			$(call install_copy, wbm-ng-plugin-ports, 0, 0, 0644, $(WBM_NG_PLUGIN_PORTS_DIR)/$$object, $(WBM_NG_PLUGIN_PORTS_TARGET_DIR)/$$object); \
		elif test -d $$object; then \
			$(call install_copy, wbm-ng-plugin-ports, 0, 0, 0755, $(WBM_NG_PLUGIN_PORTS_TARGET_DIR)/$$object); \
		fi; \
	done;


	@$(call install_finish, wbm-ng-plugin-ports)
	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/wbm-ng-plugin-ports.clean:
	@$(call targetinfo)
	@rm -rf $(PTXCONF_SYSROOT_TARGET)$(WBM_NG_PLUGIN_PORTS_TARGET_DIR)
	@$(call clean_pkg, WBM_NG_PLUGIN_PORTS)
	@rm -rf $(WBM_NG_PLUGIN_PORTS_BUILDROOT_DIR)

# vim: syntax=make
