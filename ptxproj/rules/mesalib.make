# -*-makefile-*-
#
# Copyright (C) 2006 by Erwin Rol
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_MESALIB) += mesalib

#
# Paths and names
#
MESALIB_VERSION	:= 24.0.0
MESALIB_MD5	:= 367c6b186780326de6e4ad6aacd23ae8
MESALIB		:= mesa-$(MESALIB_VERSION)
MESALIB_SUFFIX	:= tar.xz
MESALIB_URL	:= \
	https://mesa.freedesktop.org/archive/$(MESALIB).$(MESALIB_SUFFIX)
MESALIB_SOURCE	:= $(SRCDIR)/$(MESALIB).$(MESALIB_SUFFIX)
MESALIB_DIR	:= $(BUILDDIR)/Mesa-$(MESALIB_VERSION)
MESALIB_LICENSE	:= MIT
MESALIB_LICENSE_FILES := \
	file://docs/license.rst;md5=63779ec98d78d823a9dc533a0735ef10

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

MESALIB_GALLIUM_DRIVERS-$(PTXCONF_MESALIB_DRI_VIRGL)	+= virgl
ifndef PTXCONF_ARCH_ARM # broken: https://gitlab.freedesktop.org/mesa/mesa/-/issues/473
ifndef PTXCONF_ARCH_X86 # needs llvm
MESALIB_GALLIUM_DRIVERS-$(PTXCONF_MESALIB_DRI_R300)	+= r300
endif
MESALIB_GALLIUM_DRIVERS-$(PTXCONF_MESALIB_DRI_R600)	+= r600
MESALIB_GALLIUM_DRIVERS-$(PTXCONF_MESALIB_DRI_RADEONSI)	+= radeonsi
endif
MESALIB_GALLIUM_DRIVERS-$(PTXCONF_MESALIB_DRI_NOUVEAU)	+= nouveau
MESALIB_GALLIUM_DRIVERS-$(PTXCONF_MESALIB_DRI_FREEDRENO)+= freedreno
MESALIB_GALLIUM_DRIVERS-$(PTXCONF_MESALIB_DRI_ETNAVIV)	+= etnaviv
ifdef PTXCONF_ARCH_ARM_NEON
MESALIB_GALLIUM_DRIVERS-$(PTXCONF_MESALIB_DRI_V3D)	+= v3d
MESALIB_GALLIUM_DRIVERS-$(PTXCONF_MESALIB_DRI_VC4)	+= vc4
endif
MESALIB_GALLIUM_DRIVERS-$(PTXCONF_MESALIB_DRI_SWRAST)	+= swrast
MESALIB_GALLIUM_DRIVERS-$(PTXCONF_MESALIB_DRI_PANFROST)	+= panfrost
MESALIB_GALLIUM_DRIVERS-$(PTXCONF_MESALIB_DRI_LIMA)	+= lima
ifdef PTXCONF_ARCH_X86
MESALIB_GALLIUM_DRIVERS-$(PTXCONF_MESALIB_DRI_IRIS)	+= iris
endif
MESALIB_GALLIUM_DRIVERS-$(PTXCONF_MESALIB_DRI_ZINK)	+= zink
MESALIB_GALLIUM_DRIVERS-$(PTXCONF_MESALIB_DRI_ASAHI)	+= asahi
ifdef PTXCONF_ARCH_X86
MESALIB_GALLIUM_DRIVERS-$(PTXCONF_MESALIB_DRI_CROCUS)	+= crocus
MESALIB_GALLIUM_DRIVERS-$(PTXCONF_MESALIB_DRI_SVGA)	+= svga
endif

