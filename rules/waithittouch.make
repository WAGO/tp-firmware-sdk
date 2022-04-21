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
PACKAGES-$(PTXCONF_WAITHITTOUCH) += waithittouch

#
# Paths and names
#
WAITHITTOUCH_VERSION	:= 1.0.1
WAITHITTOUCH_MD5		:=
WAITHITTOUCH		:= waithittouch
WAITHITTOUCH_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/$(WAITHITTOUCH)
WAITHITTOUCH_DIR		:= $(BUILDDIR)/$(WAITHITTOUCH)
WAITHITTOUCH_BUILD_OOT	:= YES
WAITHITTOUCH_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

#$(WAITHITTOUCH_SOURCE):
#	@$(call targetinfo)
#	@$(call targetinfo)
#	sed -i s/"-lXinerama -lfontconfig -lXrandr"/"-lXinerama -lfontconfig -lXrandr -lSM -lICE -lXcursor"/ $(PTXCONF_SYSROOT_TARGET)/usr/mkspecs/linux-ptx-g++/qmake.conf
#	@$(call touch)
#	@$(call get, WAITHITTOUCH)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#WAITHITTOUCH_CONF_ENV	:= $(CROSS_ENV)

#
# qmake
#
WAITHITTOUCH_CONF_TOOL	:= qmake
WAITHITTOUCH_CONF_OPT	:= $(CROSS_QMAKE_OPT) PREFIX=/usr

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/waithittouch.targetinstall:
	@$(call targetinfo)

	@$(call install_init, waithittouch)
	@$(call install_fixup, waithittouch, PRIORITY, optional)
	@$(call install_fixup, waithittouch, SECTION, base)
	@$(call install_fixup, waithittouch, AUTHOR, "<elrest>")
	@$(call install_fixup, waithittouch, DESCRIPTION, missing)

#	#
#	# example code:; copy all binaries
#	#

	@for i in $(shell cd $(WAITHITTOUCH_PKGDIR) && find bin sbin usr/bin usr/sbin -type f); do \
		$(call install_copy, waithittouch, 0, 0, 0755, -, /$$i); \
	done

#	#
#	# FIXME: add all necessary things here
#	#

	@$(call install_finish, waithittouch)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/waithittouch.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, WAITHITTOUCH)

# vim: syntax=make
