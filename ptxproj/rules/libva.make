# -*-makefile-*-
#
# Copyright (C) 2014 by Michael Olbrich <m.olbrich@pengutronix.de>
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_LIBVA) += libva

#
# Paths and names
#
LIBVA_VERSION	:= 2.8.0
LIBVA_MD5	:= 6f56739bb46406d2e0a0a3a62b3bf5a5
LIBVA		:= libva-$(LIBVA_VERSION)
LIBVA_SUFFIX	:= tar.bz2
LIBVA_URL	:= https://github.com/intel/libva/releases/download/$(LIBVA_VERSION)/$(LIBVA).$(LIBVA_SUFFIX)
LIBVA_SOURCE	:= $(SRCDIR)/$(LIBVA).$(LIBVA_SUFFIX)
LIBVA_DIR	:= $(BUILDDIR)/$(LIBVA)
LIBVA_LICENSE	:= MIT

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

LIBVA_ENABLE-y				:= drm
LIBVA_ENABLE-$(PTXCONF_LIBVA_X11)	+= x11
LIBVA_ENABLE-$(PTXCONF_LIBVA_GLX)	+= glx
LIBVA_ENABLE-$(PTXCONF_LIBVA_WAYLAND)	+= wayland

LIBVA_CONF_TOOL	:= meson
ifeq ($(PTXCONF_CONFIGFILE_VERSION), "2020.08.0")
LIBVA_CONF_OPT	:= \
	$(CROSS_MESON_USR) \
	-Ddisable_drm=false \
	-Ddriverdir='' \
	-Denable_docs=false \
	-Denable_va_messaging=true \
	-Dwith_glx=$(call ptx/yesno, PTXCONF_LIBVA_GLX) \
	-Dwith_wayland=$(call ptx/yesno, PTXCONF_LIBVA_WAYLAND) \
	-Dwith_x11=$(call ptx/yesno, PTXCONF_LIBVA_X11)
else
LIBVA_CONF_OPT	:= \
	$(CROSS_MESON_USR) \
	-Ddisable_drm=false \
	-Ddriverdir='' \
	-Denable_docs=false \
	-Denable_va_messaging=true \
	-Dwith_glx=$(call ptx/ifdef, PTXCONF_LIBVA_GLX, yes, no) \
	-Dwith_wayland=$(call ptx/ifdef, PTXCONF_LIBVA_WAYLAND, yes, no) \
	-Dwith_x11=$(call ptx/ifdef, PTXCONF_LIBVA_X11, yes, no)
endif

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/libva.targetinstall:
	@$(call targetinfo)

	@$(call install_init, libva)
	@$(call install_fixup, libva,PRIORITY,optional)
	@$(call install_fixup, libva,SECTION,base)
	@$(call install_fixup, libva,AUTHOR,"Michael Olbrich <m.olbrich@pengutronix.de>")
	@$(call install_fixup, libva,DESCRIPTION,missing)

	@$(call install_lib, libva, 0, 0, 0644, libva)

	@$(foreach api, $(LIBVA_ENABLE-y), \
		$(call install_lib, libva, 0, 0, 0644, libva-$(api));)

	@$(call install_finish, libva)

	@$(call touch)

# vim: syntax=make
