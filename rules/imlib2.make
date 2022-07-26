# -*-makefile-*-
#
# Copyright (C) 2021 by <elrest>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_IMLIB2) += imlib2

#
# Paths and names
#
IMLIB2_VERSION	:= 1.7.1
IMLIB2_MD5		:= 459696517408020dd09b11b33a78f9ff
IMLIB2		:= imlib2-$(IMLIB2_VERSION)
IMLIB2_SUFFIX	:= tar.gz
IMLIB2_URL	:= $(call ptx/mirror, SF, imlib2/$(IMLIB2).$(IMLIB2_SUFFIX))
IMLIB2_SOURCE	:= $(SRCDIR)/$(IMLIB2).$(IMLIB2_SUFFIX)
IMLIB2_DIR		:= $(BUILDDIR)/$(IMLIB2)
IMLIB2_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------
IMLIB2_CONF_ENV	:= $(CROSS_ENV)

#
# autoconf
#
IMLIB2_CONF_TOOL	:= autoconf
IMLIB2_CONF_OPT	:= \
	$(CROSS_AUTOCONF_USR) \
	--with-x


# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/imlib2.targetinstall:
	@$(call targetinfo)

	@$(call install_init, imlib2)
	@$(call install_fixup, imlib2,PRIORITY,optional)
	@$(call install_fixup, imlib2,SECTION,base)
	@$(call install_fixup, imlib2,AUTHOR,"Michael Olbrich <m.olbrich@pengutronix.de>")
	@$(call install_fixup, imlib2,DESCRIPTION,missing)

	@$(call install_lib, imlib2, 0, 0, 0644, libImlib2)
#	@$(call install_copy, imlib2, 0, 0, 0755, -, /usr/bin/imlib2_conv)
#	@$(call install_copy, imlib2, 0, 0, 0755, -, /usr/bin/imlib2_bumpmap)
#	@$(call install_copy, imlib2, 0, 0, 0755, -, /usr/bin/imlib2_show)
#	@$(call install_copy, imlib2, 0, 0, 0755, -, /usr/bin/imlib2_test)
#	@$(call install_copy, imlib2, 0, 0, 0755, -, /usr/bin/imlib2_view)

	@$(call install_finish, imlib2)

	@$(call touch)

# vim: syntax=make
