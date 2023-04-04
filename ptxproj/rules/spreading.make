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
PACKAGES-$(PTXCONF_SPREADING) += spreading

#
# Paths and names and versions
#
SPREADING_VERSION	:= 1.0
SPREADING		:= spreading-$(SPREADING_VERSION)
SPREADING_URL		:= lndir://$(PTXDIST_WORKSPACE)/local_src/spreading
SPREADING_DIR		:= $(BUILDDIR)/$(SPREADING)
SPREADING_LICENSE	:= unknown

ifdef PTXCONF_SPREADING
$(STATEDIR)/kernel.targetinstall.post: $(STATEDIR)/spreading.targetinstall
endif

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/spreading.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/spreading.compile:
	@$(call targetinfo)
	$(KERNEL_PATH) $(KERNEL_ENV) $(MAKE) $(KERNEL_MAKEVARS) \
		-C $(KERNEL_DIR) \
		M=$(SPREADING_DIR) \
		modules
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/spreading.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/spreading.targetinstall:
	@$(call targetinfo)
	$(KERNEL_PATH) $(KERNEL_ENV) $(MAKE) $(KERNEL_MAKEVARS) \
		-C $(KERNEL_DIR) \
		M=$(SPREADING_DIR) \
		modules_install
	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/spreading.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, SPREADING)

# vim: syntax=make
