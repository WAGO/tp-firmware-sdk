# -*-makefile-*-
#
# Copyright (C) 2018 by <elrest>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_TCP_MONITOR) += tcp_monitor

#
# Paths and names
#
TCP_MONITOR_VERSION	:= 1
TCP_MONITOR_MD5		:=
TCP_MONITOR		:= tcp_monitor
TCP_MONITOR_URL		:= file://local_src/$(TCP_MONITOR)
TCP_MONITOR_DIR		:= $(BUILDDIR)/$(TCP_MONITOR)
TCP_MONITOR_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/tcp_monitor.extract:
#	@$(call targetinfo)
#	@$(call clean, $(TCP_MONITOR_DIR))
#	@$(call extract, TCP_MONITOR)
#	@$(call patchin, TCP_MONITOR)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#TCP_MONITOR_PATH	:= PATH=$(CROSS_PATH)
TCP_MONITOR_CONF_TOOL	:= NO
TCP_MONITOR_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/tcp_monitor.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, TCP_MONITOR)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/tcp_monitor.compile:
#	@$(call targetinfo)
#	@$(call world/compile, TCP_MONITOR)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/tcp_monitor.install:
#	@$(call targetinfo)
#	@$(call world/install, TCP_MONITOR)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/tcp_monitor.targetinstall:
	@$(call targetinfo)

	@$(call install_init, tcp_monitor)
	@$(call install_fixup, tcp_monitor,PRIORITY,optional)
	@$(call install_fixup, tcp_monitor,SECTION,base)
	@$(call install_fixup, tcp_monitor,AUTHOR,"<elrest>")
	@$(call install_fixup, tcp_monitor,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, tcp_monitor, 0, 0, 0755, $(TCP_MONITOR_DIR)/tcp_monitor, /usr/bin/tcp_monitor)

	@$(call install_finish, tcp_monitor)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/tcp_monitor.clean:
	@$(call targetinfo)
	cd $(TCP_MONITOR_DIR) && rm -f *.o $(TCP_MONITOR)	
	@-cd $(TCP_MONITOR_DIR) && \
		$(TCP_MONITOR_MAKE_ENV) $(TCP_MONITOR_PATH) $(MAKE) clean
	@$(call clean_pkg, TCP_MONITOR)

# vim: syntax=make
