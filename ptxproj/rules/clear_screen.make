# -*-makefile-*-
#
# Copyright (C) 2014 by <RG>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_CLEAR_SCREEN) += clear_screen

#
# Paths and names
#
CLEAR_SCREEN_VERSION	:= 1.0.1
CLEAR_SCREEN_MD5		:=
CLEAR_SCREEN		:= clear_screen-1
CLEAR_SCREEN_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/$(CLEAR_SCREEN)
CLEAR_SCREEN_DIR		:= $(BUILDDIR)/$(CLEAR_SCREEN)
CLEAR_SCREEN_BUILD_OOT	:= YES
CLEAR_SCREEN_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

#$(CLEAR_SCREEN_SOURCE):
#	@$(call targetinfo)
#	@$(call get, CLEAR_SCREEN)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#CLEAR_SCREEN_CONF_ENV	:= $(CROSS_ENV)

#
# qmake
#
CLEAR_SCREEN_PATH		:= PATH=$(PTXDIST_SYSROOT_CROSS)/bin/qt5:$(PTXDIST_SYSROOT_CROSS)/bin/qt6:$(CROSS_PATH)
CLEAR_SCREEN_CONF_TOOL	:= qmake
CLEAR_SCREEN_CONF_OPT	:= $(CROSS_QMAKE_OPT) PREFIX=/usr

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/clear_screen.targetinstall:
	@$(call targetinfo)

	@$(call install_init, clear_screen)
	@$(call install_fixup, clear_screen, PRIORITY, optional)
	@$(call install_fixup, clear_screen, SECTION, base)
	@$(call install_fixup, clear_screen, AUTHOR, "<RG>")
	@$(call install_fixup, clear_screen, DESCRIPTION, missing)

#	#
#	# example code:; copy all binaries
#	#

	@for i in $(shell cd $(CLEAR_SCREEN_PKGDIR) && find bin sbin usr/bin usr/sbin -type f); do \
		$(call install_copy, clear_screen, 0, 0, 0755, -, /$$i); \
	done

#	#
#	# FIXME: add all necessary things here
#	#

	@$(call install_finish, clear_screen)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/clear_screen.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, CLEAR_SCREEN)

# vim: syntax=make
