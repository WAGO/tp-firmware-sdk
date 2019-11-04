# -*-makefile-*-
#
# Copyright (C) 2017 by <RG>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_XORG_DRIVER_VIDEO_ARMADA) += xorg-driver-video-armada

#
# Paths and names
#
XORG_DRIVER_VIDEO_ARMADA_VERSION	:= 0.0.3
XORG_DRIVER_VIDEO_ARMADA_SUFFIX		:= tar.bz2
XORG_DRIVER_VIDEO_ARMADA_MD5		:= 23b0e7c3fa6661d38d4f4ffa5fc682b7
XORG_DRIVER_VIDEO_ARMADA			:= xf86-driver-video-armada-$(XORG_DRIVER_VIDEO_ARMADA_VERSION)
XORG_DRIVER_VIDEO_ARMADA_SOURCE		:= $(SRCDIR)/$(XORG_DRIVER_VIDEO_ARMADA).$(XORG_DRIVER_VIDEO_ARMADA_SUFFIX)
XORG_DRIVER_VIDEO_ARMADA_URL		:= https://svsv01003/svn/repo14/$(XORG_DRIVER_VIDEO_ARMADA).$(XORG_DRIVER_VIDEO_ARMADA_SUFFIX)
XORG_DRIVER_VIDEO_ARMADA_DIR		:= $(BUILDDIR)/$(XORG_DRIVER_VIDEO_ARMADA)
XORG_DRIVER_VIDEO_ARMADA_BUILD_OOT	:= YES
XORG_DRIVER_VIDEO_ARMADA_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

ifdef PTXCONF_XORG_DRIVER_VIDEO_ARMADA_TRUNK
$(STATEDIR)/xorg-driver-video-armada.extract: $(STATEDIR)/autogen-tools
endif

$(STATEDIR)/xorg-driver-video-armada.extract: $(STATEDIR)/host-xorg-server.install.post
	@$(call targetinfo)
	@$(call clean, $(XORG_DRIVER_VIDEO_ARMADA_DIR))
	@$(call extract, XORG_DRIVER_VIDEO_ARMADA)
	cd $(XORG_DRIVER_VIDEO_ARMADA_DIR) &&  mkdir -p m4 && \
	autoreconf --install
	@$(call patchin, XORG_DRIVER_VIDEO_ARMADA)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#XORG_DRIVER_VIDEO_ARMADA_CONF_ENV	:= $(CROSS_ENV)

#
# autoconf
#
XORG_DRIVER_VIDEO_ARMADA_CONF_TOOL	:= autoconf
XORG_DRIVER_VIDEO_ARMADA_CONF_OPT	:= \
	$(CROSS_AUTOCONF_USR)

$(STATEDIR)/xorg-driver-video-armada.prepare:
	@$(call targetinfo)
#	@$(call world/prepare, XORG_DRIVER_VIDEO_ARMADA)
	cd $(XORG_DRIVER_VIDEO_ARMADA_DIR) && \
	export CC=$(CROSS_CC) && \
	export LD=$(CROSS_LD) && \
	./configure $(XORG_DRIVER_VIDEO_ARMADA_CONF_OPT) \
	--with-sysroot=$(PTXCONF_SYSROOT_TARGET) \
	--prefix=/usr \
	--disable-vivante \
	--with-etnaviv-lib=$(PTXCONF_SYSROOT_TARGET)/usr/lib \
	--with-etnaviv-include=$(PTXCONF_SYSROOT_TARGET)/usr/include/etnaviv \
	--with-xorg-module-dir=$(PTXCONF_SYSROOT_TARGET)/usr/lib/xorg/modules \
	XORG_CFLAGS="-I$(PTXCONF_SYSROOT_TARGET)/usr/include \
	-I$(PTXCONF_SYSROOT_TARGET)/usr/include/pixman-1 \
	-I$(PTXCONF_SYSROOT_TARGET)/usr/include/libdrm \
	-I$(PTXCONF_SYSROOT_TARGET)/usr/include/libdrm_armada \
	-I$(PTXCONF_SYSROOT_TARGET)/usr/include/xorg" \
	XORG_LIBS="-L$(PTXCONF_SYSROOT_TARGET)/usr/lib -lX11 -lXrand -lXv -lXrender" \
	DRMARMADA_CFLAGS="-I$(PTXCONF_SYSROOT_TARGET)/usr/include" \
	DRMARMADA_LIBS="-L$(PTXCONF_SYSROOT_TARGET)/usr/lib -ldrm_armada" \
	DRM_CFLAGS="-I$(PTXCONF_SYSROOT_TARGET)/usr/include/libdrm" \
	DRM_LIBS="-L$(PTXCONF_SYSROOT_TARGET)/usr/lib -ldrm" \
	PKG_CONFIG_PATH=$(PTXCONF_SYSROOT_TARGET)/usr/lib/pkgconfig && \
	make clean
	@$(call touch)


# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/xorg-driver-video-armada.compile:
	@$(call targetinfo)
	cd $(XORG_DRIVER_VIDEO_ARMADA_DIR) && make
#	@$(call world/compile, XORG_DRIVER_VIDEO_ARMADA)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/xorg-driver-video-armada.install:
	@$(call targetinfo)
#	@$(call world/install, XORG_DRIVER_VIDEO_ARMADA)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/xorg-driver-video-armada.targetinstall:
	@$(call targetinfo)

	@$(call install_init, xorg-driver-video-armada)
	@$(call install_fixup, xorg-driver-video-armada, PRIORITY, optional)
	@$(call install_fixup, xorg-driver-video-armada, SECTION, base)
	@$(call install_fixup, xorg-driver-video-armada, AUTHOR, "<RG>")
	@$(call install_fixup, xorg-driver-video-armada, DESCRIPTION, missing)

#	@$(call install_copy, xorg-driver-video-armada, 0, 0, 0644, $(XORG_DRIVER_VIDEO_ARMADA_DIR)/etnaviv/.libs/etnaviv_gpu.so, /usr/lib/xorg/modules/drivers/etnaviv_gpu.so)
#	@$(call install_copy, xorg-driver-video-armada, 0, 0, 0644, $(XORG_DRIVER_VIDEO_ARMADA_DIR)/etnaviv/etnaviv_gpu.la, /usr/lib/xorg/modules/drivers/etnaviv_gpu.la)
	@$(call install_copy, xorg-driver-video-armada, 0, 0, 0644, $(XORG_DRIVER_VIDEO_ARMADA_DIR)/etnaviv/.libs/etnadrm_gpu.so, /usr/lib/xorg/modules/drivers/etnadrm_gpu.so)
#	@$(call install_copy, xorg-driver-video-armada, 0, 0, 0644, $(XORG_DRIVER_VIDEO_ARMADA_DIR)/etnaviv/etnadrm_gpu.la, /usr/lib/xorg/modules/drivers/etnadrm_gpu.la)
	@$(call install_copy, xorg-driver-video-armada, 0, 0, 0644, $(XORG_DRIVER_VIDEO_ARMADA_DIR)/src/.libs/armada_drv.so, /usr/lib/xorg/modules/drivers/armada_drv.so)
#	@$(call install_copy, xorg-driver-video-armada, 0, 0, 0644, $(XORG_DRIVER_VIDEO_ARMADA_DIR)/src/.libs/armada_drv.la, /usr/lib/xorg/modules/drivers/armada_drv.la)
	@$(call install_finish, xorg-driver-video-armada)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/xorg-driver-video-armada.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, XORG_DRIVER_VIDEO_ARMADA)

# vim: syntax=make
