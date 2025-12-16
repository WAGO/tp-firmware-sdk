# -*-makefile-*-
#
# Copyright (C) 2017 by <elrest>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_MENUQTSLIDE) += menuqtslide

#
# Paths and names
#
ifdef PTXCONF_MENUQTSLIDE_TRUNK
MENUQTSLIDE_VERSION	:= trunk
else
MENUQTSLIDE_VERSION	:= 1.0.1
MENUQTSLIDE_MD5		:=
endif
#MENUQTSLIDE		:= menuqtslide-$(MENUQTSLIDE_VERSION)
MENUQTSLIDE		:= menuqtslide
MENUQTSLIDE_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/$(MENUQTSLIDE)
MENUQTSLIDE_DIR		:= $(BUILDDIR)/$(MENUQTSLIDE)
MENUQTSLIDE_BUILD_OOT	:= YES
MENUQTSLIDE_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

#$(MENUQTSLIDE_SOURCE):
#	@$(call targetinfo)
#	@$(call get, MENUQTSLIDE)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#MENUQTSLIDE_CONF_ENV	:= $(CROSS_ENV)

#
# qmake
#
MENUQTSLIDE_PATH		:= PATH=$(PTXDIST_SYSROOT_CROSS)/bin/qt5:$(PTXDIST_SYSROOT_CROSS)/bin/qt6:$(CROSS_PATH)
MENUQTSLIDE_CONF_TOOL	:= qmake
MENUQTSLIDE_CONF_OPT	:= $(CROSS_QMAKE_OPT) PREFIX=/usr

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/menuqtslide.targetinstall:
	@$(call targetinfo)

	@$(call install_init, menuqtslide)
	@$(call install_fixup, menuqtslide, PRIORITY, optional)
	@$(call install_fixup, menuqtslide, SECTION, base)
	@$(call install_fixup, menuqtslide, AUTHOR, "<elrest>")
	@$(call install_fixup, menuqtslide, DESCRIPTION, missing)

#	#
#	# example code:; copy all binaries
#	#

	@for i in $(shell cd $(MENUQTSLIDE_PKGDIR) && find bin sbin usr/bin usr/sbin -type f); do \
		$(call install_copy, menuqtslide, 0, 0, 0755, -, /$$i); \
	done

#	#
#	# FIXME: add all necessary things here
#	#

	@$(call install_finish, menuqtslide)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/menuqtslide.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, MENUQTSLIDE)

# vim: syntax=make
