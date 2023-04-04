# -*-makefile-*-
#
# Copyright (C) 2013 by Jan Luebbe <jlu@pengutronix.de>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
IMAGE_PACKAGES-$(PTXCONF_IMAGE_DATA_UBI) += image-data-ubi

#
# Paths and names
#
IMAGE_DATA_UBI		:= image-data-ubi
IMAGE_DATA_UBI_DIR	:= $(BUILDDIR)/$(IMAGE_DATA_UBI)
IMAGE_DATA_UBI_IMAGE	:= $(IMAGEDIR)/data.ubi
IMAGE_DATA_UBI_FILES	:= $(IMAGEDIR)/root.tgz
IMAGE_DATA_UBI_CONFIG	:= data-ubi.config

# ----------------------------------------------------------------------------
# Image
# ----------------------------------------------------------------------------

$(IMAGE_DATA_UBI_IMAGE):
	@$(call targetinfo)
	@$(call image/genimage, IMAGE_DATA_UBI)
	@$(call finish)

# vim: syntax=make
