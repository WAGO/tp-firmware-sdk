# -*-makefile-*-
#
# Copyright (C) 2025 by WAGO GmbH & Co. KG
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_RAUC_FAILSAFE) += rauc-failsafe

#
# Paths and names
#
RAUC_FAILSAFE_VERSION	:= 1.0.0
RAUC_FAILSAFE			:= rauc-failsafe
RAUC_FAILSAFE_LICENSE	:= MPL-2.0

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/rauc-failsafe.targetinstall:
	@$(call targetinfo)

	@$(call install_init, rauc-failsafe)
	@$(call install_fixup, rauc-failsafe,PRIORITY,optional)
	@$(call install_fixup, rauc-failsafe,SECTION,base)
	@$(call install_fixup, rauc-failsafe,AUTHOR,"WAGO GmbH \& Co. KG")
	@$(call install_fixup, rauc-failsafe,DESCRIPTION,missing)

	@$(call install_alternative, rauc-failsafe, 0, 0, 0755, /usr/sbin/rauc_failsafe.sh)

	@$(call install_finish, rauc-failsafe)

	@$(call touch)

# vim: syntax=make
