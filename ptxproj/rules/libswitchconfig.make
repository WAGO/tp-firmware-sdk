# -*-makefile-*-
#
# Copyright (C) 2023 by WAGO GmbH \& Co. KG
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_LIBSWITCHCONFIG) += libswitchconfig

#
# Paths and names
#
LIBSWITCHCONFIG_VERSION        := 1.1.0
LIBSWITCHCONFIG_MD5            :=
LIBSWITCHCONFIG                := libswitchconfig
LIBSWITCHCONFIG_BUILDCONFIG    := Release
LIBSWITCHCONFIG_SRC_DIR        := $(PTXDIST_WORKSPACE)/local_src/$(LIBSWITCHCONFIG)
LIBSWITCHCONFIG_BUILDROOT_DIR  := $(BUILDDIR)/$(LIBSWITCHCONFIG)
LIBSWITCHCONFIG_DIR            := $(LIBSWITCHCONFIG_BUILDROOT_DIR)/src
LIBSWITCHCONFIG_BIN_DIR        := $(LIBSWITCHCONFIG_BUILDROOT_DIR)/bin/$(LIBSWITCHCONFIG_BUILDCONFIG)
LIBSWITCHCONFIG_LICENSE        := MPL-2.0
LIBSWITCHCONFIG_CONF_TOOL      := NO
LIBSWITCHCONFIG_MAKE_ENV       := $(CROSS_ENV) \
BUILDCONFIG=$(LIBSWITCHCONFIG_BUILDCONFIG) \
BIN_DIR=$(LIBSWITCHCONFIG_BIN_DIR) \
SCRIPT_DIR=$(PTXDIST_SYSROOT_HOST)/usr/lib/ct-build \


# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

# During BSP creation local_src is deleted and the source code directories are
# copied on demand. To handle this condition an order-only dependency on
# the source code directory is created. When it is missing, the target below
# is executed and an error message is generated.
$(LIBSWITCHCONFIG_SRC_DIR):
	@echo "Error: $@: directory not found!" >&2; exit 2

$(STATEDIR)/libswitchconfig.extract:  | $(LIBSWITCHCONFIG_SRC_DIR) 
	@$(call targetinfo)
	@mkdir -p $(LIBSWITCHCONFIG_BUILDROOT_DIR)
	@if [ ! -L $(LIBSWITCHCONFIG_DIR) ]; then \
	  ln -s $(LIBSWITCHCONFIG_SRC_DIR) $(LIBSWITCHCONFIG_DIR); \
	fi
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/libswitchconfig.prepare:
	@$(call targetinfo)
	@$(call world/prepare, LIBSWITCHCONFIG)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/libswitchconfig.compile:
	@$(call targetinfo)
	@$(call world/compile, LIBSWITCHCONFIG)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/libswitchconfig.install:
	@$(call targetinfo)

	@$(call world/install, LIBSWITCHCONFIG)


	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/libswitchconfig.targetinstall:
	@$(call targetinfo)

	@$(call install_init, libswitchconfig)
	@$(call install_fixup, libswitchconfig,PRIORITY,optional)
	@$(call install_fixup, libswitchconfig,SECTION,base)
	@$(call install_fixup, libswitchconfig,AUTHOR,"WAGO GmbH \& Co. KG")
	@$(call install_fixup, libswitchconfig,DESCRIPTION,missing)

	@$(call install_lib, libswitchconfig, 0, 0, 0644, libswitchconfig)
	@$(call install_alternative, libswitchconfig, 0, 0, 0755, /etc/init.d/featuredetect_switch)
	@$(call install_link, libswitchconfig, ../init.d/featuredetect_switch, /etc/rc.d/S99_featuredetect_switch)

	@$(call install_finish, libswitchconfig)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/libswitchconfig.clean:
	@$(call targetinfo)
	@if [ -d $(LIBSWITCHCONFIG_DIR) ]; then \
		$(LIBSWITCHCONFIG_MAKE_ENV) $(LIBSWITCHCONFIG_PATH) $(MAKE) $(MFLAGS) -C $(LIBSWITCHCONFIG_DIR) clean; \
	fi
	@$(call clean_pkg, LIBSWITCHCONFIG)
	@rm -rf $(LIBSWITCHCONFIG_BUILDROOT_DIR)

# vim: syntax=make
