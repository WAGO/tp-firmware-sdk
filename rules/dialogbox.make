# -*-makefile-*-
#
# Copyright (C) 2014 by <elrest>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_DIALOGBOX) += dialogbox

#
# Paths and names
#
DIALOGBOX_VERSION	:= 1.0.1
DIALOGBOX_MD5		:=
DIALOGBOX		:= dialogbox
DIALOGBOX_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/$(DIALOGBOX)
DIALOGBOX_DIR		:= $(BUILDDIR)/$(DIALOGBOX)
DIALOGBOX_BUILD_OOT	:= YES
DIALOGBOX_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

#$(DIALOGBOX_SOURCE):
#	@$(call targetinfo)
#	@$(call get, DIALOGBOX)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#DIALOGBOX_CONF_ENV	:= $(CROSS_ENV)

#
# qmake
#
DIALOGBOX_CONF_TOOL	:= qmake
DIALOGBOX_CONF_OPT	:= $(CROSS_QMAKE_OPT) PREFIX=/usr

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/dialogbox.targetinstall:
	@$(call targetinfo)

	@$(call install_init, dialogbox)
	@$(call install_fixup, dialogbox, PRIORITY, optional)
	@$(call install_fixup, dialogbox, SECTION, base)
	@$(call install_fixup, dialogbox, AUTHOR, "<elrest>")
	@$(call install_fixup, dialogbox, DESCRIPTION, missing)

#	#
#	# example code:; copy all binaries
#	#

	@for i in $(shell cd $(DIALOGBOX_PKGDIR) && find bin sbin usr/bin usr/sbin -type f); do \
		$(call install_copy, dialogbox, 0, 0, 0755, -, /$$i); \
	done

#	#
#	# FIXME: add all necessary things here
#	#

	@$(call install_finish, dialogbox)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/dialogbox.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, DIALOGBOX)

# vim: syntax=make
