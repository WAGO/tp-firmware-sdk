# -*-makefile-*-
#
# Copyright (C) 2006 by Erwin Rol
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

# FIXME, we only need the source tree, do we still need the package ?


#
# We provide this package
#
PACKAGES-$(PTXCONF_MESALIB) += mesalib

#
# Paths and names
#
MESALIB_VERSION	:= 19.0.4
MESALIB_MD5	:= edd3055e863a44c4850f0db0d1412992
MESALIB		:= mesa-$(MESALIB_VERSION)
MESALIB_SUFFIX	:= tar.xz
MESALIB_URL	:= ftp://ftp.freedesktop.org/pub/mesa/$(MESALIB).$(MESALIB_SUFFIX)
MESALIB_SOURCE	:= $(SRCDIR)/$(MESALIB).$(MESALIB_SUFFIX)
MESALIB_DIR	:= $(BUILDDIR)/Mesa-$(MESALIB_VERSION)
MESALIB_LICENSE	:= MIT
MESALIB_LICENSE_FILES := \
	file://docs/license.html;md5=899fbe7e42d494c7c8c159c7001693d5

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

MESALIB_CONF_ENV := \
	$(CROSS_ENV) \
	ac_cv_prog_PYTHON2=$(PTXDIST_TOPDIR)/bin/python \
	LIBDRM_LIBS="-L${PTXDIST_SYSROOT_TARGET}/usr/lib -ldrm"

ifdef PTXCONF_ARCH_X86
MESALIB_DRI_DRIVERS-$(PTXCONF_MESALIB_DRI_I915)		+= i915
MESALIB_DRI_DRIVERS-$(PTXCONF_MESALIB_DRI_I965)		+= i965
endif
MESALIB_DRI_DRIVERS-$(PTXCONF_MESALIB_DRI_NOUVEAU_VIEUX)+= nouveau
MESALIB_DRI_DRIVERS-$(PTXCONF_MESALIB_DRI_R200)		+= r200

ifndef PTXCONF_ARCH_ARM # broken: https://bugs.freedesktop.org/show_bug.cgi?id=72064
ifndef PTXCONF_ARCH_X86 # needs llvm
MESALIB_GALLIUM_DRIVERS-$(PTXCONF_MESALIB_DRI_R300)	+= r300
endif
MESALIB_GALLIUM_DRIVERS-$(PTXCONF_MESALIB_DRI_R600)	+= r600
MESALIB_GALLIUM_DRIVERS-$(PTXCONF_MESALIB_DRI_RADEONSI)	+= radeonsi
endif

MESALIB_GALLIUM_DRIVERS-$(PTXCONF_MESALIB_DRI_NOUVEAU)	+= nouveau
MESALIB_GALLIUM_DRIVERS-$(PTXCONF_MESALIB_DRI_FREEDRENO)+= freedreno
ifdef PTXCONF_ARCH_ARM
MESALIB_GALLIUM_DRIVERS-$(PTXCONF_MESALIB_DRI_VC4)	+= vc4
endif

MESALIB_GALLIUM_DRIVERS-$(PTXCONF_MESALIB_DRI_SWRAST)	+= swrast

MESALIB_DRI_LIBS-y += \
	$(subst nouveau,nouveau_vieux,$(MESALIB_DRI_DRIVERS-y)) \
	$(subst freedreno,kgsl,$(MESALIB_GALLIUM_DRIVERS-y))

MESALIB_LIBS-y				:= libglapi
MESALIB_LIBS-$(PTXCONF_MESALIB_GLX)	+= libGL
MESALIB_LIBS-$(PTXCONF_MESALIB_GLES1)	+= libGLESv1_CM
MESALIB_LIBS-$(PTXCONF_MESALIB_GLES2)	+= libGLESv2
MESALIB_LIBS-$(PTXCONF_MESALIB_EGL)	+= libEGL
MESALIB_LIBS-$(PTXCONF_MESALIB_GBM)	+= libgbm

MESALIBS_EGL_PLATFORMS-$(PTXCONF_MESALIB_EGL_X11)	+= x11
MESALIBS_EGL_PLATFORMS-$(PTXCONF_MESALIB_EGL_DRM)	+= drm
MESALIBS_EGL_PLATFORMS-$(PTXCONF_MESALIB_EGL_WAYLAND)	+= wayland

MESALIB_LIBS-$(PTXCONF_MESALIB_EGL_WAYLAND)	+= libwayland-egl

