# -*-makefile-*-
#
# Copyright (C) 2018 by Michael Olbrich <m.olbrich@pengutronix.de>
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
HOST_PACKAGES-$(PTXCONF_HOST_LIBEPOXY) += host-libepoxy

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#
# autoconf
#
HOST_LIBEPOXY_CONF_TOOL	:= meson
HOST_LIBEPOXY_CONF_OPT	:= \
	$(HOST_MESON_OPT) \
	-Ddocs=false \
	-Degl=no \
	-Dglx=no \
	-Dtests=false \
	-Dx11=false

# vim: syntax=make
