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
PACKAGES-$(PTXCONF_GET_TESTABILITY) += get_testability

#
# Paths and names
#
GET_TESTABILITY_VERSION	:= 1
GET_TESTABILITY_MD5		:=
GET_TESTABILITY		:= get_testability
GET_TESTABILITY_URL		:= file://local_src/$(GET_TESTABILITY)
GET_TESTABILITY_DIR		:= $(BUILDDIR)/$(GET_TESTABILITY)
GET_TESTABILITY_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_testability.extract:
#	@$(call targetinfo)
#	@$(call clean, $(GET_TESTABILITY_DIR))
#	@$(call extract, GET_TESTABILITY)
#	@$(call patchin, GET_TESTABILITY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#GET_TESTABILITY_PATH	:= PATH=$(CROSS_PATH)
GET_TESTABILITY_CONF_TOOL	:= NO
GET_TESTABILITY_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/get_testability.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, GET_TESTABILITY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_testability.compile:
#	@$(call targetinfo)
#	@$(call world/compile, GET_TESTABILITY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_testability.install:
#	@$(call targetinfo)
#	@$(call world/install, GET_TESTABILITY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/get_testability.targetinstall:
	@$(call targetinfo)

	@$(call install_init, get_testability)
	@$(call install_fixup, get_testability,PRIORITY,optional)
	@$(call install_fixup, get_testability,SECTION,base)
	@$(call install_fixup, get_testability,AUTHOR,"<elrest>")
	@$(call install_fixup, get_testability,DESCRIPTION,missing)

	@$(call install_copy, get_testability, 0, 0, 0755, /etc/config-tools);

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, get_testability, 0, 0, 0755, $(GET_TESTABILITY_DIR)/get_testability, /etc/config-tools/get_testability)

	@$(call install_finish, get_testability)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/get_testability.clean:
	@$(call targetinfo)
	cd $(GET_TESTABILITY_DIR) && rm -f *.o $(GET_TESTABILITY)	
	@-cd $(GET_TESTABILITY_DIR) && \
		$(GET_TESTABILITY_ENV) $(GET_TESTABILITY_PATH) $(MAKE) clean
	@$(call clean_pkg, GET_TESTABILITY)

# vim: syntax=make
