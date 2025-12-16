# -*-makefile-*-
#
# Copyright (C) 2017 by Alexander Dahl <ada@thorsis.com>
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_MOSQUITTO) += mosquitto

#
# Paths and names
#
MOSQUITTO_VERSION	:= 2.0.20
MOSQUITTO_MD5		:= 2e8002042c758a55b32e32fbe6326980
MOSQUITTO		:= mosquitto-$(MOSQUITTO_VERSION)
MOSQUITTO_SUFFIX	:= tar.gz
MOSQUITTO_URL		:= https://mosquitto.org/files/source/$(MOSQUITTO).$(MOSQUITTO_SUFFIX)
MOSQUITTO_SOURCE	:= $(SRCDIR)/$(MOSQUITTO).$(MOSQUITTO_SUFFIX)
MOSQUITTO_DIR		:= $(BUILDDIR)/$(MOSQUITTO)
# "Eclipse Distribution License - v 1.0" is in fact BSD-3-Clause
MOSQUITTO_LICENSE	:= EPL-2.0 OR BSD-3-Clause
MOSQUITTO_LICENSE_FILES	:= \
	file://LICENSE.txt;md5=ca9a8f366c6babf593e374d0d7d58749 \
	file://epl-v20;md5=2dd765ca47a05140be15ebafddbeadfe \
	file://edl-v10;md5=9f6accb1afcb570f8be65039e2fcd49e
MOSQUITTO_DEVPKG    := NO

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

MOSQUITTO_CONF_TOOL	:= cmake
MOSQUITTO_CONF_OPT	:= \
	$(CROSS_CMAKE_USR) \
	-DDOCUMENTATION=OFF \
	-DINC_BRIDGE_SUPPORT=ON \
	-DINC_DB_UPGRADE=ON \
	-DINC_MEMTRACK=ON \
	-DUSE_LIBWRAP=OFF \
	-DWITH_ADNS=OFF \
	-DWITH_APPS=ON \
	-DWITH_BROKER=$(call ptx/onoff, PTXCONF_MOSQUITTO_BROKER) \
	-DWITH_BUNDLED_DEPS=ON \
	-DWITH_CLIENTS=$(call ptx/onoff, PTXCONF_MOSQUITTO_CLIENTS) \
	-DWITH_CONTROL=ON \
	-DWITH_DLT=OFF \
	-DWITH_EC=ON \
	-DWITH_LIB_CPP=ON \
	-DWITH_PERSISTENCE=ON \
	-DWITH_PLUGINS=ON \
	-DWITH_SOCKS=ON \
	-DWITH_SRV=$(call ptx/onoff, PTXCONF_MOSQUITTO_SRV) \
	-DWITH_STATIC_LIBRARIES=OFF \
	-DWITH_SYSTEMD=$(call ptx/onoff, PTXCONF_MOSQUITTO_SYSTEMD_UNIT) \
	-DWITH_SYS_TREE=ON \
	-DWITH_THREADING=ON \
	-DWITH_TLS=$(call ptx/onoff, PTXCONF_MOSQUITTO_TLS) \
	-DWITH_TLS_PSK=$(call ptx/onoff, PTXCONF_MOSQUITTO_TLS) \
	-DWITH_UNIX_SOCKETS=ON \
	-DWITH_WEBSOCKETS=$(call ptx/onoff, PTXCONF_MOSQUITTO_WEBSOCKETS)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------
$(STATEDIR)/mosquitto.install:
	@$(call targetinfo)
	@$(call world/install, MOSQUITTO)
	@install -v -D -m644 $(MOSQUITTO_DIR)/service/systemd/mosquitto.service.notify \
		$(MOSQUITTO_PKGDIR)/usr/lib/systemd/system/mosquitto.service
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/mosquitto.targetinstall:
	@$(call targetinfo)

	@$(call install_init, mosquitto)
	@$(call install_fixup, mosquitto,PRIORITY,optional)
	@$(call install_fixup, mosquitto,SECTION,base)
	@$(call install_fixup, mosquitto,AUTHOR,"Alexander Dahl <ada@thorsis.com>")
	@$(call install_fixup, mosquitto,DESCRIPTION,missing)

	@$(call install_lib, mosquitto, 0, 0, 0644, libmosquitto)
	@$(call install_lib, mosquitto, 0, 0, 0644, libmosquittopp)

	@cp $(MOSQUITTO_DIR)/edl-v10 $(MOSQUITTO_DIR)/license
	@echo -e "----------------------------------------\n" >> $(MOSQUITTO_DIR)/license
	@cat $(MOSQUITTO_DIR)/epl-v20 >> $(MOSQUITTO_DIR)/license

	@$(call install_copy, mosquitto, 0, 0, 0644, $(MOSQUITTO_DIR)/license, /usr/share/licenses/oss/license.mosquitto_$(MOSQUITTO_VERSION).txt)

ifdef PTXCONF_MOSQUITTO_CLIENTS
	@$(call install_copy, mosquitto, 0, 0, 0755, -, /usr/bin/mosquitto_pub)
	@$(call install_copy, mosquitto, 0, 0, 0755, -, /usr/bin/mosquitto_sub)
endif

ifdef PTXCONF_MOSQUITTO_BROKER
	@$(call install_copy, mosquitto, 0, 0, 0755, -, /usr/sbin/mosquitto)
	@$(call install_alternative, mosquitto, 0, 0, 0644, \
		/etc/mosquitto/mosquitto.conf)

ifdef PTXCONF_MOSQUITTO_SYSTEMD_UNIT
	@$(call install_copy, mosquitto, 0, 0, 0644, -, \
		/usr/lib/systemd/system/mosquitto.service)
	@$(call install_link, mosquitto, ../mosquitto.service, \
		/usr/lib/systemd/system/multi-user.target.wants/mosquitto.service)
endif
endif

	@$(call install_finish, mosquitto)

	@$(call touch)

# vim: ft=make noet ts=8 sw=8