MESALIB_DRI_GALLIUM_LIBS-y = \
	$(call ptx/ifdef, PTXCONF_MESALIB_DRI_KMSRO, \
		armada-drm \
		exynos \
		hx8357d \
		ili9225 \
		ili9341 \
		imx-dcss \
		imx-drm \
		imx-lcdif \
		ingenic-drm \
		kirin \
		komeda \
		mali-dp \
		mcde \
		mediatek \
		meson \
		mi0283qt \
		mxsfb-drm \
		pl111 \
		rcar-du \
		repaper \
		rockchip \
		st7586 \
		st7735r \
		stm \
		sun4i-drm) \
	$(subst swrast,swrast kms_swrast \
	,$(subst freedreno,kgsl msm \
	,$(subst svga,vmwgfx \
	,$(subst virgl,virtio_gpu \
	,$(MESALIB_GALLIUM_DRIVERS-y) \
	))))

MESALIB_VIDEO_CODECS-$(PTXCONF_MESALIB_VIDEO_VC1DEC)	+= vc1dec
MESALIB_VIDEO_CODECS-$(PTXCONF_MESALIB_VIDEO_H264DEC)	+= h264dec
MESALIB_VIDEO_CODECS-$(PTXCONF_MESALIB_VIDEO_H264ENC)	+= h264enc
MESALIB_VIDEO_CODECS-$(PTXCONF_MESALIB_VIDEO_H265DEC)	+= h265dec
MESALIB_VIDEO_CODECS-$(PTXCONF_MESALIB_VIDEO_H265ENC)	+= h265enc

ifdef PTXCONF_ARCH_X86
MESALIB_VULKAN_DRIVERS-$(PTXCONF_MESALIB_VULKAN_AMD)		+= amd
MESALIB_VULKAN_DRIVERS-$(PTXCONF_MESALIB_VULKAN_INTEL)		+= intel
MESALIB_VULKAN_DRIVERS-$(PTXCONF_MESALIB_VULKAN_INTEL_HASVK)	+= intel_hasvk
endif
ifdef PTXCONF_ARCH_ARM_NEON
MESALIB_VULKAN_DRIVERS-$(PTXCONF_MESALIB_VULKAN_BROADCOM)	+= broadcom
endif
ifdef PTXCONF_ARCH_ARM
MESALIB_VULKAN_DRIVERS-$(PTXCONF_MESALIB_VULKAN_FREEDRENO)	+= freedreno
MESALIB_VULKAN_DRIVERS-$(PTXCONF_MESALIB_VULKAN_PANFROST)	+= panfrost
endif
MESALIB_VULKAN_DRIVERS-$(PTXCONF_MESALIB_VULKAN_SWRAST)		+= swrast

MESALIB_VULKAN_LIBS-y = $(subst amd,radeon,$(subst swrast,lvp,$(MESALIB_VULKAN_DRIVERS-y)))

MESALIB_VULKAN_LAYERS-$(PTXCONF_MESALIB_VULKAN_DEVICE_SELECT)	+= device-select
MESALIB_VULKAN_LAYERS-$(PTXCONF_MESALIB_VULKAN_INTEL_NULLHW)	+= intel-nullhw
MESALIB_VULKAN_LAYERS-$(PTXCONF_MESALIB_VULKAN_OVERLAY)		+= overlay

MESALIB_LIBS-y				:= libglapi
MESALIB_LIBS-$(PTXCONF_MESALIB_GLX)	+= libGL
MESALIB_LIBS-$(PTXCONF_MESALIB_GLES1)	+= libGLESv1_CM
MESALIB_LIBS-$(PTXCONF_MESALIB_GLES2)	+= libGLESv2
MESALIB_LIBS-$(PTXCONF_MESALIB_EGL)	+= libEGL
MESALIB_LIBS-$(PTXCONF_MESALIB_GBM)	+= libgbm

MESALIBS_EGL_PLATFORMS-$(PTXCONF_MESALIB_EGL_WAYLAND)	+= wayland
MESALIBS_EGL_PLATFORMS-$(PTXCONF_MESALIB_EGL_X11)	+= x11

ifdef PTXCONF_MESALIB_VA
ifndef PTXCONF_ARCH_ARM # broken: https://gitlab.freedesktop.org/mesa/mesa/-/issues/473
MESALIB_DRI_VA_LIBS-$(PTXCONF_MESALIB_DRI_R600)		+= r600
MESALIB_DRI_VA_LIBS-$(PTXCONF_MESALIB_DRI_RADEONSI)	+= radeonsi
endif
MESALIB_DRI_VA_LIBS-$(PTXCONF_MESALIB_DRI_NOUVEAU)	+= nouveau
endif

