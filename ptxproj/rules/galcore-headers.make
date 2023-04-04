# -*-makefile-*-
#
# Copyright (C) 2014 by <>WAGO GmbH @AUTHOR@ Co KG
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_GALCORE_HEADERS) += galcore_headers

#
# Paths and names
#
GALCORE_HEADERS_VERSION	:= 1.0.0
GALCORE_HEADERS		:= galcore_headers
GALCORE_HEADERS_SUFFIX	:= tar.bz2
#GALCORE_HEADERS_MD5	= $(shell wget --timeout=1 -q -O - "$$@" http://svsv01003.wago.local/wbm/$(GALCORE_HEADERS).$(GALCORE_HEADERS_SUFFIX).md5 | sed -e 's/*$(GALCORE_HEADERS).$(GALCORE_HEADERS_SUFFIX)//g')
GALCORE_HEADERS_MD5	:=  deabb1cb6fff5961be4b1d454a053930
GALCORE_HEADERS_URL	:= http://svsv01003.wago.local/$(GALCORE_HEADERS).$(GALCORE_HEADERS_SUFFIX)
GALCORE_HEADERS_SOURCE	:= $(SRCDIR)/$(GALCORE_HEADERS).$(GALCORE_HEADERS_SUFFIX)
GALCORE_HEADERS_DIR	:= $(BUILDDIR)/$(GALCORE_HEADERS)
GALCORE_HEADERS_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

$(GALCORE_HEADERS_SOURCE):
	@$(call targetinfo)
	@$(call get, GALCORE_HEADERS)

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------
$(STATEDIR)/galcore_headers.extreact:
	@$(call targetinfo)
	@$(call extract, GALCORE_HEADERS)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------
$(STATEDIR)/galcore_headers.prepare:
	@$(call targetinfo)
	
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/galcore_headers.compile:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/galcore_headers.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/galcore_headers.targetinstall:
	@$(call targetinfo)
	@$(call install_init, galcore_headers)
	@$(call install_fixup, galcore_headers,PRIORITY,optional)
	@$(call install_fixup, galcore_headers,SECTION,base)
	@$(call install_fixup, galcore_headers,AUTHOR,"<>WAGO GmbH @AUTHOR@ Co KG")
	@$(call install_fixup, galcore_headers,DESCRIPTION,missing)
	tar xjvf $(SRCDIR)/$(GALCORE_HEADERS).$(GALCORE_HEADERS_SUFFIX) -C $(PTXCONF_SYSROOT_TARGET)/usr/include
	@$(call install_finish, galcore_headers)
	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/galcore_headers.clean:
	@$(call targetinfo)
	@$(call clean, $(SYSROOT_TARGET)/usr/include/galcore_headers)
	@$(call clean, $(ROOTDIR)/usr/include/galcore_headers)
	@$(call clean, $(GALCORE_HEADERS_DIR))
	@$(call clean_pkg, GALCORE_HEADERS)


vim: syntax=make
