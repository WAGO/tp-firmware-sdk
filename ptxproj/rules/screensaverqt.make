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
PACKAGES-$(PTXCONF_SCREENSAVERQT) += screensaverqt

#
# Paths and names
#
ifdef PTXCONF_SCREENSAVERQT_TRUNK
SCREENSAVERQT_VERSION	:= trunk
else
SCREENSAVERQT_VERSION	:= 1.0.1
SCREENSAVERQT_MD5		:=
endif
SCREENSAVERQT		:= screensaverqt
SCREENSAVERQT_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/$(SCREENSAVERQT)
SCREENSAVERQT_DIR		:= $(BUILDDIR)/$(SCREENSAVERQT)
SCREENSAVERQT_BUILD_OOT	:= YES
SCREENSAVERQT_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

#$(SCREENSAVERQT_SOURCE):
#	@$(call targetinfo)
#	@$(call get, SCREENSAVERQT)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#SCREENSAVERQT_CONF_ENV	:= $(CROSS_ENV)

#
# qmake
#
SCREENSAVERQT_PATH		:= PATH=$(PTXDIST_SYSROOT_CROSS)/bin/qt5:$(PTXDIST_SYSROOT_CROSS)/bin/qt6:$(CROSS_PATH)
SCREENSAVERQT_CONF_TOOL	:= qmake
SCREENSAVERQT_CONF_OPT	:= $(CROSS_QMAKE_OPT) PREFIX=/usr

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/screensaverqt.targetinstall:
	@$(call targetinfo)

	@$(call install_init, screensaverqt)
	@$(call install_fixup, screensaverqt, PRIORITY, optional)
	@$(call install_fixup, screensaverqt, SECTION, base)
	@$(call install_fixup, screensaverqt, AUTHOR, "<elrest>")
	@$(call install_fixup, screensaverqt, DESCRIPTION, missing)

#	#
#	# example code:; copy all binaries
#	#

	@for i in $(shell cd $(SCREENSAVERQT_PKGDIR) && find bin sbin usr/bin usr/sbin -type f); do \
		$(call install_copy, screensaverqt, 0, 0, 0755, -, /$$i); \
	done

#	#
#	# FIXME: add all necessary things here
#	#

	@$(call install_finish, screensaverqt)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/screensaverqt.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, SCREENSAVERQT)

# vim: syntax=make
