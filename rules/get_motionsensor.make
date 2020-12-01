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
PACKAGES-$(PTXCONF_GET_MOTIONSENSOR) += get_motionsensor

#
# Paths and names
#
GET_MOTIONSENSOR_VERSION	:= 0.0.1
GET_MOTIONSENSOR_MD5		:=
GET_MOTIONSENSOR		:= get_motionsensor
GET_MOTIONSENSOR_URL		:= file://local_src/$(GET_MOTIONSENSOR)
GET_MOTIONSENSOR_DIR		:= $(BUILDDIR)/$(GET_MOTIONSENSOR)
GET_MOTIONSENSOR_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_motionsensor.extract:
#	@$(call targetinfo)
#	@$(call clean, $(GET_MOTIONSENSOR_DIR))
#	@$(call extract, GET_MOTIONSENSOR)
#	@$(call patchin, GET_MOTIONSENSOR)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#GET_MOTIONSENSOR_PATH	:= PATH=$(CROSS_PATH)
GET_MOTIONSENSOR_CONF_TOOL	:= NO
GET_MOTIONSENSOR_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/get_motionsensor.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, GET_MOTIONSENSOR)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_motionsensor.compile:
#	@$(call targetinfo)
#	@$(call world/compile, GET_MOTIONSENSOR)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_motionsensor.install:
#	@$(call targetinfo)
#	@$(call world/install, GET_MOTIONSENSOR)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/get_motionsensor.targetinstall:
	@$(call targetinfo)

	@$(call install_init, get_motionsensor)
	@$(call install_fixup, get_motionsensor,PRIORITY,optional)
	@$(call install_fixup, get_motionsensor,SECTION,base)
	@$(call install_fixup, get_motionsensor,AUTHOR,"<elrest>")
	@$(call install_fixup, get_motionsensor,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, get_motionsensor, 0, 0, 0755, $(GET_MOTIONSENSOR_DIR)/get_motionsensor, /etc/config-tools/get_motionsensor)

	@$(call install_finish, get_motionsensor)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/get_motionsensor.clean:
	@$(call targetinfo)
	cd $(GET_MOTIONSENSOR_DIR) && rm -f *.o $(GET_MOTIONSENSOR)	
	@-cd $(GET_MOTIONSENSOR_DIR) && \
		$(GET_MOTIONSENSOR_MAKE_ENV) $(GET_MOTIONSENSOR_PATH) $(MAKE) clean
	@$(call clean_pkg, GET_MOTIONSENSOR)

# vim: syntax=make
