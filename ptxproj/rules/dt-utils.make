# -*-makefile-*-
#
# Copyright (C) 2013 by Sascha Hauer <s.hauer@pengutronix.de>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_DT_UTILS) += dt-utils

#
# Paths and names
#
DT_UTILS_VERSION	:= 2023.11.0
DT_UTILS_MD5		:= 4aa4ef310c76a2baa5df62254f0b7453
DT_UTILS		:= dt-utils-$(DT_UTILS_VERSION)
DT_UTILS_SUFFIX		:= tar.xz
DT_UTILS_URL		:= https://public.pengutronix.de/software/dt-utils/$(DT_UTILS).$(DT_UTILS_SUFFIX)
DT_UTILS_SOURCE		:= $(SRCDIR)/$(DT_UTILS).$(DT_UTILS_SUFFIX)
DT_UTILS_DIR		:= $(BUILDDIR)/$(DT_UTILS)

# Note: the source tarball includes files under GPL-3.0-or-later, CC0-1.0 and
# LicenseRef-DCO too, which don't make it into the compiled code.
DT_UTILS_LICENSE	:= GPL-2.0-only AND GPL-2.0-or-later AND Zlib
DT_UTILS_LICENSE_FILES	:= \
	file://README;startline=67;endline=83;md5=ac976dc9413ba8aee0286c950eca0374 \
	file://LICENSES/GPL-2.0-only.txt;md5=18d902a0242c37a4604224b47d02f802 \
	file://src/barebox-state/state_variables.c;startline=1;endline=5;md5=48db6eca86398f8f9806370f78598bf4 \
	file://LICENSES/GPL-2.0-or-later.txt;md5=df0715377b556699ce0837d45cb8bec9 \
	file://src/crc32.c;startline=1;endline=9;md5=ea2a8d555d3a705de4bdff531a08e257 \
	file://LICENSES/Zlib.txt;md5=48d37249fc5889f9a914aabb29781276

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

DT_UTILS_CONF_TOOL := meson
DT_UTILS_CONF_OPT  := \
	$(CROSS_MESON_USR) \
	-Dbarebox-state=true \
	-Dlock-device=true \
	-Dstate-backward-compatibility=$(call ptx/truefalse, PTXCONF_DT_UTILS_STATE_COMPAT) \
	-Dtests=false

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/dt-utils.targetinstall:
	@$(call targetinfo)

	@$(call install_init, dt-utils)
	@$(call install_fixup, dt-utils,PRIORITY,optional)
	@$(call install_fixup, dt-utils,SECTION,base)
	@$(call install_fixup, dt-utils,AUTHOR,"Sascha Hauer <s.hauer@pengutronix.de>")
	@$(call install_fixup, dt-utils,DESCRIPTION,missing)

	@$(call install_lib, dt-utils, 0, 0, 0644, libdt-utils)
ifdef PTXCONF_BAREBOX
	@$(call install_copy, dt-utils, 0, 0, 0755, -, /usr/bin/barebox-state)
endif
	@$(call install_copy, dt-utils, 0, 0, 0755, -, /usr/bin/fdtdump)

	@$(call install_finish, dt-utils)

	@$(call touch)

# vim: syntax=make
