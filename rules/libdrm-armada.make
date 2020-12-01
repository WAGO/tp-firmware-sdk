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
PACKAGES-$(PTXCONF_LIBDRM_ARMADA) += libdrm-armada

#
# Paths and names
#
LIBDRM_ARMADA_VERSION	:= 1.0
LIBDRM_ARMADA_MD5		:= c9e7bb974295f482c498c4ebf8262525
LIBDRM_ARMADA			:= libdrm-armada-$(LIBDRM_ARMADA_VERSION)
LIBDRM_ARMADA-SUFFIX	:= tar.bz2
LIBDRM_ARMADA_URL		:= https://svsv01003/svn/repo14/$(LIBDRM_ARMADA).$(LIBDRM_ARMADA-SUFFIX)
LIBDRM_ARMADA_SOURCE	:= $(SRCDIR)/$(LIBDRM_ARMADA).$(LIBDRM_ARMADA-SUFFIX)
LIBDRM_ARMADA_DIR		:= $(BUILDDIR)/$(LIBDRM_ARMADA)
LIBDRM_ARMADA_BUILD_OOT	:= YES
LIBDRM_ARMADA_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/libdrm-armada.extract: $(STATEDIR)/autogen-tools
	@$(call targetinfo)
	@$(call clean, $(LIBDRM_ARMADA_DIR))
	@$(call extract, LIBDRM_ARMADA)
	cd $(LIBDRM_ARMADA_DIR) && autoreconf -f -i
	@$(call patchin, LIBDRM_ARMADA)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

LIBDRM_ARMADA_CONF_ENV	:= $(CROSS_ENV)
LIBDRM_ARMADA_CONF_OPT	:= $(CROSS_AUTOCONF_USR)

$(STATEDIR)/libdrm-armada.prepare:
	@$(call targetinfo)
	cd $(LIBDRM_ARMADA_DIR) && \
	PKG_CONFIG_LIBDIR=$(PTXCONF_SYSROOT_TARGET)/usr/lib \
	PKG_CONFIG_PATH=$(PTXCONF_SYSROOT_TARGET)/usr/lib \
	CC=$(CROSS_CC) ./configure --prefix=/usr/lib --host=arm-linux \
	--with-sysroot=$(PTXCONF_SYSROOT_TARGET) \
	CFLAGS="-I$(PTXCONF_SYSROOT_TARGET)/usr/include  -I$(PTXCONF_SYSROOT_TARGET)/usr/include/libdrm" \
	LDFLAGS="-L$(PTXCONF_SYSROOT_TARGET)/usr/lib" \
	LIBDRM_CFLAGS=-I$(PTXCONF_SYSROOT_TARGET)/usr/include \
	LIBDRM_LIBS="-L$(PTXCONF_SYSROOT_TARGET)/usr/lib -ldrm" && \
	PKG_CONFIG_PATH=$(PTXCONF_SYSROOT_TARGET)/usr/lib/pkgconfig && \
	make clean
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/libdrm-armada.compile:
	@$(call targetinfo)
	cd $(LIBDRM_ARMADA_DIR) && make
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/libdrm-armada.install:
	@$(call targetinfo)
	@$(call targetinfo)
	cd $(LIBDRM_ARMADA_DIR) && \
	cp -f libdrm_armada.la $(PTXCONF_SYSROOT_TARGET)/usr/lib && \
	cp -f libdrm_armada.pc $(PTXCONF_SYSROOT_TARGET)/usr/lib/pkgconfig
	cd $(LIBDRM_ARMADA_DIR)/.libs && \
	cp -f libdrm_armada.so.0.2.0 $(PTXCONF_SYSROOT_TARGET)/usr/lib && \
	cd $(PTXCONF_SYSROOT_TARGET)/usr/lib && \
	ln -sf libdrm_armada.so.0.2.0 libdrm_armada.so && \
	ln -sf libdrm_armada.so.0.2.0 libdrm_armada.so.0 && \
	mkdir -p $(PTXCONF_SYSROOT_TARGET)/usr/include/libdrm_armada && \
	cp -f $(LIBDRM_ARMADA_DIR)/armada_bufmgr.h $(PTXCONF_SYSROOT_TARGET)/usr/include/libdrm_armada && \
	cp -f $(LIBDRM_ARMADA_DIR)/armada_ioctl.h $(PTXCONF_SYSROOT_TARGET)/usr/include/libdrm_armada && \
	cp -f $(LIBDRM_ARMADA_DIR)/libdrm_lists.h $(PTXCONF_SYSROOT_TARGET)/usr/include/libdrm_armada
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/libdrm-armada.targetinstall:
	@$(call targetinfo)

	@$(call install_init, libdrm-armada)
	@$(call install_fixup, libdrm-armada, PRIORITY, optional)
	@$(call install_fixup, libdrm-armada, SECTION, base)
	@$(call install_fixup, libdrm-armada, AUTHOR, "<RG>")
	@$(call install_fixup, libdrm-armada, DESCRIPTION, missing)

	@$(call install_copy, libdrm-armada, 0, 0, 0644, $(LIBDRM_ARMADA_DIR)/.libs/libdrm_armada.so.0.2.0, /usr/lib/libdrm_armada.so.0.2.0)
	@$(call install_link, libdrm-armada, libdrm_armada.so.0.2.0, /usr/lib/libdrm_armada.so.0)
	@$(call install_link, libdrm-armada, libdrm_armada.so.0.2.0, /usr/lib/libdrm_armada.so)

	@$(call install_finish, libdrm-armada)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/libdrm-armada.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, LIBDRM_ARMADA)

# vim: syntax=make
