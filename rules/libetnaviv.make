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
PACKAGES-$(PTXCONF_LIBETNAVIV) += libetnaviv

#
# Paths and names
#
LIBETNAVIV_VERSION	:= 1.0
LIBETNAVIV_MD5		:= 3bbccbb1292944f5f5fc4afe9b573315
LIBETNAVIV			:= libetnaviv-$(LIBETNAVIV_VERSION)
LIBETNAVIV_SUFFIX	:= tar.bz2
LIBETNAVIV_URL		:= https://svsv01003/svn/repo14/$(LIBETNAVIV).$(LIBETNAVIV_SUFFIX)
LIBETNAVIV_SOURCE	:= $(SRCDIR)/$(LIBETNAVIV).$(LIBETNAVIV_SUFFIX)
LIBETNAVIV_DIR		:= $(BUILDDIR)/$(LIBETNAVIV)
LIBETNAVIV_BUILD_OOT	:= YES
LIBETNAVIV_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

ifdef PTXCONF_LIBETNAVIV_TRUNK
$(STATEDIR)/libetnaviv.extract: $(STATEDIR)/autogen-tools
endif

$(STATEDIR)/libetnaviv.extract: $(STATEDIR)/autogen-tools
	@$(call targetinfo)
	@$(call clean, $(LIBETNAVIV_DIR))
	@$(call extract, LIBETNAVIV)
	cd $(LIBETNAVIV_DIR) && autoreconf --force --install --verbose && \
	mkdir -p $(PTXCONF_SYSROOT_TARGET)/usr/include/etnaviv && \
	cp -f $(LIBETNAVIV_DIR)/src/*.h $(PTXCONF_SYSROOT_TARGET)/usr/include/etnaviv
	@$(call patchin, LIBETNAVIV)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

LIBETNAVIV_CONF_ENV	:= $(CROSS_ENV)
LIBETNAVIV_CONF_OPT	:= $(CROSS_AUTOCONF_USR)

$(STATEDIR)/libetnaviv.prepare:
	@$(call targetinfo)
	cd $(LIBETNAVIV_DIR) && \
	export CC=$(CROSS_CC) && \
	./configure --prefix=/usr --host=arm-linux \
	CFLAGS="-I$(PTXCONF_SYSROOT_TARGET)/usr/include  -I$(BUILDDIR)/galcore_headers/include_imx6_v4_6_9" \
	LDFLAGS="-L$(PTXCONF_SYSROOT_TARGET)/usr/lib" && \
	PKG_CONFIG_PATH=$(PTXCONF_SYSROOT_TARGET)/usr/lib/pkgconfig && \
	make clean
#	@$(call world/prepare, LIBETNAVIV)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/libetnaviv.compile:
	@$(call targetinfo)
	cd $(LIBETNAVIV_DIR) && make
#	@$(call world/compile, LIBETNAVIV)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/libetnaviv.install:
	@$(call targetinfo)
	cd $(LIBETNAVIV_DIR)/src/.libs && \
	cp -f libetnaviv.la $(PTXCONF_SYSROOT_TARGET)/usr/lib && \
	cp -f libetnaviv.so.1.0.0 $(PTXCONF_SYSROOT_TARGET)/usr/lib && \
	cd $(PTXCONF_SYSROOT_TARGET)/usr/lib && \
	ln -sf libetnaviv.so.1.0.0 libetnaviv.so && \
	ln -sf libetnaviv.so.1.0.0 libetnaviv.so.1 && \
	cd $(LIBETNAVIV_DIR) && \
	cp -f libetnaviv.pc $(PTXCONF_SYSROOT_TARGET)/usr/lib/pkgconfig
#	@$(call world/install, LIBETNAVIV)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/libetnaviv.targetinstall:
	@$(call targetinfo)

	@$(call install_init, libetnaviv)
	@$(call install_fixup, libetnaviv, PRIORITY, optional)
	@$(call install_fixup, libetnaviv, SECTION, base)
	@$(call install_fixup, libetnaviv, AUTHOR, "<RG>")
	@$(call install_fixup, libetnaviv, DESCRIPTION, missing)

	@$(call install_copy, libetnaviv, 0, 0, 0644, $(LIBETNAVIV_DIR)/src/.libs/libetnaviv.so.1.0.0, /usr/lib/libetnaviv.so.1.0.0)
	@$(call install_link, libetnaviv, libetnaviv.so.1.0.0, /usr/lib/libetnaviv.so.0)
	@$(call install_link, libetnaviv, libetnaviv.so.1.0.0, /usr/lib/libetnaviv.so)

	@$(call install_finish, libetnaviv)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/libetnaviv.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, LIBETNAVIV)

# vim: syntax=make
