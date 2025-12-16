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
PACKAGES-$(PTXCONF_CONFIG_SWITCH) += config_switch

#
# Paths and names
#
CONFIG_SWITCH_VERSION        := 1.1.0
CONFIG_SWITCH_MD5            :=
CONFIG_SWITCH                := config_switch
CONFIG_SWITCH_BUILDCONFIG    := Release
CONFIG_SWITCH_SRC_DIR        := $(PTXDIST_WORKSPACE)/local_src/$(CONFIG_SWITCH)
CONFIG_SWITCH_BUILDROOT_DIR  := $(BUILDDIR)/$(CONFIG_SWITCH)
CONFIG_SWITCH_DIR            := $(CONFIG_SWITCH_BUILDROOT_DIR)/src
CONFIG_SWITCH_BIN_DIR        := $(CONFIG_SWITCH_BUILDROOT_DIR)/bin/$(CONFIG_SWITCH_BUILDCONFIG)
CONFIG_SWITCH_LICENSE        := MPL-2.0
CONFIG_SWITCH_CONF_TOOL      := NO
CONFIG_SWITCH_MAKE_ENV       := $(CROSS_ENV) \
BUILDCONFIG=$(CONFIG_SWITCH_BUILDCONFIG) \
BIN_DIR=$(CONFIG_SWITCH_BIN_DIR) \
SCRIPT_DIR=$(PTXDIST_SYSROOT_HOST)/usr/lib/ct-build \


# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

# During BSP creation local_src is deleted and the source code directories are
# copied on demand. To handle this condition an order-only dependency on
# the source code directory is created. When it is missing, the target below
# is executed and an error message is generated.
$(CONFIG_SWITCH_SRC_DIR):
	@echo "Error: $@: directory not found!" >&2; exit 2

$(STATEDIR)/config_switch.extract:  | $(CONFIG_SWITCH_SRC_DIR)
	@$(call targetinfo)
	@mkdir -p $(CONFIG_SWITCH_BUILDROOT_DIR)
	@if [ ! -L $(CONFIG_SWITCH_DIR) ]; then \
	  ln -s $(CONFIG_SWITCH_SRC_DIR) $(CONFIG_SWITCH_DIR); \
	fi
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/config_switch.prepare:
	@$(call targetinfo)
	@$(call world/prepare, CONFIG_SWITCH)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/config_switch.compile:
	@$(call targetinfo)
	@$(call world/compile, CONFIG_SWITCH)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/config_switch.install:
	@$(call targetinfo)

	@$(call world/install, CONFIG_SWITCH)


	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/config_switch.targetinstall:
	@$(call targetinfo)

	@$(call install_init, config_switch)
	@$(call install_fixup, config_switch,PRIORITY,optional)
	@$(call install_fixup, config_switch,SECTION,base)
	@$(call install_fixup, config_switch,AUTHOR,"WAGO GmbH \& Co. KG")
	@$(call install_fixup, config_switch,DESCRIPTION,missing)

	@$(call install_copy, config_switch, 0, 0, 0755, $(CONFIG_SWITCH_BIN_DIR)/config_switch.elf, /etc/config-tools/config_switch)

	@$(call install_finish, config_switch)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/config_switch.clean:
	@$(call targetinfo)
	@if [ -d $(CONFIG_SWITCH_DIR) ]; then \
		$(CONFIG_SWITCH_MAKE_ENV) $(CONFIG_SWITCH_PATH) $(MAKE) $(MFLAGS) -C $(CONFIG_SWITCH_DIR) clean; \
	fi
	@$(call clean_pkg, CONFIG_SWITCH)
	@rm -rf $(CONFIG_SWITCH_BUILDROOT_DIR)

# vim: syntax=make
