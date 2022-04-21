# -*-makefile-*-
#
# Copyright (C) 2014 by <elrest GmbH>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_SCREEN_CARE) += screen_care

#
# Paths and names
#
ifdef PTXCONF_SCREEN_CARE_TRUNK
SCREEN_CARE_VERSION	:= trunk
else
SCREEN_CARE_VERSION	:= 1.0.1
SCREEN_CARE_MD5		:=
endif
#SCREEN_CARE		:= screen_care-$(SCREEN_CARE_VERSION)
SCREEN_CARE		:= screen_care
SCREEN_CARE_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/$(SCREEN_CARE)
SCREEN_CARE_DIR		:= $(BUILDDIR)/$(SCREEN_CARE)
SCREEN_CARE_BUILD_OOT	:= YES
SCREEN_CARE_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

#$(SCREEN_CARE_SOURCE):
#	@$(call targetinfo)
#	@$(call get, SCREEN_CARE)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#SCREEN_CARE_CONF_ENV	:= $(CROSS_ENV)

#
# qmake
#
SCREEN_CARE_CONF_TOOL	:= qmake
SCREEN_CARE_CONF_OPT	:= $(CROSS_QMAKE_OPT) PREFIX=/usr

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/screen_care.targetinstall:
	@$(call targetinfo)

	@$(call install_init, screen_care)
	@$(call install_fixup, screen_care, PRIORITY, optional)
	@$(call install_fixup, screen_care, SECTION, base)
	@$(call install_fixup, screen_care, AUTHOR, "<elrest GmbH>")
	@$(call install_fixup, screen_care, DESCRIPTION, missing)

#	#
#	# example code:; copy all binaries
#	#

	@for i in $(shell cd $(SCREEN_CARE_PKGDIR) && find bin sbin usr/bin usr/sbin -type f); do \
		$(call install_copy, screen_care, 0, 0, 0755, -, /$$i); \
	done

#	#
#	# FIXME: add all necessary things here
#	#

	@$(call install_finish, screen_care)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/screen_care.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, SCREEN_CARE)

# vim: syntax=make
