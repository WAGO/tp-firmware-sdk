# -*-makefile-*-
#
# Copyright (C) 2016 by <elrest>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_WEBENGINEBROWSER) += webenginebrowser

#
# Paths and names
#
ifdef PTXCONF_WEBENGINEBROWSER_TRUNK
WEBENGINEBROWSER_VERSION	:= trunk
else
WEBENGINEBROWSER_VERSION	:= 1.1.1
WEBENGINEBROWSER_MD5		:=
endif
#WEBENGINEBROWSER		:= webenginebrowser-$(WEBENGINEBROWSER_VERSION)
WEBENGINEBROWSER		:= webenginebrowser
WEBENGINEBROWSER_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/$(WEBENGINEBROWSER)
WEBENGINEBROWSER_DIR		:= $(BUILDDIR)/$(WEBENGINEBROWSER)
WEBENGINEBROWSER_BUILD_OOT	:= YES
WEBENGINEBROWSER_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

#$(WEBENGINEBROWSER_SOURCE):
#	@$(call targetinfo)
#	@$(call get, WEBENGINEBROWSER)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#WEBENGINEBROWSER_CONF_ENV	:= $(CROSS_ENV)

#
# qmake
#
WEBENGINEBROWSER_PATH				:= PATH=$(PTXDIST_SYSROOT_CROSS)/bin/qt5:$(CROSS_PATH)
WEBENGINEBROWSER_CONF_TOOL	:= qmake
WEBENGINEBROWSER_CONF_OPT		:= $(CROSS_QMAKE_OPT) PREFIX=/usr

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/webenginebrowser.targetinstall:
	@$(call targetinfo)

	@$(call install_init, webenginebrowser)
	@$(call install_fixup, webenginebrowser, PRIORITY, optional)
	@$(call install_fixup, webenginebrowser, SECTION, base)
	@$(call install_fixup, webenginebrowser, AUTHOR, "<elrest>")
	@$(call install_fixup, webenginebrowser, DESCRIPTION, missing)

#	#
#	# example code:; copy all binaries
#	#

	@for i in $(shell cd $(WEBENGINEBROWSER_PKGDIR) && find bin sbin usr/bin usr/sbin -type f); do \
		$(call install_copy, webenginebrowser, 0, 0, 0755, -, /$$i); \
	done

#	#
#	# FIXME: add all necessary things here
#	#

	@$(call install_finish, webenginebrowser)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/webenginebrowser.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, WEBENGINEBROWSER)

# vim: syntax=make