MESALIB_BUILD_OOT	:= YES
MESALIB_CONF_TOOL	:= autoconf
MESALIB_CONF_OPT	:= \
	$(CROSS_AUTOCONF_USR) \
	--$(call ptx/endis, PTXCONF_GLOBAL_LARGE_FILE)-largefile \
	--disable-static \
	--enable-shared \
	--disable-debug \
	--disable-mangling \
	--disable-asm \
	--disable-selinux \
	--$(call ptx/endis, PTXCONF_MESALIB_OPENGL)-opengl \
	--$(call ptx/endis, PTXCONF_MESALIB_GLES1)-gles1 \
	--$(call ptx/endis, PTXCONF_MESALIB_GLES2)-gles2 \
	--enable-dri \
	--disable-gallium-extra-hud \
	--disable-lmsensors \
	--enable-dri3 \
	--$(call ptx/endis, PTXCONF_MESALIB_GLX)-glx \
	--enable-osmesa \
	--disable-gallium-osmesa \
	--$(call ptx/endis, PTXCONF_MESALIB_EGL)-egl \
	--disable-xa \
	--$(call ptx/endis, PTXCONF_MESALIB_GBM)-gbm \
	--disable-nine \
	--disable-xvmc \
	--disable-vdpau \
	--disable-va \
	--disable-omx-bellagio \
	--disable-opencl \
	--disable-opencl-icd \
	--disable-gallium-tests \
	--enable-shared-glapi \
	--disable-glx-read-only-text \
	--enable-driglx-direct \
	--enable-glx-tls \
	--disable-llvm \
	--enable-llvm-shared-libs \
	--disable-libglvnd \
	--with-dri-driverdir=/usr/lib/dri \
	--without-vulkan-drivers \
	--enable-autotools \
	--with-platforms="$(MESALIBS_EGL_PLATFORMS-y)"

ifdef PTXCONF_MESALIB_GALLIUM_DRIVERS
#set environment variables ETNAVIV_CFLAGS ETNAVIV_LIBS to avoid the need to call pkg-config.
MESALIB_CONF_OPT += --with-gallium-drivers=etnaviv,swrast
endif

ifdef PTXCONF_MESALIB_DRI_SWRAST
MESALIB_CONF_OPT += --with-dri-drivers=swrast
endif

#
#   --with-dri-drivers=$(subst $(space),$(comma),$(MESALIB_DRI_DRIVERS-y)) \
#	--with-gallium-drivers=$(subst $(space),$(comma),$(MESALIB_GALLIUM_DRIVERS-y)) \
# 	--with-sha1= \
#	--disable-shader-cache \

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/mesalib.compile:
	@$(call targetinfo)
	cp $(PTXCONF_SYSROOT_HOST)/bin/mesa/glsl_compiler $(MESALIB_DIR)/src/compiler/
	@$(call world/compile, MESALIB)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/mesalib.targetinstall:
	@$(call targetinfo)

	@$(call install_init, mesalib)
	@$(call install_fixup, mesalib,PRIORITY,optional)
	@$(call install_fixup, mesalib,SECTION,base)
	@$(call install_fixup, mesalib,AUTHOR,"Robert Schwebel <r.schwebel@pengutronix.de>")
	@$(call install_fixup, mesalib,DESCRIPTION,missing)

	@$(foreach lib, $(MESALIB_DRI_LIBS-y), \
		$(call install_copy, mesalib, 0, 0, 0644, -, \
		/usr/lib/dri/$(lib)_dri.so);)

	@$(foreach lib, $(MESALIB_LIBS-y), \
		$(call install_lib, mesalib, 0, 0, 0644, $(lib));)
ifdef PTXCONF_MESALIB_GALLIUM_DRIVERS
	@$(call install_copy, mesalib, 0, 0, 0644, $(MESALIB_DIR)-build/lib/gallium/etnaviv_dri.so, /usr/lib/dri/etnaviv_dri.so)
#	@$(call install_copy, mesalib, 0, 0, 0644, $(MESALIB_DIR)-build/lib/gallium/imx-drm_dri.so, /usr/lib/dri/imx-drm_dri.so)
#	@$(call install_copy, mesalib, 0, 0, 0644, $(MESALIB_DIR)-build/lib/gallium/kgsl_dri.so, /usr/lib/dri/kgsl_dri.so)
#	@$(call install_copy, mesalib, 0, 0, 0644, $(MESALIB_DIR)-build/lib/gallium/kms_swrast_dri.so, /usr/lib/dri/kms_swrast_dri.so)
#	@$(call install_copy, mesalib, 0, 0, 0644, $(MESALIB_DIR)-build/lib/gallium/msm_dri.so, /usr/lib/dri/msm_dri.so)
	@$(call install_copy, mesalib, 0, 0, 0644, $(MESALIB_DIR)-build/lib/gallium/swrast_dri.so, /usr/lib/dri/swrast_dri.so)
endif
	@$(call install_copy, mesalib, 0, 0, 0644, $(MESALIB_DIR)-build/lib/mesa_dri_drivers.so, /usr/lib/dri/mesa_dri_drivers.so)
#	@$(call install_copy, mesalib, 0, 0, 0644, $(MESALIB_DIR)-build/lib/mesa_dri_drivers.so, /usr/lib/mesa_dri_drivers.so)
#	@$(call install_copy, mesalib, 0, 0, 0644, $(MESALIB_DIR)-build/lib/swrast_dri.so, /usr/lib/swrast_dri.so)
#	@$(call install_copy, mesalib, 0, 0, 0644, $(MESALIB_DIR)/src/util/drirc, /etc/drirc)
#	@$(call install_copy, mesalib, 0, 0, 0644, $(MESALIB_DIR)/src/util/drirc, /root/.drirc)
	@$(call install_finish, mesalib)

	@$(call touch)


# vim: syntax=make
