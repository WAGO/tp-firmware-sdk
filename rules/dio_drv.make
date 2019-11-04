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
PACKAGES-$(PTXCONF_DIO_DRV) += dio_drv

#
# Paths and names and versions
#
DIO_DRV_VERSION	:= 1
DIO_DRV		:= dio_drv-$(DIO_DRV_VERSION)
DIO_DRV_URL		:= lndir://$(PTXDIST_WORKSPACE)/local_src/$(DIO_DRV)
DIO_DRV_DIR		:= $(BUILDDIR)/$(DIO_DRV)
DIO_DRV_LICENSE	:= unknown

ifdef PTXCONF_DIO_DRV
$(STATEDIR)/kernel.targetinstall.post: $(STATEDIR)/dio_drv.targetinstall
endif

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/dio_drv.prepare:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/dio_drv.compile:
	@$(call targetinfo)
	$(KERNEL_PATH) $(KERNEL_ENV) $(MAKE) $(KERNEL_MAKEVARS) \
		-C $(KERNEL_DIR) \
		M=$(DIO_DRV_DIR) \
		modules
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/dio_drv.install:
	@$(call targetinfo)
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/dio_drv.targetinstall:
	@$(call targetinfo)
	$(KERNEL_PATH) $(KERNEL_ENV) $(MAKE) $(KERNEL_MAKEVARS) \
		-C $(KERNEL_DIR) \
		M=$(DIO_DRV_DIR) \
		modules_install
	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/dio_drv.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, DIO_DRV)

# vim: syntax=make
