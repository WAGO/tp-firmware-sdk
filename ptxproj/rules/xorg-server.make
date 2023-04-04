# -*-makefile-*-
#
# Copyright (C) 2006, 2009 by Robert Schwebel
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_XORG_SERVER) += xorg-server

#
# Paths and names
#
XORG_SERVER_VERSION	:= 21.1.6
XORG_SERVER_MD5		:= b3d9a66554b2fd72960ea0545a87ea80
XORG_SERVER		:= xorg-server-$(XORG_SERVER_VERSION)
XORG_SERVER_SUFFIX	:= tar.xz
XORG_SERVER_URL		:= $(call ptx/mirror, XORG, individual/xserver/$(XORG_SERVER).$(XORG_SERVER_SUFFIX))
XORG_SERVER_SOURCE	:= $(SRCDIR)/$(XORG_SERVER).$(XORG_SERVER_SUFFIX)
XORG_SERVER_DIR		:= $(BUILDDIR)/$(XORG_SERVER)
XORG_SERVER_LICENSE	:= MIT

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

# The xorg module loader needs lazy symbol binding
XORG_SERVER_WRAPPER_BLACKLIST := \
        TARGET_HARDEN_BINDNOW

#
# FIXME: not all processors support MTRR. Geode GX1 not for
# example. But it is a 586 clone. configure decides always to support
# mtrr!
#
# XORG_SERVER_ENV		+= ac_cv_asm_mtrr_h=no

XORG_FONT_DIRS	:= \
	misc \
	truetype \
	opentype \
	100dpi \
	Type1 \
	75dpi \
	cyrillic \
	Speedo


#
# meson
#
XORG_SERVER_CONF_TOOL	:= meson
# use "=" here
XORG_SERVER_CONF_OPT	= \
	$(CROSS_MESON_USR) \
	-Dbuilder_addr=ptxdist@pengutronix.de \
	-Dbuilder_string=PTXdist \
	-Ddefault_font_path=$(subst $(space),$(comma),$(addprefix $(XORG_FONTDIR)/,$(XORG_FONT_DIRS))) \
	-Ddga=$(call ptx/truefalse, PTXCONF_XORG_SERVER_EXT_DGA) \
	-Ddmx=$(call ptx/truefalse, PTXCONF_XORG_SERVER_DMX) \
	-Ddpms=$(call ptx/truefalse, PTXCONF_XORG_SERVER_EXT_DPMS) \
	-Ddri1=$(call ptx/truefalse, PTXCONF_XORG_SERVER_EXT_DRI) \
	-Ddri2=$(call ptx/truefalse, PTXCONF_XORG_SERVER_EXT_DRI2) \
	-Ddri3=$(call ptx/truefalse, PTXCONF_XORG_SERVER_EXT_DRI3) \
	-Dglamor=$(call ptx/truefalse, PTXCONF_XORG_SERVER_GLAMOR) \
	-Dglx=$(call ptx/truefalse, PTXCONF_XORG_SERVER_EXT_GLX) \
	-Dhal=false \
	-Dipv6=$(call ptx/truefalse, PTXCONF_GLOBAL_IPV6) \
	-Dlinux_acpi=true \
	-Dlinux_apm=true \
	-Dlisten_local=true \
	-Dlisten_tcp=false \
	-Dlisten_unix=true \
	-Dmitshm=$(call ptx/truefalse, PTXCONF_XORG_SERVER_EXT_SHM) \
	-Dos_vendor=PTXdist \
	-Dpciaccess=$(call ptx/truefalse, PTXCONF_XORG_SERVER_XORG) \
	-Dscreensaver=$(call ptx/truefalse, PTXCONF_XORG_SERVER_EXT_SCREENSAVER) \
	-Dsecure-rpc=$(call ptx/truefalse, PTXCONF_XORG_SERVER_OPT_SECURE_RPC) \
	-Dsuid_wrapper=false \
	-Dsystemd_logind=false \
	-Dudev=$(call ptx/truefalse, PTXCONF_XORG_SERVER_UDEV) \
	-Dvbe=true \
	-Dvgahw=true \
	-Dxace=$(call ptx/truefalse, PTXCONF_XORG_SERVER_EXT_XACE) \
	-Dxcsecurity=$(call ptx/truefalse, PTXCONF_XORG_SERVER_EXT_XCSECURITY) \
	-Dxdm-auth-1=$(call ptx/truefalse, PTXCONF_XORG_SERVER_EXT_XDM_AUTH_1) \
	-Dxdmcp=$(call ptx/truefalse, PTXCONF_XORG_SERVER_EXT_XDMCP) \
	-Dxephyr=$(call ptx/truefalse, PTXCONF_XORG_SERVER_XEPHYR) \
	-Dxf86bigfont=$(call ptx/truefalse, PTXCONF_XORG_LIB_X11_XF86BIGFONT) \
	-Dxinerama=$(call ptx/truefalse, PTXCONF_XORG_SERVER_EXT_XINERAMA) \
	-Dxkb_bin_dir=/usr/bin \
	-Dxkb_dir=/usr/share/X11/xkb \
	-Dxnest=$(call ptx/truefalse, PTXCONF_XORG_SERVER_XNEST) \
	-Dxorg=$(call ptx/truefalse, PTXCONF_XORG_SERVER_XORG) \
	-Dxres=$(call ptx/truefalse, PTXCONF_XORG_SERVER_EXT_XRES) \
	-Dxv=$(call ptx/truefalse, PTXCONF_XORG_SERVER_EXT_XV) \
	-Dxvfb=$(call ptx/truefalse, PTXCONF_XORG_SERVER_XVFB) \
	-Dxvmc=$(call ptx/truefalse, PTXCONF_XORG_SERVER_EXT_XVMC) \
	-Dxwayland=$(call ptx/truefalse, PTXCONF_XORG_SERVER_XWAYLAND) \
	-Dxwayland_eglstream=false \
	-Dxwin=$(call ptx/truefalse, PTXCONF_XORG_SERVER_XWIN)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/xorg-server.targetinstall:
	@$(call targetinfo)

	@$(call install_init, xorg-server)
	@$(call install_fixup, xorg-server,PRIORITY,optional)
	@$(call install_fixup, xorg-server,SECTION,base)
	@$(call install_fixup, xorg-server,AUTHOR,"Robert Schwebel <r.schwebel@pengutronix.de>")
	@$(call install_fixup, xorg-server,DESCRIPTION,missing)