MESALIB_CONF_TOOL	:= meson
MESALIB_CONF_OPT	:= \
	$(CROSS_MESON_USR) \
	-Dallow-kcmp=enabled \
	-Dandroid-libbacktrace=disabled \
	-Dandroid-stub=false \
	-Dbuild-aco-tests=false \
	-Dbuild-tests=false \
	-Dcustom-shader-replacement= \
	-Dd3d-drivers-path=/usr/lib/d3d \
	-Ddatasources=auto \
	-Ddraw-use-llvm=true \
	-Ddri-drivers-path=/usr/lib/dri \
	-Ddri-search-path=/usr/lib/dri \
	-Ddri3=$(call ptx/endis, PTXCONF_MESALIB_DRI3)d \
	-Degl=$(call ptx/endis, PTXCONF_MESALIB_EGL)d \
	-Degl-lib-suffix= \
	-Degl-native-platform=auto \
	-Denable-glcpp-tests=false \
	-Dexecmem=true \
	-Dexpat=enabled \
	-Dfreedreno-kmds=msm \
	-Dgallium-d3d10umd=false \
	-Dgallium-d3d12-video=disabled \
	-Dgallium-drivers=$(subst $(space),$(comma),$(MESALIB_GALLIUM_DRIVERS-y)) \
	-Dgallium-extra-hud=$(call ptx/truefalse, PTXCONF_MESALIB_EXTENDED_HUD) \
	-Dgallium-nine=false \
	-Dgallium-omx=disabled \
	-Dgallium-opencl=disabled \
	-Dgallium-rusticl=false \
	-Dgallium-va=$(call ptx/endis, PTXCONF_MESALIB_VA)d \
	-Dgallium-vdpau=disabled \
	-Dgallium-windows-dll-name=libgallium_wgl \
	-Dgallium-xa=disabled \
	-Dgbm=$(call ptx/endis, PTXCONF_MESALIB_GBM)d \
	-Dgbm-backends-path= \
	-Dgles-lib-suffix= \
	-Dgles1=$(call ptx/endis, PTXCONF_MESALIB_GLES1)d \
	-Dgles2=$(call ptx/endis, PTXCONF_MESALIB_GLES2)d \
	-Dglvnd=false \
	-Dglvnd-vendor-name= \
	-Dglx=$(call ptx/ifdef, PTXCONF_MESALIB_GLX, dri, disabled) \
	-Dglx-direct=true \
	-Dglx-read-only-text=false \
	-Dimagination-srv=false \
	-Dinstall-intel-gpu-tests=false \
	-Dintel-clc=disabled \
	-Dintel-xe-kmd=disabled \
	-Dlibunwind=disabled \
	-Dllvm=disabled \
	-Dlmsensors=$(call ptx/endis, PTXCONF_MESALIB_LMSENSORS)d \
	-Dmicrosoft-clc=disabled \
	-Dmin-windows-version=8 \
	-Dmoltenvk-dir= \
	-Domx-libs-path=/usr/lib/dri \
	-Dopencl-spirv=false \
	-Dopengl=$(call ptx/truefalse, PTXCONF_MESALIB_OPENGL) \
	-Dosmesa=false \
	-Dperfetto=false \
	-Dplatform-sdk-version=25 \
	-Dplatforms=$(subst $(space),$(comma),$(MESALIBS_EGL_PLATFORMS-y)) \
	-Dpower8=disabled \
	-Dradv-build-id='' \
	-Dselinux=false \
	-Dshader-cache=$(call ptx/endis, PTXCONF_MESALIB_SHADER_CACHE)d \
	-Dshader-cache-default=true \
	-Dshader-cache-max-size=1G \
	-Dshared-glapi=enabled \
	-Dshared-llvm=disabled \
	-Dspirv-to-dxil=false \
	-Dsse2=true \
	-Dstatic-libclc=[] \
	-Dtools=[] \
	-Dva-libs-path=/usr/lib/dri \
	-Dvalgrind=disabled \
	-Dvdpau-libs-path=/usr/lib/vdpau \
	-Dvideo-codecs=$(subst $(space),$(comma),$(MESALIB_VIDEO_CODECS-y)) \
	-Dvmware-mks-stats=false \
	-Dvulkan-beta=false \
	-Dvulkan-drivers=$(subst $(space),$(comma),$(MESALIB_VULKAN_DRIVERS-y)) \
	-Dvulkan-icd-dir=/etc/vulkan/icd.d \
	-Dvulkan-layers=$(subst $(space),$(comma),$(MESALIB_VULKAN_LAYERS-y)) \
	-Dxlib-lease=$(call ptx/endis, PTXCONF_MESALIB_EGL_X11)d \
	-Dxmlconfig=$(call ptx/endis, PTXCONF_MESALIB_XMLCONFIG)d \
	-Dzlib=enabled \
	-Dzstd=$(call ptx/endis, PTXCONF_MESALIB_SHADER_CACHE)d

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

