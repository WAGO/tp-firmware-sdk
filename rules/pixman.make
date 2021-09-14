# -*-makefile-*-
#
# Copyright (C) 2007,2012 by Michael Olbrich <m.olbrich@pengutronix.de>
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_PIXMAN) += pixman

#
# Paths and names
#
PIXMAN_VERSION	:= 0.40.0
PIXMAN_MD5	:= ee642c14f4f18934246c57a440da9ca2
PIXMAN		:= pixman-$(PIXMAN_VERSION)
PIXMAN_SUFFIX	:= tar.xz
PIXMAN_URL	:= $(call ptx/mirror, XORG, individual/lib/$(PIXMAN).$(PIXMAN_SUFFIX))
PIXMAN_SOURCE	:= $(SRCDIR)/$(PIXMAN).$(PIXMAN_SUFFIX)
PIXMAN_DIR	:= $(BUILDDIR)/$(PIXMAN)
PIXMAN_LICENSE	:= MIT

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#
# meson
#
PIXMAN_CONF_TOOL	:= meson
PIXMAN_CONF_OPT		:= \
	$(CROSS_MESON_USR) \
	-Darm-simd=$(call ptx/endis, PTXCONF_ARCH_ARM_V6)d \
	-Dgnu-inline-asm=disabled \
	-Dgnuplot=false \
	-Dgtk=disabled \
	-Diwmmxt=disabled \
	-Diwmmxt2=false \
	-Dlibpng=disabled \
	-Dloongson-mmi=disabled \
	-Dmips-dspr2=disabled \
	-Dmmx=$(call ptx/endis, PTXCONF_ARCH_X86)d \
	-Dneon=$(call ptx/endis, PTXCONF_ARCH_ARM_NEON)d \
	-Dopenmp=disabled \
	-Dsse2=$(call ptx/endis, PTXCONF_ARCH_X86)d \
	-Dssse3=$(call ptx/endis, PTXCONF_ARCH_X86)d \
	-Dtimers=false \
	-Dvmx=disabled

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/pixman.targetinstall:
	@$(call targetinfo)

	@$(call install_init, pixman)
	@$(call install_fixup, pixman,PRIORITY,optional)
	@$(call install_fixup, pixman,SECTION,base)
	@$(call install_fixup, pixman,AUTHOR,"Robert Schwebel <r.schwebel@pengutronix.de>")
	@$(call install_fixup, pixman,DESCRIPTION,missing)

	@$(call install_lib, pixman, 0, 0, 0644, libpixman-1)

	@$(call install_finish, pixman)

	@$(call touch)

# vim: syntax=make
