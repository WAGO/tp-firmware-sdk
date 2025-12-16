# -*-makefile-*-
#
# Copyright (C) 2013 by Michael Olbrich <m.olbrich@pengutronix.de>
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_OPUS) += opus

#
# Paths and names
#
OPUS_VERSION	:= 1.5.2
OPUS_MD5	:= c40b3a1fbdbb9a7aa178600b88200c76
OPUS		:= opus-$(OPUS_VERSION)
OPUS_SUFFIX	:= tar.gz
OPUS_URL	:= http://downloads.xiph.org/releases/opus/$(OPUS).$(OPUS_SUFFIX)
OPUS_SOURCE	:= $(SRCDIR)/$(OPUS).$(OPUS_SUFFIX)
OPUS_DIR	:= $(BUILDDIR)/$(OPUS)
OPUS_LICENSE	:= BSD-3-Clause

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

OPUS_INTRINSICS	:= y
ifdef PTXCONF_ARCH_PPC
OPUS_INTRINSICS	:=
endif
ifdef PTXCONF_ARCH_ARM
ifndef PTXCONF_ARCH_ARM_NEON
OPUS_INTRINSICS	:=
endif
endif
#ifeq ($(OPUS_INTRINSICS),)
#OPUS_ASM	:= y
#endif
ifdef PTXCONF_ARCH_PPC
OPUS_ASM	:=
endif
OPUS_RTCD	:= $(if $(OPUS_INTRINSICS)$(OPUS_ASM),y)
OPUS_DEEP_PLC	:= $(PTXCONF_OPUS_DEEP_PLC)
OPUS_OSCE	:= $(PTXCONF_OPUS_OSCE)
ifndef PTXCONF_HAS_HARDFLOAT
OPUS_DEEP_PLC	:=
OPUS_OSCE	:=
endif

OPUS_CONF_TOOL	:= meson
OPUS_CONF_OPT	:= \
	$(CROSS_MESON_USR) \
	-Dasm=$(call ptx/endis, OPUS_ASM)d \
	-Dassertions=false \
	-Dcheck-asm=false \
	-Dcustom-modes=false \
	-Ddeep-plc=$(call ptx/endis, OPUS_DEEP_PLC)d \
	-Ddnn-debug-float=disabled \
	-Ddred=disabled \
	-Ddocdir=doc/opus \
	-Ddocs=disabled \
	-Dextra-programs=disabled \
	-Dfixed-point=$(call ptx/falsetrue, PTXCONF_HAS_HARDFLOAT) \
	-Dfixed-point-debug=false \
	-Dfloat-api=true \
	-Dfloat-approx=true \
	-Dfuzzing=false \
	-Dhardening=true \
	-Dintrinsics=$(call ptx/endis, OPUS_INTRINSICS)d \
	-Dosce=$(call ptx/endis, OPUS_OSCE)d \
	-Drtcd=$(call ptx/endis, OPUS_RTCD)d \
	-Dtests=disabled

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/opus.targetinstall:
	@$(call targetinfo)

	@$(call install_init, opus)
	@$(call install_fixup, opus,PRIORITY,optional)
	@$(call install_fixup, opus,SECTION,base)
	@$(call install_fixup, opus,AUTHOR,"Michael Olbrich <m.olbrich@pengutronix.de>")
	@$(call install_fixup, opus,DESCRIPTION,missing)

	@$(call install_lib, opus, 0, 0, 0644, libopus)

	@$(call install_finish, opus)

	@$(call touch)

# vim: syntax=make
