# -*-makefile-*-
#
# Copyright (C) 2019 by <elrest Automationssysteme GmbH>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_AUDIOSERVER) += audioserver

#
# Paths and names
#
AUDIOSERVER_VERSION	:= 1
AUDIOSERVER_MD5		:=
AUDIOSERVER		:= audioserver
AUDIOSERVER_URL		:= file://local_src/audioserver
AUDIOSERVER_DIR		:= $(BUILDDIR)/$(AUDIOSERVER)
AUDIOSERVER_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/audioserver.extract:
#	@$(call targetinfo)
#	@$(call clean, $(AUDIOSERVER_DIR))
#	@$(call extract, AUDIOSERVER)
#	@$(call patchin, AUDIOSERVER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#AUDIOSERVER_PATH	:= PATH=$(CROSS_PATH)
AUDIOSERVER_CONF_TOOL	:= NO
AUDIOSERVER_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/audioserver.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, AUDIOSERVER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/audioserver.compile:
#	@$(call targetinfo)
#	@$(call world/compile, AUDIOSERVER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/audioserver.install:
#	@$(call targetinfo)
#	@$(call world/install, AUDIOSERVER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/audioserver.targetinstall:
	@$(call targetinfo)

	@$(call install_init, audioserver)
	@$(call install_fixup, audioserver,PRIORITY,optional)
	@$(call install_fixup, audioserver,SECTION,base)
	@$(call install_fixup, audioserver,AUTHOR,"<elrest Automationssysteme GmbH>")
	@$(call install_fixup, audioserver,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, audioserver, 0, 0, 0755, $(AUDIOSERVER_DIR)/audioserver, /usr/bin/audioserver)

	@$(call install_finish, audioserver)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/audioserver.clean:
	@$(call targetinfo)
	@-cd $(AUDIOSERVER_DIR) && \
		$(AUDIOSERVER_ENV) $(AUDIOSERVER_PATH) $(MAKE) clean
	@$(call clean_pkg, AUDIOSERVER)

# vim: syntax=make
