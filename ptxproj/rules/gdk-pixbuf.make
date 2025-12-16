# -*-makefile-*-
#
# Copyright (C) 2011 by Robert Schwebel <r.schwebel@pengutronix.de>
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_GDK_PIXBUF) += gdk-pixbuf

#
# Paths and names
#
GDK_PIXBUF_VERSION	:= 2.42.10
GDK_PIXBUF_MD5		:= 4a62f339cb1424693fba9bb7ffef8150
GDK_PIXBUF		:= gdk-pixbuf-$(GDK_PIXBUF_VERSION)
GDK_PIXBUF_SUFFIX	:= tar.xz
GDK_PIXBUF_URL		:= $(call ptx/mirror, GNOME, gdk-pixbuf/$(basename $(GDK_PIXBUF_VERSION))/$(GDK_PIXBUF).$(GDK_PIXBUF_SUFFIX))
GDK_PIXBUF_SOURCE	:= $(SRCDIR)/$(GDK_PIXBUF).$(GDK_PIXBUF_SUFFIX)
GDK_PIXBUF_DIR		:= $(BUILDDIR)/$(GDK_PIXBUF)
GDK_PIXBUF_LICENSE	:= LGPL-2.0-or-later
GDK_PIXBUF_LICENSE_FILES := file://COPYING;md5=4fbd65380cdd255951079008b364516c

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

GDK_PIXBUF_CONF_ENV	:= $(CROSS_ENV) \
	gio_can_sniff=yes

GDK_PIXBUF_LOADER-$(PTXCONF_GDK_PIXBUF_LOADER_PNG)	+= png
GDK_PIXBUF_LOADER-$(PTXCONF_GDK_PIXBUF_LOADER_JPEG)	+= jpeg

#
# meson
#
GDK_PIXBUF_CONF_TOOL	:= meson
GDK_PIXBUF_CONF_OPT	:= \
	$(CROSS_MESON_USR) \
	-Dbuiltin_loaders=$(subst $(space),$(comma),$(GDK_PIXBUF_LOADER-y)) \
	-Ddocs=false \
	-Dgio_sniffing=false \
	-Dgtk_doc=false \
	-Dinstalled_tests=false \
	-Dintrospection=disabled \
	-Djpeg=$(call ptx/endis, PTXCONF_GDK_PIXBUF_LOADER_JPEG)d \
	-Dman=false \
	-Dnative_windows_loaders=false \
	-Dpng=$(call ptx/endis, PTXCONF_GDK_PIXBUF_LOADER_PNG)d \
	-Drelocatable=false \
	-Dtests=false \
	-Dtiff=disabled \
	-Dothers=enabled

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/gdk-pixbuf.targetinstall:
	@$(call targetinfo)

	@$(call install_init, gdk-pixbuf)
	@$(call install_fixup, gdk-pixbuf,PRIORITY,optional)
	@$(call install_fixup, gdk-pixbuf,SECTION,base)
	@$(call install_fixup, gdk-pixbuf,AUTHOR,"Robert Schwebel <r.schwebel@pengutronix.de>")
	@$(call install_fixup, gdk-pixbuf,DESCRIPTION,missing)

	@$(call install_lib, gdk-pixbuf, 0, 0, 0644, libgdk_pixbuf-2.0)

	@$(call install_finish, gdk-pixbuf)

	@$(call touch)

# vim: syntax=make
