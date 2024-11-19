# -*-makefile-*-
#
# Copyright (C) 2007 by Robert Schwebel
#               2010 by Marc Kleine-Budde <mkl@pengutronix.de>
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
HOST_PACKAGES-$(PTXCONF_HOST_LIBXSLT) += host-libxslt

#
# Paths and names
#
HOST_LIBXSLT_DIR	= $(HOST_BUILDDIR)/$(LIBXSLT)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#
# autoconf
#
HOST_LIBXSLT_CONF_TOOL	:= autoconf
HOST_LIBXSLT_CONF_OPT	:= \
	$(HOST_AUTOCONF) \
	--disable-static \
	--without-python \
	--without-crypto \
	--without-debug \
	--without-debugger \
	--without-profiler \
	--without-plugins

# vim: syntax=make