ifdef PTXCONF_ARCH_ARM
ifndef PTXCONF_ARCH_ARM_NEON
# don't try to build NEON code on platforms that don't have NEON
MESALIB_CFLAGS := -DNO_FORMAT_ASM
endif
endif

$(STATEDIR)/mesalib.compile:
	@$(call targetinfo)
	cp $(PTXDIST_SYSROOT_HOST)/usr/bin/mesa/glsl_compiler $(MESALIB_DIR)/src/compiler/
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

ifneq ($(strip $(MESALIB_DRI_GALLIUM_LIBS-y)),)
	@$(call install_copy, mesalib, 0, 0, 0644, \
		$(MESALIB_PKGDIR)/usr/lib/dri/$(firstword $(MESALIB_DRI_GALLIUM_LIBS-y))_dri.so, \
		/usr/lib/dri/gallium_dri.so)

	@$(foreach lib, $(MESALIB_DRI_GALLIUM_LIBS-y), \
		test -f $(MESALIB_PKGDIR)/usr/lib/dri/$(lib)_dri.so || \
			ptxd_bailout "missing gallium driver $(lib)_dri.so"$(ptx/nl) \
		$(call install_link, mesalib, gallium_dri.so, \
		/usr/lib/dri/$(lib)_dri.so)$(ptx/nl))
endif
ifneq ($(strip $(MESALIB_DRI_VA_LIBS-y)),)
	@$(call install_copy, mesalib, 0, 0, 0644, \
		$(MESALIB_PKGDIR)/usr/lib/dri/$(firstword $(MESALIB_DRI_VA_LIBS-y))_drv_video.so, \
		/usr/lib/dri/va_dri.so)

	@$(foreach lib, $(MESALIB_DRI_VA_LIBS-y), \
		test -f $(MESALIB_PKGDIR)/usr/lib/dri/$(lib)_drv_video.so || \
			ptxd_bailout "missing va driver $(lib)_drv_video.so"$(ptx/nl) \
		$(call install_link, mesalib, va_dri.so, \
		/usr/lib/dri/$(lib)_drv_video.so)$(ptx/nl))
endif

ifneq ($(strip $(MESALIB_VULKAN_LIBS-y)),)
	@$(foreach lib, $(MESALIB_VULKAN_LIBS-y), \
		$(call install_copy, mesalib, 0, 0, 0644, -, \
		/usr/lib/libvulkan_$(lib).so)$(ptx/nl) \
		$(call install_glob, mesalib, 0, 0, -, \
		/etc/vulkan/icd.d, */$(lib)_icd.*.json)$(ptx/nl))
endif

	@$(foreach lib, $(MESALIB_LIBS-y), \
		$(call install_lib, mesalib, 0, 0, 0644, $(lib))$(ptx/nl))

	@$(call install_finish, mesalib)

	@$(call touch)


# vim: syntax=make
