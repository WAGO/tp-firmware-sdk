# -*-makefile-*-
#
# Copyright (C) 2017 by <elrest>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_SCREENSHOTQT) += screenshotqt

#
# Paths and names
#
ifdef PTXCONF_SCREENSHOTQT_TRUNK
SCREENSHOTQT_VERSION	:= trunk
else
SCREENSHOTQT_VERSION	:= 1
SCREENSHOTQT_MD5		:=
endif
#SCREENSHOTQT		:= screenshotqt-$(SCREENSHOTQT_VERSION)
SCREENSHOTQT		:= screenshotqt
SCREENSHOTQT_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/$(SCREENSHOTQT)
SCREENSHOTQT_DIR		:= $(BUILDDIR)/$(SCREENSHOTQT)
SCREENSHOTQT_BUILD_OOT	:= YES
SCREENSHOTQT_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

#$(SCREENSHOTQT_SOURCE):
#	@$(call targetinfo)
#	@$(call get, SCREENSHOTQT)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#SCREENSHOTQT_CONF_ENV	:= $(CROSS_ENV)

#
# qmake
#
SCREENSHOTQT_PATH		:= PATH=$(PTXDIST_SYSROOT_CROSS)/bin/qt5:$(PTXDIST_SYSROOT_CROSS)/bin/qt6:$(CROSS_PATH)
SCREENSHOTQT_CONF_TOOL	:= qmake
SCREENSHOTQT_CONF_OPT	:= $(CROSS_QMAKE_OPT) PREFIX=/usr

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/screenshotqt.targetinstall:
	@$(call targetinfo)

	@$(call install_init, screenshotqt)
	@$(call install_fixup, screenshotqt, PRIORITY, optional)
	@$(call install_fixup, screenshotqt, SECTION, base)
	@$(call install_fixup, screenshotqt, AUTHOR, "<elrest>")
	@$(call install_fixup, screenshotqt, DESCRIPTION, missing)

#	#
#	# example code:; copy all binaries
#	#

	@for i in $(shell cd $(SCREENSHOTQT_PKGDIR) && find bin sbin usr/bin usr/sbin -type f); do \
		$(call install_copy, screenshotqt, 0, 0, 0755, -, /$$i); \
	done

#	#
#	# FIXME: add all necessary things here
#	#

	@$(call install_finish, screenshotqt)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/screenshotqt.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, SCREENSHOTQT)

# vim: syntax=make
