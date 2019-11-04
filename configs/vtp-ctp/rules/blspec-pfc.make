# -*-makefile-*-
#
# Copyright (C) 2018 by <oleg.karfich@wago.com>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_BLSPEC_PFC) += blspec-pfc

BLSPEC_PFC_VERSION		:= 0.0.1
BLSPEC_PFC_KERNEL_VERSION	:= $(call remove_quotes,$(PTXCONF_KERNEL_VERSION))
BLSPEC_PFC_ROOTARG_SD_ROOTFS	:= $(call remove_quotes,$(PTXCONF_BLSPEC_PFC_SD_ROOTARG_ROOTFS))
BLSPEC_PFC_ROOTARG_EMMC_ROOTFS0	:= $(call remove_quotes,$(PTXCONF_BLSPEC_PFC_EMMC_ROOTARG_ROOTFS0))
BLSPEC_PFC_ROOTARG_EMMC_ROOTFS1	:= $(call remove_quotes,$(PTXCONF_BLSPEC_PFC_EMMC_ROOTARG_ROOTFS1))

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/blspec-pfc.targetinstall: $(STATEDIR)/kernel.targetinstall.post


$(STATEDIR)/blspec-pfc.targetinstall:
	@$(call targetinfo)

	@$(call install_init,  blspec-pfc)
	@$(call install_fixup, blspec-pfc, PRIORITY, optional)
	@$(call install_fixup, blspec-pfc, SECTION, base)
	@$(call install_fixup, blspec-pfc, AUTHOR, "<oleg.karfich@wago.com>")
	@$(call install_fixup, blspec-pfc, DESCRIPTION, missing)

# SD-Entrys
	@$(foreach file, $(shell find $(PTXDIST_PLATFORMCONFIGDIR)/projectroot/boot/sd/loader/entries/ -type f), \
		@$(call install_alternative, blspec-pfc, 0, 0, 0755, /boot/sd/loader/entries/$(notdir $(file))) \
		@$(call install_replace, blspec-pfc, /boot/sd/loader/entries/$(notdir $(file)), @KERNEL_VERSION@, "$(BLSPEC_PFC_KERNEL_VERSION)") \
		@$(call install_replace, blspec-pfc, /boot/sd/loader/entries/$(notdir $(file)), @ROOTARG@, "$(BLSPEC_PFC_ROOTARG_SD_ROOTFS)"))

# EMMC-Entrys
	@$(foreach file, $(shell find $(PTXDIST_PLATFORMCONFIGDIR)/projectroot/boot/emmc0/loader/entries/ -type f), \
		@$(call install_alternative, blspec-pfc, 0, 0, 0755, /boot/emmc0/loader/entries/$(notdir $(file))) \
		@$(call install_replace, blspec-pfc, /boot/emmc0/loader/entries/$(notdir $(file)), @KERNEL_VERSION@, "$(BLSPEC_PFC_KERNEL_VERSION)") \
		@$(call install_replace, blspec-pfc, /boot/emmc0/loader/entries/$(notdir $(file)), @ROOTARG@, "$(BLSPEC_PFC_ROOTARG_EMMC_ROOTFS0)"))

	@$(foreach file, $(shell find $(PTXDIST_PLATFORMCONFIGDIR)/projectroot/boot/emmc1/loader/entries/ -type f), \
		@$(call install_alternative, blspec-pfc, 0, 0, 0755, /boot/emmc1/loader/entries/$(notdir $(file))) \
		@$(call install_replace, blspec-pfc, /boot/emmc1/loader/entries/$(notdir $(file)), @KERNEL_VERSION@, "$(BLSPEC_PFC_KERNEL_VERSION)") \
		@$(call install_replace, blspec-pfc, /boot/emmc1/loader/entries/$(notdir $(file)), @ROOTARG@, "$(BLSPEC_PFC_ROOTARG_EMMC_ROOTFS1)"))


# WORKAROUND: this is only a workaround until the build infrastructure will handle
# this properly. Currently [17.12.2018] the build infrastructure needs the at
# least one artifact of the following pattern:
#
#	- *.hdimg
#	- *commission*
#	- *wago-production*
#	- vmlinux
#	- firmware.tar
#
# Some of this files are not available on e.g. vtp-ctp builds. To fullfill the
# dependency, we create dummy files for the build infrastructure to solve broken
# builds. Remove this workaround if it is fixed by the development team

	@> $(IMAGEDIR)/vtp-ctp-commission.img
	@> $(IMAGEDIR)/vtp-ctp-wago-production.img
	@> $(IMAGEDIR)/vmlinux
	@> $(IMAGEDIR)/firmware.tar

	@$(call install_finish,blspec-pfc)

	@$(call touch)

# vim: syntax=make
