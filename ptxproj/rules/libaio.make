# -*-makefile-*-
#
# Copyright (C) 2016 by Juergen Borleis <jbe@pengutronix.de>
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
ifndef PTXCONF_LIBAIO_SKIP_TARGETINSTALL
PACKAGES-$(PTXCONF_LIBAIO) += libaio
else
LAZY_PACKAGES-$(PTXCONF_LIBAIO) += libaio
endif

#
# Paths and names
#
LIBAIO_VERSION		:= 0.3.113
LIBAIO_MD5		:= 4422d9f1655f358d74ff48af2a3b9f49
LIBAIO			:= libaio-$(LIBAIO_VERSION)
LIBAIO_SUFFIX		:= tar.gz
LIBAIO_URL		:= https://pagure.io/libaio/archive/$(LIBAIO)/libaio-$(LIBAIO).$(LIBAIO_SUFFIX)
LIBAIO_SOURCE		:= $(SRCDIR)/$(LIBAIO).$(LIBAIO_SUFFIX)
LIBAIO_DIR		:= $(BUILDDIR)/$(LIBAIO)
LIBAIO_LICENSE		:= LGPL-2.1-only
LIBAIO_LICENSE_FILES	:= \
	file://COPYING;md5=d8045f3b8f929c1cb29a1e3fd737b499

# ----------------------------------------------------------------------------
# Prepare + Compile
# ----------------------------------------------------------------------------

#
# just a Makefile made by a creative Linux kernel hacker...
#
LIBAIO_CONF_TOOL	:= NO
LIBAIO_MAKE_ENV		:= $(CROSS_ENV)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/libaio.targetinstall:
	@$(call targetinfo)

	@$(call install_init, libaio)
	@$(call install_fixup, libaio,PRIORITY,optional)
	@$(call install_fixup, libaio,SECTION,base)
	@$(call install_fixup, libaio,AUTHOR,"Juergen Borleis <jbe@pengutronix.de>")
	@$(call install_fixup, libaio,DESCRIPTION,"asynchronous I/O access library")

	@$(call install_lib, libaio, 0, 0, 0644, libaio)

	@$(call install_finish, libaio)

	@$(call touch)

# vim: syntax=make
