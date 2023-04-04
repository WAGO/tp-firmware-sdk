# -*-makefile-*-
#
# Copyright (C) 2007 by Robert Schwebel
#               2008 by Marc Kleine-Budde <mkl@pengutronix.de>
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
HOST_PACKAGES-$(PTXCONF_HOST_LIBXML2) += host-libxml2

#
# Paths and names
#
HOST_LIBXML2_DIR	= $(HOST_BUILDDIR)/$(LIBXML2)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#
# autoconf
#
HOST_LIBXML2_CONF_TOOL	:= autoconf
HOST_LIBXML2_CONF_OPT	:= \
	$(HOST_AUTOCONF) \
	--disable-static \
	--without-iconv \
	--without-icu \
	--without-python \
	--without-lzma

# vim: syntax=make
