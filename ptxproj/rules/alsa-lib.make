# -*-makefile-*-
#
# Copyright (C) 2006 by Erwin Rol
#               2009 by Marc Kleine-Budde <mkl@pengutronix.de>
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_ALSA_LIB) += alsa-lib

#
# Paths and names
#
ALSA_LIB_VERSION	:= 1.2.8
ALSA_LIB_MD5		:= ab32de4b6054d7ab23723f8d85e50a27
ALSA_LIB		:= alsa-lib-$(ALSA_LIB_VERSION)
ALSA_LIB_SUFFIX		:= tar.bz2
ALSA_LIB_URL		:= \
	https://www.alsa-project.org/files/pub/lib/$(ALSA_LIB).$(ALSA_LIB_SUFFIX) \
	ftp://ftp.alsa-project.org/pub/lib/$(ALSA_LIB).$(ALSA_LIB_SUFFIX)
ALSA_LIB_SOURCE		:= $(SRCDIR)/$(ALSA_LIB).$(ALSA_LIB_SUFFIX)
ALSA_LIB_DIR		:= $(BUILDDIR)/$(ALSA_LIB)
ALSA_LIB_LICENSE	:= LGPL-2.1-or-later
ALSA_LIB_LICENSE_FILES	:= \
	file://src/async.c;startline=7;endline=25;md5=dcff7cc43cebd5f1816961f1016b162f \
	file://COPYING;md5=a916467b91076e631dd8edb7424769c7

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#
# autoconf
#
ALSA_LIB_CONF_TOOL	:= autoconf
ALSA_LIB_CONF_OPT	:= \
	$(CROSS_AUTOCONF_USR) \
	--enable-shared \
	$(GLOBAL_LARGE_FILE_OPTION) \
	--enable-symbolic-functions \
	--with-debug=no \
	--$(call ptx/endis, PTXCONF_ALSA_LIB_RESMGR)-resmgr \
	--$(call ptx/endis, PTXCONF_ALSA_LIB_READ)-aload \
	--$(call ptx/endis, PTXCONF_ALSA_LIB_MIXER)-mixer \
	--$(call ptx/endis, PTXCONF_ALSA_LIB_PCM)-pcm \
	--$(call ptx/endis, PTXCONF_ALSA_LIB_RAWMIDI)-rawmidi \
	--$(call ptx/endis, PTXCONF_ALSA_LIB_HWDEP)-hwdep \
	--$(call ptx/endis, PTXCONF_ALSA_LIB_SEQ)-seq \
	--$(call ptx/endis, PTXCONF_ALSA_LIB_UCM)-ucm \
	--disable-topology \
	--$(call ptx/endis, PTXCONF_ALSA_LIB_ALISP)-alisp \
	--disable-old-symbols \
	--disable-mixer-modules \
	--disable-mixer-pymods \
	--disable-python \
	--disable-python2 \
	--disable-lockless-dmix \
	--enable-thread-safety \
	--with-versioned \
	--with-tmpdir=/tmp \
	--with-softfloat=$(call ptx/ifdef, PTXCONF_HAS_HARDFLOAT, no, yes) \
	--with-libdl \
	--with-pthread \
	--with-librt \
	--without-wordexp \
	--with-alsa-devdir=/dev/snd \
	--with-aload-devdir=/dev

ifdef PTXCONF_ALSA_LIB_PCM
ALSA_LIB_CONF_OPT += \
	--with-pcm-plugins=$(PTXCONF_ALSA_LIB_PCM_MODULES)
endif

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/alsa-lib.targetinstall:
	@$(call targetinfo)

	@$(call install_init, alsa-lib)
	@$(call install_fixup, alsa-lib, PRIORITY,optional)
	@$(call install_fixup, alsa-lib, SECTION,base)
	@$(call install_fixup, alsa-lib, AUTHOR,"Erwin Rol <ero@pengutronix.de>")
	@$(call install_fixup, alsa-lib, DESCRIPTION,missing)

	@$(call install_lib, alsa-lib, 0, 0, 0644, libasound)
	@$(call install_tree, alsa-lib, 0, 0, -, /usr/share/alsa)
ifdef PTXCONF_ALSA_LIB_ASOUND_CONF
	@$(call install_alternative, alsa-lib, 0, 0, 0644, /etc/asound.conf)
endif

	@$(call install_finish, alsa-lib)

	@$(call touch)

# vim: syntax=make