ifdef PTXCONF_XORG_SERVER_CONFIG_FILES_XORG_CONF
	@$(call install_alternative, xorg-server, 0, 0, 0644, \
		/etc/X11/xorg.conf)
endif

ifdef PTXCONF_PRELINK
	@$(call install_alternative, xorg-server, 0, 0, 0644, \
		/etc/prelink.conf.d/xorg)
endif

ifdef PTXCONF_XORG_SERVER_XVFB
	@$(call install_copy, xorg-server, 0, 0, 0755, -, \
		$(XORG_PREFIX)/bin/Xvfb)
endif
ifdef PTXCONF_XORG_SERVER_DMX
	@$(call install_copy, xorg-server, 0, 0, 0755, -, \
		$(XORG_PREFIX)/bin/Xdmx)
endif
ifdef PTXCONF_XORG_SERVER_XNEST
	@$(call install_copy, xorg-server, 0, 0, 0755, -, \
		$(XORG_PREFIX)/bin/Xnest)
endif
ifdef PTXCONF_XORG_SERVER_XWAYLAND
	@$(call install_copy, xorg-server, 0, 0, 0755, -, \
		$(XORG_PREFIX)/bin/Xwayland)
endif
ifdef PTXCONF_XORG_SERVER_XWIN
	@$(call install_copy, xorg-server, 0, 0, 0755, -, \
		$(XORG_PREFIX)/bin/Xwin)
endif
ifdef PTXCONF_XORG_SERVER_XORG
	@$(call install_copy, xorg-server, 0, 0, 0755, -, \
		$(XORG_PREFIX)/bin/Xorg)
	@$(call install_link, xorg-server, Xorg, /usr/bin/X)

ifdef PTXCONF_XORG_DRIVER_VIDEO
	@$(call install_copy, xorg-server, 0, 0, 0644, -, \
		$(XORG_PREFIX)/lib/xorg/modules/libexa.so)
#	@$(call install_copy, xorg-server, 0, 0, 0644, -, \
#		$(XORG_PREFIX)/lib/xorg/modules/libfb.so)
	@$(call install_copy, xorg-server, 0, 0, 0644, -, \
		$(XORG_PREFIX)/lib/xorg/modules/libfbdevhw.so)
	@$(call install_copy, xorg-server, 0, 0, 0644, -, \
		$(XORG_PREFIX)/lib/xorg/modules/libint10.so)
	@$(call install_copy, xorg-server, 0, 0, 0644, -, \
		$(XORG_PREFIX)/lib/xorg/modules/libshadow.so)
	@$(call install_copy, xorg-server, 0, 0, 0644, -, \
		$(XORG_PREFIX)/lib/xorg/modules/libshadowfb.so)
#	@$(call install_copy, xorg-server, 0, 0, 0644, -, \
#		$(XORG_PREFIX)/lib/xorg/modules/libvbe.so)
	@$(call install_copy, xorg-server, 0, 0, 0644, -, \
		$(XORG_PREFIX)/lib/xorg/modules/libwfb.so)
	@$(call install_copy, xorg-server, 0, 0, 0644, -, \
		$(XORG_PREFIX)/lib/xorg/modules/libvgahw.so)
ifdef PTXCONF_XORG_DRIVER_VIDEO_MODESETTING
	@$(call install_copy, xorg-server, 0, 0, 0644, -, \
		/usr/lib/xorg/modules/drivers/modesetting_drv.so)
endif
endif

ifdef PTXCONF_XORG_SERVER_EXT_GLX
	@$(call install_copy, xorg-server, 0, 0, 0644, -, \
		/usr/lib/xorg/modules/extensions/libglx.so)
endif
ifdef PTXCONF_XORG_SERVER_GLAMOR
	@$(call install_copy, xorg-server, 0, 0, 0644, -, \
		$(XORG_PREFIX)/lib/xorg/modules/libglamoregl.so)
endif

endif # PTXCONF_XORG_SERVER_XORG
	@$(call install_finish, xorg-server)

	mv $(PTXCONF_SYSROOT_TARGET)/usr/include/xorg/xf86Opt.h \
	$(PTXCONF_SYSROOT_TARGET)/usr/include/xorg/xf86Opt.h_org && \
	sed '/Bool bool/d' $(PTXCONF_SYSROOT_TARGET)/usr/include/xorg/xf86Opt.h_org > \
	$(PTXCONF_SYSROOT_TARGET)/usr/include/xorg/xf86Opt.h

	@$(call touch)

# vim: syntax=make
