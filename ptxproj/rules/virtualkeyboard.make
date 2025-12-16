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
PACKAGES-$(PTXCONF_VIRTUALKEYBOARD) += virtualkeyboard

#
# Paths and names
#
ifdef PTXCONF_VIRTUALKEYBOARD_TRUNK
VIRTUALKEYBOARD_VERSION	:= trunk
else
VIRTUALKEYBOARD_VERSION	:= 1
VIRTUALKEYBOARD_MD5		:=
endif
VIRTUALKEYBOARD		:= virtualkeyboard
VIRTUALKEYBOARD_URL		:= file://$(PTXDIST_WORKSPACE)/local_src/$(VIRTUALKEYBOARD)
VIRTUALKEYBOARD_DIR		:= $(BUILDDIR)/$(VIRTUALKEYBOARD)
VIRTUALKEYBOARD_BUILD_OOT	:= YES
VIRTUALKEYBOARD_LICENSE	:= unknown
VIRTUALKEYBOARD_DEVPKG  := NO

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

#$(VIRTUALKEYBOARD_SOURCE):
#	@$(call targetinfo)
#	@$(call get, VIRTUALKEYBOARD)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#VIRTUALKEYBOARD_CONF_ENV	:= $(CROSS_ENV)

#
# qmake
#
VIRTUALKEYBOARD_PATH      := PATH=$(PTXDIST_SYSROOT_CROSS)/bin/qt5:$(CROSS_PATH)
VIRTUALKEYBOARD_CONF_TOOL	:= qmake
VIRTUALKEYBOARD_CONF_OPT	:= $(CROSS_QMAKE_OPT) PREFIX=/usr

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/virtualkeyboard.targetinstall:
	@$(call targetinfo)

	@$(call install_init, virtualkeyboard)
	@$(call install_fixup, virtualkeyboard, PRIORITY, optional)
	@$(call install_fixup, virtualkeyboard, SECTION, base)
	@$(call install_fixup, virtualkeyboard, AUTHOR, "<elrest>")
	@$(call install_fixup, virtualkeyboard, DESCRIPTION, missing)

# virtual keyboard needs the plugin vkimplatforminputcontext
	@$(call install_copy, virtualkeyboard, 0, 0, 0755, $(VIRTUALKEYBOARD_DIR)-build/plugin/libvkimplatforminputcontextplugin.so, /usr/lib/qt5/plugins/platforminputcontexts/libvkimplatforminputcontextplugin.so)  

#	#
#	# example code:; copy all binaries
#	#

	@for i in $(shell cd $(VIRTUALKEYBOARD_PKGDIR) && find bin sbin usr/bin usr/sbin -type f); do \
		$(call install_copy, virtualkeyboard, 0, 0, 0755, -, /$$i); \
	done

#	#
#	# FIXME: add all necessary things here
#	#

	@$(call install_finish, virtualkeyboard)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/virtualkeyboard.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, VIRTUALKEYBOARD)

# vim: syntax=make
