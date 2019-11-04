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
PACKAGES-$(PTXCONF_QT_SOUND) += qt_sound

#
# Paths and names
#
QT_SOUND_VERSION	:= 1
QT_SOUND_MD5		:=
QT_SOUND			:= qt_sound
QT_SOUND_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/$(QT_SOUND)
QT_SOUND_DIR		:= $(BUILDDIR)/$(QT_SOUND)
QT_SOUND_BUILD_OOT	:= YES
QT_SOUND_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

#$(QT_SOUND_SOURCE):
#	@$(call targetinfo)
#	@$(call targetinfo)
#	sed -i s/"-lXinerama -lfontconfig -lXrandr"/"-lXinerama -lfontconfig -lXrandr -lSM -lICE -lXcursor"/ $(PTXCONF_SYSROOT_TARGET)/usr/mkspecs/linux-ptx-g++/qmake.conf
#	@$(call touch)
#	@$(call get, QT_SOUND)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#QT_SOUND_CONF_ENV	:= $(CROSS_ENV)

#
# qmake
#
QT_SOUND_CONF_TOOL	:= qmake
QT_SOUND_CONF_OPT	:= $(CROSS_QMAKE_OPT) PREFIX=/usr

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/qt_sound.targetinstall:
	@$(call targetinfo)

	@$(call install_init, qt_sound)
	@$(call install_fixup, qt_sound, PRIORITY, optional)
	@$(call install_fixup, qt_sound, SECTION, base)
	@$(call install_fixup, qt_sound, AUTHOR, "<elrest>")
	@$(call install_fixup, qt_sound, DESCRIPTION, missing)

#	#
#	# example code:; copy all binaries
#	#

	@for i in $(shell cd $(QT_SOUND_PKGDIR) && find bin sbin usr/bin usr/sbin -type f); do \
		$(call install_copy, qt_sound, 0, 0, 0755, -, /$$i); \
	done

#	#
#	# FIXME: add all necessary things here
#	#

	@$(call install_finish, qt_sound)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/qt_sound.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, WAITHITTOUCH)

# vim: syntax=make
