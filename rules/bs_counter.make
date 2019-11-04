# -*-makefile-*-
#
# Copyright (C) 2013 by <RG>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_BS_COUNTER) += bs_counter

#
# Paths and names
#
BS_COUNTER_VERSION	:= 1
BS_COUNTER_MD5		:=
BS_COUNTER		:= bs_counter
BS_COUNTER_URL		:= file://local_src/$(BS_COUNTER)
BS_COUNTER_DIR		:= $(BUILDDIR)/$(BS_COUNTER)
BS_COUNTER_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/bs_counter.extract:
#	@$(call targetinfo)
#	@$(call clean, $(BS_COUNTER_DIR))
#	@$(call extract, BS_COUNTER)
#	@$(call patchin, BS_COUNTER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#BS_COUNTER_PATH	:= PATH=$(CROSS_PATH)
BS_COUNTER_CONF_TOOL	:= NO
BS_COUNTER_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/bs_counter.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, BS_COUNTER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/bs_counter.compile:
#	@$(call targetinfo)
#	@$(call world/compile, BS_COUNTER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/bs_counter.install:
#	@$(call targetinfo)
#	@$(call world/install, BS_COUNTER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/bs_counter.targetinstall:
	@$(call targetinfo)

	@$(call install_init, bs_counter)
	@$(call install_fixup, bs_counter,PRIORITY,optional)
	@$(call install_fixup, bs_counter,SECTION,base)
	@$(call install_fixup, bs_counter,AUTHOR,"<RG>")
	@$(call install_fixup, bs_counter,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, bs_counter, 0, 0, 0755, $(BS_COUNTER_DIR)/tempd, /usr/bin/tempd)
	@$(call install_copy, bs_counter, 0, 0, 0755, $(BS_COUNTER_DIR)/tempd.sh, /etc/init.d/tempd)
#	@$(call install_link, bs_counter, ../init.d/codesys_screen, /etc/rc.d/$(PTXCONF_PLCLINUXRT_CODESYS_LINK));
	@$(call install_link, bs_counter, ../init.d/tempd, /etc/rc.d/S87_tempd) 

	@$(call install_finish, bs_counter)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/bs_counter.clean:
	@$(call targetinfo)
	@-cd $(BS_COUNTER_DIR) && \
		$(BS_COUNTER_MAKE_ENV) $(BS_COUNTER_PATH) $(MAKE) clean
	@$(call clean_pkg, BS_COUNTER)

# vim: syntax=make
