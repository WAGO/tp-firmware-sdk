# -*-makefile-*-
#
# Copyright (C) 2018 by Juergen Borleis <jbe@pengutronix.de>
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
HOST_PACKAGES-$(PTXCONF_HOST_LIBTASN1) += host-libtasn1

#
# Paths and names
#
HOST_LIBTASN1_DIR	= $(HOST_BUILDDIR)/$(LIBTASN1)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#
# autoconf
#
HOST_LIBTASN1_CONF_TOOL	:= autoconf
HOST_LIBTASN1_CONF_OPT	:= \
	$(HOST_AUTOCONF) \
	--disable-doc \
	--disable-gtk-doc \
	--disable-gtk-doc-html \
	--disable-gtk-doc-pdf \
	--disable-valgrind-tests \
	--disable-code-coverage \
	--disable-gcc-warnings

# Needed for old compilers that default to c90
HOST_LIBTASN1_CFLAGS := -std=c99

# vim: syntax=make
