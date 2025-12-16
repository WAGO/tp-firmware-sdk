# -*-makefile-*-
#
# Copyright (C) 2021 by <TBi>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
ifndef PTXCONF_MODBUS_TESTS_SKIP_TARGETINSTALL
PACKAGES-$(PTXCONF_MODBUS_TESTS) += modbus-tests
else
LAZY_PACKAGES-$(PTXCONF_MODBUS_TESTS) += modbus-tests
endif

#
# Paths and names
#
MODBUS_TESTS_VERSION		:= 1.1.0
MODBUS_TESTS		:= modbus-tests
MODBUS_TESTS_URL		:= file://wago_intern/device/modbus/tests
MODBUS_TESTS_SRC_DIR		:= $(PTXDIST_WORKSPACE)/wago_intern/device/modbus/tests
MODBUS_TESTS_DIR		:=$(BUILDDIR)/$(MODBUS_TESTS)
MODBUS_TESTS_RTU_CLIENT		:= mbrtu_client
MODBUS_TESTS_RTU_SERVER		:= mbrtu_server

MODBUS_TESTS_PACKAGE_NAME	:= $(MODBUS_TESTS)_$(MODBUS_TESTS_VERSION)_$(PTXDIST_IPKG_ARCH_STRING)
MODBUS_TESTS_PLATFORMCONFIG_PACKAGEDIR := $(PTXDIST_PLATFORMCONFIGDIR)/packages

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
$(STATEDIR)/$(MODBUS_TESTS).extract:
	@$(call targetinfo)
	@mkdir -p $(MODBUS_TESTS_DIR)
	@$(call touch)
endif

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#$(STATEDIR)/$(MODBUS_TESTS).prepare:
#	@$(call targetinfo)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

MODBUS_TESTS_MAKE_ENV := $(CROSS_ENV)

$(STATEDIR)/$(MODBUS_TESTS).compile:
	@$(call targetinfo)
ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
	@$(call world/compile, MODBUS_TESTS)
endif
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/$(MODBUS_TESTS).install:
	@$(call targetinfo)
ifndef PTXCONF_WAGO_TOOLS_BUILD_VERSION_BINARIES
	@mkdir -p $(MODBUS_TESTS_PKGDIR)/usr/bin/
	@mv $(MODBUS_TESTS_DIR)/$(MODBUS_TESTS_RTU_CLIENT) $(MODBUS_TESTS_PKGDIR)/usr/bin/
	@mv $(MODBUS_TESTS_DIR)/$(MODBUS_TESTS_RTU_SERVER) $(MODBUS_TESTS_PKGDIR)/usr/bin/
ifdef PTXCONF_WAGO_TOOLS_BUILD_VERSION_RELEASE
	@mkdir -p $(MODBUS_TESTS_PLATFORMCONFIG_PACKAGEDIR) && \
	  cd $(MODBUS_TESTS_PKGDIR) && tar cvzf $(MODBUS_TESTS_PLATFORMCONFIG_PACKAGEDIR)/$(MODBUS_TESTS_PACKAGE_NAME).tgz *
endif
else
	@mkdir -p $(MODBUS_TESTS_PKGDIR) && \
	  tar xvzf $(MODBUS_TESTS_PLATFORMCONFIG_PACKAGEDIR)/$(MODBUS_TESTS_PACKAGE_NAME).tgz -C $(MODBUS_TESTS_PKGDIR)
endif
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/$(MODBUS_TESTS).targetinstall:
	@$(call targetinfo)
	@$(call install_init, $(MODBUS_TESTS))

	@$(call install_fixup, $(MODBUS_TESTS), PRIORITY, optional)
	@$(call install_fixup, $(MODBUS_TESTS), SECTION, base)
	@$(call install_fixup, $(MODBUS_TESTS), AUTHOR, "TBi")
	@$(call install_fixup, $(MODBUS_TESTS), DESCRIPTION, missing)

	@$(call install_copy, $(MODBUS_TESTS), 0, 0, 0755, -, /usr/bin/$(MODBUS_TESTS_RTU_CLIENT))
	@$(call install_copy, $(MODBUS_TESTS), 0, 0, 0755, -, /usr/bin/$(MODBUS_TESTS_RTU_SERVER))

	@$(call install_finish, $(MODBUS_TESTS))
	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/$(MODBUS_TESTS_NAME).clean:
	@$(call targetinfo)
	@$(call world/clean, MODBUS_TESTS_NAME)
	@$(call clean_pkg, MODBUS_TESTS_NAME)
	@rm -f $(ROOTDIR)/usr/bin/$(MODBUS_TESTS_RTU_CLIENT)
	@rm -f $(ROOTDIR)/usr/bin/$(MODBUS_TESTS_RTU_SERVER)

# vim: syntax=make

