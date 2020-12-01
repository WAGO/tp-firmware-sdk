# -*-makefile-*-
#
# Copyright (C) 2014 by <elrest>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_GET_BOOT) += get_boot

#
# Paths and names
#
GET_BOOT_VERSION	:= 1
GET_BOOT_MD5		:=
GET_BOOT		:= get_boot
GET_BOOT_URL		:= file://local_src/$(GET_BOOT)
GET_BOOT_DIR		:= $(BUILDDIR)/$(GET_BOOT)
GET_BOOT_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_boot.extract:
#	@$(call targetinfo)
#	@$(call clean, $(GET_BOOT_DIR))
#	@$(call extract, GET_BOOT)
#	@$(call patchin, GET_BOOT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#GET_BOOT_PATH	:= PATH=$(CROSS_PATH)
GET_BOOT_CONF_TOOL	:= NO
GET_BOOT_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/get_boot.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, GET_BOOT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_boot.compile:
#	@$(call targetinfo)
#	@$(call world/compile, GET_BOOT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_boot.install:
#	@$(call targetinfo)
#	@$(call world/install, GET_BOOT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/get_boot.targetinstall:
	@$(call targetinfo)

	@$(call install_init, get_boot)
	@$(call install_fixup, get_boot,PRIORITY,optional)
	@$(call install_fixup, get_boot,SECTION,base)
	@$(call install_fixup, get_boot,AUTHOR,"<elrest>")
	@$(call install_fixup, get_boot,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, get_boot, 0, 0, 0755, $(GET_BOOT_DIR)/get_boot, /etc/config-tools/get_boot)

	@$(call install_finish, get_boot)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/get_boot.clean:
	@$(call targetinfo)
	cd $(GET_BOOT_DIR) && rm -f *.o $(GET_BOOT)	
	@-cd $(GET_BOOT_DIR) && \
		$(GET_BOOT_MAKE_ENV) $(GET_BOOT_PATH) $(MAKE) clean
	@$(call clean_pkg, GET_BOOT)

# vim: syntax=make
