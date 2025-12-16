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
HOST_PACKAGES-$(PTXCONF_HOST_WAGO_CODE_SIGNING) += host-wago-code-signing

#
# Paths and names
#
HOST_WAGO_CODE_SIGNING_VERSION	:= 1.0.0
HOST_WAGO_CODE_SIGNING		:= wago-code-signing-$(HOST_WAGO_CODE_SIGNING_VERSION)
HOST_WAGO_CODE_SIGNING_URL		:= file://local_src/wago-code-signing
HOST_WAGO_CODE_SIGNING_DIR		:= $(HOST_BUILDDIR)/$(HOST_WAGO_CODE_SIGNING)
HOST_WAGO_CODE_SIGNING_LICENSE	:= MPL-2.0

HOST_WAGO_CODE_SIGNING_CONF_TOOL	:= NO

RAUC_CERT ?= $(PTXCONF_RAUC_CERT)
RAUC_KEY ?= $(PTXCONF_RAUC_KEY)

define in_ptxdist_path_or_as_is
$(or $(call ptx/in-path, PTXDIST_PATH, $(call remove_quotes, $(1))),$(strip $(1)))
endef

HOST_WAGO_CODE_SIGNING_RAUC_CERT := $(call in_ptxdist_path_or_as_is, $(RAUC_CERT))
HOST_WAGO_CODE_SIGNING_RAUC_KEY := $(call in_ptxdist_path_or_as_is, $(RAUC_KEY))

$(call ptx/cfghash, HOST_WAGO_CODE_SIGNING, $(HOST_WAGO_CODE_SIGNING_RAUC_CERT))
$(call ptx/cfghash-file, HOST_WAGO_CODE_SIGNING, $(HOST_WAGO_CODE_SIGNING_RAUC_CERT))

$(call ptx/cfghash, HOST_WAGO_CODE_SIGNING, $(HOST_WAGO_CODE_SIGNING_RAUC_KEY))
$(call ptx/cfghash-file, HOST_WAGO_CODE_SIGNING, $(HOST_WAGO_CODE_SIGNING_RAUC_KEY))


# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

HOST_WAGO_CODE_SIGNING_MAKE_ENV	:= \
	$(CODE_SIGNING_ENV) \
	RAUC_CERT=$(HOST_WAGO_CODE_SIGNING_RAUC_CERT) \
	RAUC_KEY=$(HOST_WAGO_CODE_SIGNING_RAUC_KEY)

$(STATEDIR)/host-wago-code-signing.compile:
	@$(call targetinfo)
	@$(call world/execute, HOST_WAGO_CODE_SIGNING, \
		./ptxdist-set-keys.sh)
	@$(call touch)

$(STATEDIR)/host-wago-code-signing.install:
	@$(call targetinfo)
	@$(call touch)

# vim: syntax=make
