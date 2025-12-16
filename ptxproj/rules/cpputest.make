# -*-makefile-*-
#
# Copyright (C) 2010 by WAGO
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_CPPUTEST) += cpputest

#
# Paths and names
#
CPPUTEST_VERSION	:= 3.8
CPPUTEST_MD5		:= fc34eab33c08e8dcd83698988e9a33c5 e8fdbbb5dd37d32d65919f240f984905
CPPUTEST		:= cpputest-$(CPPUTEST_VERSION)
CPPUTEST_SUFFIX		:= tar.gz
CPPUTEST_URL		:= https://github.com/cpputest/cpputest/releases/download/v$(CPPUTEST_VERSION)/$(CPPUTEST).$(CPPUTEST_SUFFIX)
CPPUTEST_SOURCE		:= $(SRCDIR)/$(CPPUTEST).$(CPPUTEST_SUFFIX)
CPPUTEST_DIR		:= $(BUILDDIR)/$(CPPUTEST)
CPPUTEST_LICENSE	:= BSD-3-Clause
CPPUTEST_LICENSE_FILES	:= file://COPYING;md5=ce5d5f1fe02bcd1343ced64a06fd4177

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#
# autoconf
#
CPPUTEST_CONF_TOOL	:= autoconf
CPPUTEST_CONF_OPT	:=  \
	$(CROSS_AUTOCONF_USR) \
	--enable-std-c \
	--enable-std-cpp \
	--enable-std-cpp11 \
	--enable-cpputest-flags \
	--enable-memory-leak-detection \
	--enable-extensions \
	--enable-longlong \
	--enable-generate-map-file \
	--enable-coverage

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

# $(STATEDIR)/cpputest.prepare:
#	@$(call targetinfo)	
#	@$(call world/prepare, CPPUTEST)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/cpputest.compile:
#	@$(call targetinfo)
#	@$(call world/compile, CPPUTEST)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/cpputest.install:
#	@$(call targetinfo)
#	@$(call world/install, CPPUTEST)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/cpputest.targetinstall:
	@$(call targetinfo)

# Packet contains no artifcats.

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

cpputest_clean:
	rm -rf $(STATEDIR)/cpputest.*
	rm -rf $(PKGDIR)/cpputest_*
	rm -rf $(CPPUTEST_DIR)

# vim: syntax=make
