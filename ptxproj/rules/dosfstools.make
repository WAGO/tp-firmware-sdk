# -*-makefile-*-
#
# Copyright (C) 2005 by Steven Scholz <steven.scholz@imc-berlin.de>
#               2009 by Marc Kleine-Budde <mkl@pengutronix.de>
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_DOSFSTOOLS) += dosfstools

#
# Paths and names
#
DOSFSTOOLS_VERSION	:= 4.2
DOSFSTOOLS_MD5		:= 49c8e457327dc61efab5b115a27b087a
DOSFSTOOLS		:= dosfstools-$(DOSFSTOOLS_VERSION)
DOSFSTOOLS_SUFFIX	:= tar.gz
DOSFSTOOLS_SRC		:= $(DOSFSTOOLS).$(DOSFSTOOLS_SUFFIX)
DOSFSTOOLS_URL		:= https://github.com/dosfstools/dosfstools/releases/download/v$(DOSFSTOOLS_VERSION)/$(DOSFSTOOLS_SRC)
DOSFSTOOLS_SOURCE	:= $(SRCDIR)/$(DOSFSTOOLS_SRC)
DOSFSTOOLS_DIR		:= $(BUILDDIR)/$(DOSFSTOOLS)
DOSFSTOOLS_LICENSE	:= GPL-3.0-or-later
DOSFSTOOLS_LICENSE_FILES	:= \
	file://COPYING;md5=d32239bcb673463ab874e80d47fae504 \
	file://src/fsck.fat.c;startline=1;endline=23;md5=29df02b73d125363cfcbacaa45707eed

# ----------------------------------------------------------------------------
# Prepare (nothing to be done here)
# ----------------------------------------------------------------------------

DOSFSTOOLS_CONF_TOOL	:= autoconf
DOSFSTOOLS_CONF_OPT	:= \
	$(CROSS_AUTOCONF_USR) \
	--enable-compat-symlinks \
	--disable-atari-check \
	$(GLOBAL_LARGE_FILE_OPTION) \
	--disable-rpath \
	--without-iconv

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/dosfstools.targetinstall:
	@$(call targetinfo)

	@$(call install_init, dosfstools)
	@$(call install_fixup, dosfstools,PRIORITY,optional)
	@$(call install_fixup, dosfstools,SECTION,base)
	@$(call install_fixup, dosfstools,AUTHOR,"Steven Scholz <steven.scholz@imc-berlin.de>")
	@$(call install_fixup, dosfstools,DESCRIPTION,missing)

ifdef PTXCONF_DOSFSTOOLS_MKFS_FAT
	@$(call install_copy, dosfstools, 0, 0, 0755, -, \
		/usr/sbin/mkfs.fat)
endif
ifdef PTXCONF_DOSFSTOOLS_MKFS_FAT_MSDOS
	@$(call install_link, dosfstools, mkfs.fat, /usr/sbin/mkfs.msdos)
endif
ifdef PTXCONF_DOSFSTOOLS_MKFS_FAT_VFAT
	@$(call install_link, dosfstools, mkfs.fat, /usr/sbin/mkfs.vfat)
endif


ifdef PTXCONF_DOSFSTOOLS_FSCK_FAT
	@$(call install_copy, dosfstools, 0, 0, 0755, -, \
		/usr/sbin/fsck.fat)
endif
ifdef PTXCONF_DOSFSTOOLS_FSCK_FAT_MSDOS
	@$(call install_link, dosfstools, fsck.fat, /usr/sbin/fsck.msdos)
endif
ifdef PTXCONF_DOSFSTOOLS_FSCK_FAT_VFAT
	@$(call install_link, dosfstools, fsck.fat, /usr/sbin/fsck.vfat)
endif

ifdef PTXCONF_DOSFSTOOLS_FATLABEL
	@$(call install_copy, dosfstools, 0, 0, 0755, -, \
		/usr/sbin/fatlabel)
endif

	@$(call install_finish, dosfstools)

	@$(call touch)

# vim: syntax=make
