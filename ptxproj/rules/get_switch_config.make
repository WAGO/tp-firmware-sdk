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
PACKAGES-$(PTXCONF_GET_SWITCH_CONFIG) += get_switch_config

#
# Paths and names
#
GET_SWITCH_CONFIG_VERSION        := 1.1.0
GET_SWITCH_CONFIG_MD5            :=
GET_SWITCH_CONFIG                := get_switch_config
GET_SWITCH_CONFIG_BUILDCONFIG    := Release
GET_SWITCH_CONFIG_SRC_DIR        := $(PTXDIST_WORKSPACE)/local_src/$(GET_SWITCH_CONFIG)
GET_SWITCH_CONFIG_BUILDROOT_DIR  := $(BUILDDIR)/$(GET_SWITCH_CONFIG)
GET_SWITCH_CONFIG_DIR            := $(GET_SWITCH_CONFIG_BUILDROOT_DIR)/src
GET_SWITCH_CONFIG_BIN_DIR        := $(GET_SWITCH_CONFIG_BUILDROOT_DIR)/bin/$(GET_SWITCH_CONFIG_BUILDCONFIG)
GET_SWITCH_CONFIG_LICENSE        := MPL-2.0
GET_SWITCH_CONFIG_CONF_TOOL      := NO
GET_SWITCH_CONFIG_MAKE_ENV       := $(CROSS_ENV) \
BUILDCONFIG=$(GET_SWITCH_CONFIG_BUILDCONFIG) \
BIN_DIR=$(GET_SWITCH_CONFIG_BIN_DIR) \
SCRIPT_DIR=$(PTXDIST_SYSROOT_HOST)/usr/lib/ct-build \


# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

# During BSP creation local_src is deleted and the source code directories are
# copied on demand. To handle this condition an order-only dependency on
# the source code directory is created. When it is missing, the target below
# is executed and an error message is generated.
$(GET_SWITCH_CONFIG_SRC_DIR):
	@echo "Error: $@: directory not found!" >&2; exit 2

$(STATEDIR)/get_switch_config.extract:  | $(GET_SWITCH_CONFIG_SRC_DIR) 
	@$(call targetinfo)
	@mkdir -p $(GET_SWITCH_CONFIG_BUILDROOT_DIR)
	@if [ ! -L $(GET_SWITCH_CONFIG_DIR) ]; then \
	  ln -s $(GET_SWITCH_CONFIG_SRC_DIR) $(GET_SWITCH_CONFIG_DIR); \
	fi
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/get_switch_config.prepare:
	@$(call targetinfo)
	@$(call world/prepare, GET_SWITCH_CONFIG)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/get_switch_config.compile:
	@$(call targetinfo)
	@$(call world/compile, GET_SWITCH_CONFIG)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/get_switch_config.install:
	@$(call targetinfo)

	@$(call world/install, GET_SWITCH_CONFIG)


	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/get_switch_config.targetinstall:
	@$(call targetinfo)

	@$(call install_init, get_switch_config)
	@$(call install_fixup, get_switch_config,PRIORITY,optional)
	@$(call install_fixup, get_switch_config,SECTION,base)
	@$(call install_fixup, get_switch_config,AUTHOR,"WAGO GmbH \& Co. KG")
	@$(call install_fixup, get_switch_config,DESCRIPTION,missing)

	@$(call install_copy, get_switch_config, 0, 0, 0755, $(GET_SWITCH_CONFIG_BIN_DIR)/get_switch_config.elf, /etc/config-tools/get_switch_config)

	@$(call install_finish, get_switch_config)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/get_switch_config.clean:
	@$(call targetinfo)
	@if [ -d $(GET_SWITCH_CONFIG_DIR) ]; then \
		$(GET_SWITCH_CONFIG_MAKE_ENV) $(GET_SWITCH_CONFIG_PATH) $(MAKE) $(MFLAGS) -C $(GET_SWITCH_CONFIG_DIR) clean; \
	fi
	@$(call clean_pkg, GET_SWITCH_CONFIG)
	@rm -rf $(GET_SWITCH_CONFIG_BUILDROOT_DIR)

# vim: syntax=make
