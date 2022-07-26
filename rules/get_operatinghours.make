# -*-makefile-*-
#
# Copyright (C) 2013 by <elrest>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_GET_OPERATINGHOURS) += get_operatinghours

#
# Paths and names
#
GET_OPERATINGHOURS_VERSION	:= 0.0.1
GET_OPERATINGHOURS_MD5		:=
GET_OPERATINGHOURS		    := get_operatinghours
GET_OPERATINGHOURS_URL		:= file://local_src/$(GET_OPERATINGHOURS)
GET_OPERATINGHOURS_DIR		:= $(BUILDDIR)/$(GET_OPERATINGHOURS)
GET_OPERATINGHOURS_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_operatinghours.extract:
#	@$(call targetinfo)
#	@$(call clean, $(GET_OPERATINGHOURS_DIR))
#	@$(call extract, GET_OPERATINGHOURS)
#	@$(call patchin, GET_OPERATINGHOURS)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#GET_OPERATINGHOURS_PATH	:= PATH=$(CROSS_PATH)
GET_OPERATINGHOURS_CONF_TOOL	:= NO
GET_OPERATINGHOURS_MAKE_ENV	:= $(CROSS_ENV)

$(STATEDIR)/get_operatinghours.prepare:
	@$(call targetinfo)
	cd $(GET_OPERATINGHOURS_DIR) && rm -f *.o $(GET_OPERATINGHOURS)
#	@$(call world/prepare, GET_OPERATINGHOURS)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_operatinghours.compile:
#	@$(call targetinfo)
#	@$(call world/compile, GET_OPERATINGHOURS)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_operatinghours.install:
#	@$(call targetinfo)
#	@$(call world/install, GET_OPERATINGHOURS)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/get_operatinghours.targetinstall:
	@$(call targetinfo)

	@$(call install_init, get_operatinghours)
	@$(call install_fixup, get_operatinghours,PRIORITY,optional)
	@$(call install_fixup, get_operatinghours,SECTION,base)
	@$(call install_fixup, get_operatinghours,AUTHOR,"<elrest>")
	@$(call install_fixup, get_operatinghours,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, get_operatinghours, 0, 0, 0755, $(GET_OPERATINGHOURS_DIR)/get_operatinghours, /etc/config-tools/get_operatinghours)

	@$(call install_finish, get_operatinghours)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/get_operatinghours.clean:
	@$(call targetinfo)
	-cd $(GET_OPERATINGHOURS_DIR) && rm -f *.o $(GET_OPERATINGHOURS)
	@-cd $(GET_OPERATINGHOURS_DIR) && \
		$(GET_OPERATINGHOURS_MAKE_ENV) $(GET_OPERATINGHOURS_PATH) $(MAKE) clean
	@$(call clean_pkg, GET_OPERATINGHOURS)


# vim: syntax=make
