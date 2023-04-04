# -*-makefile-*-
#
# Copyright (C) 2011 by <RG>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_SETUP_DISPLAY) += setup_display

#
# Paths and names
#
SETUP_DISPLAY_VERSION	:= 1
SETUP_DISPLAY		:= setup_display
SETUP_DISPLAY_URL		:= file://local_src/$(SETUP_DISPLAY)
SETUP_DISPLAY_DIR		:= $(BUILDDIR)/$(SETUP_DISPLAY)
SETUP_DISPLAY_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/setup_display.extract:
#	@$(call targetinfo)
#	@$(call clean, $(SETUP_DISPLAY_DIR))
#	@$(call extract, SETUP_DISPLAY)
#	@$(call patchin, SETUP_DISPLAY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#SETUP_DISPLAY_PATH	:= PATH=$(CROSS_PATH)
SETUP_DISPLAY_CONF_TOOL	:= NO
SETUP_DISPLAY_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/setup_display.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, SETUP_DISPLAY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/setup_display.compile:
	@$(call targetinfo)
	cd $(SETUP_DISPLAY_DIR) && \
	make CFLAGS=-I$(BUILDDIR)/../sysroot-target/usr/include LD=$(COMPILER_PREFIX)ld\
  CC=$(COMPILER_PREFIX)gcc CFLAGS="-I$(BUILDDIR)/../sysroot-target/usr/include"\
  LDADD="$(BUILDDIR)/../sysroot-target/usr/lib/lconfparse.so"\
  LDFLAGS="-lconfparse -L$(BUILDDIR)/../sysroot-target/usr/lib"
	@$(call touch)
  
# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/setup_display.install:
#	@$(call targetinfo)
#	@$(call world/install, SETUP_DISPLAY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/setup_display.targetinstall:
	@$(call targetinfo)

	@$(call install_init, setup_display)
	@$(call install_fixup, setup_display,PRIORITY,optional)
	@$(call install_fixup, setup_display,SECTION,base)
	@$(call install_fixup, setup_display,AUTHOR,"<RG>")
	@$(call install_fixup, setup_display,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, setup_display, 0, 0, 0755, $(SETUP_DISPLAY_DIR)/setup_display, /usr/bin/setup_display)

	@$(call install_finish, setup_display)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/setup_display.clean:
	@$(call targetinfo)
	@-cd $(SETUP_DISPLAY_DIR) && \
		$(SETUP_DISPLAY_MAKE_ENV) $(SETUP_DISPLAY_PATH) $(MAKE) clean
	@$(call clean_pkg, SETUP_DISPLAY)

# vim: syntax=make
