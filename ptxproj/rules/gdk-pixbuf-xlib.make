# -*-makefile-*-
#
# Copyright (C) 2025 by elrest Automationssysteme GmbH <www.elrest.de>
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_GDK_PIXBUF_XLIB) += gdk-pixbuf-xlib

#
# Paths and names
#
GDK_PIXBUF_XLIB_VERSION	:= 2.40.2
GDK_PIXBUF_XLIB_MD5		:= fbd57e867e039a8cf9164d145c0f0434
GDK_PIXBUF_XLIB			:= gdk-pixbuf-xlib-$(GDK_PIXBUF_XLIB_VERSION)
GDK_PIXBUF_XLIB_SUFFIX	:= tar.xz
GDK_PIXBUF_XLIB_URL		:= $(call ptx/mirror, GNOME, gdk-pixbuf/$(basename $(GDK_PIXBUF_XLIB_VERSION))/$(GDK_PIXBUF_XLIB).$(GDK_PIXBUF_XLIB_SUFFIX))
GDK_PIXBUF_XLIB_SOURCE	:= $(SRCDIR)/$(GDK_PIXBUF_XLIB).$(GDK_PIXBUF_XLIB_SUFFIX)
GDK_PIXBUF_XLIB_DIR		:= $(BUILDDIR)/$(GDK_PIXBUF_XLIB)
GDK_PIXBUF_XLIB_LICENSE	:= LGPL-2.0-or-later
GDK_PIXBUF_XLIB_LICENSE_FILES := file://COPYING;md5=4fbd65380cdd255951079008b364516c

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

GDK_PIXBUF_XLIB_CONF_ENV	:= $(CROSS_ENV)

#
# meson
#
GDK_PIXBUF_XLIB_CONF_TOOL	:= meson
GDK_PIXBUF_XLIB_CONF_OPT	:= \
	$(CROSS_MESON_USR)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/gdk-pixbuf-xlib.targetinstall:
	@$(call targetinfo)

	@$(call install_init, gdk-pixbuf-xlib)
	@$(call install_fixup, gdk-pixbuf-xlib,PRIORITY,optional)
	@$(call install_fixup, gdk-pixbuf-xlib,SECTION,base)
	@$(call install_fixup, gdk-pixbuf-xlib,AUTHOR,"elrest Automationssysteme GmbH <www.elrest.de>")
	@$(call install_fixup, gdk-pixbuf-xlib,DESCRIPTION,missing)

	@$(call install_lib, gdk-pixbuf-xlib, 0, 0, 0644, libgdk_pixbuf_xlib-2.0)

	@$(call install_finish, gdk-pixbuf-xlib)

	@$(call touch)

# vim: syntax=make
