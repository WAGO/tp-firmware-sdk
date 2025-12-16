# -*-makefile-*-
#
# Copyright (C) 2024 by Ian Abbott <abbotti@mev.co.uk>
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_LIBWEBSOCKETS) += libwebsockets

#
# Paths and names
#
LIBWEBSOCKETS_VERSION		:= 4.3.3
LIBWEBSOCKETS_MD5		:= c078b08b712316f6302f54a9d05273ae
LIBWEBSOCKETS			:= libwebsockets-$(LIBWEBSOCKETS_VERSION)
LIBWEBSOCKETS_SUFFIX		:= tar.gz
LIBWEBSOCKETS_URL		:= https://github.com/warmcat/libwebsockets/archive/refs/tags/v$(LIBWEBSOCKETS_VERSION).$(LIBWEBSOCKETS_SUFFIX)
LIBWEBSOCKETS_SOURCE		:= $(SRCDIR)/$(LIBWEBSOCKETS).$(LIBWEBSOCKETS_SUFFIX)
LIBWEBSOCKETS_DIR		:= $(BUILDDIR)/$(LIBWEBSOCKETS)
LIBWEBSOCKETS_LICENSE		:= MIT AND BSD-2-Clause AND BSD-3-Clause
LIBWEBSOCKETS_LICENSE_FILES	:= file://LICENSE;md5=382bfdf329e774859fd401eaf850d29b
LIBWEBSOCKETS_DEVPKG    := NO

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#
# cmake
#
LIBWEBSOCKETS_CONF_TOOL	:= cmake
LIBWEBSOCKETS_CONF_OPT	:=  \
	$(CROSS_CMAKE_USR) \
	-DLWS_WITH_ZLIB=ON \
	-DLWS_WITH_SSL=$(call ptx/onoff, PTXCONF_LIBWEBSOCKETS_TLS) \
	-DLWS_WITH_LIBEV=$(call ptx/onoff, PTXCONF_LIBWEBSOCKETS_LIBEV) \
	-DLWS_WITH_LIBUV=$(call ptx/onoff, PTXCONF_LIBWEBSOCKETS_LIBUV) \
	-DLWS_WITH_LIBEVENT=$(call ptx/onoff, PTXCONF_LIBWEBSOCKETS_LIBEVENT) \
	-DLWS_WITH_GLIB=$(call ptx/onoff, PTXCONF_LIBWEBSOCKETS_GLIB) \
	-DLWS_WITHOUT_TESTAPPS=ON \
	-DLWS_WITHOUT_TEST_PING=ON \
	-DLWS_WITHOUT_TEST_CLIENT=ON \
	-DLWS_WITH_EXTERNAL_POLL=ON \
	-DLWS_SSL_CLIENT_USE_OS_CA_CERTS=OFF \
	-DLWS_WITH_EVLIB_PLUGINS=OFF \
	-DLWS_WITHOUT_EVENTFD=ON \
	-DLWS_WITH_NETLINK=OFF \
	-DLWS_WITH_SYS_SMD=OFF

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/libwebsockets.targetinstall:
	@$(call targetinfo)

	@$(call install_init, libwebsockets)
	@$(call install_fixup, libwebsockets,PRIORITY,optional)
	@$(call install_fixup, libwebsockets,SECTION,base)
	@$(call install_fixup, libwebsockets,AUTHOR,"Ian Abbott <abbotti@mev.co.uk>")
	@$(call install_fixup, libwebsockets,DESCRIPTION,missing)

#	libraries
	@$(call install_lib, libwebsockets, 0, 0, 0644, libwebsockets)

#	plug-in libraries
ifdef PTXCONF_LIBWEBSOCKETS_LIBEV
	@$(call install_lib, libwebsockets, 0, 0, 0644, libwebsockets-evlib_ev)
endif
ifdef PTXCONF_LIBWEBSOCKETS_LIBUV
	@$(call install_lib, libwebsockets, 0, 0, 0644, libwebsockets-evlib_uv)
endif
ifdef PTXCONF_LIBWEBSOCKETS_LIBEVENT
	@$(call install_lib, libwebsockets, 0, 0, 0644, libwebsockets-evlib_event)
endif
ifdef PTXCONF_LIBWEBSOCKETS_GLIB
	@$(call install_lib, libwebsockets, 0, 0, 0644, libwebsockets-evlib_glib)
endif

	@$(call install_copy, libwebsockets, 0, 0, 0644, $(LIBWEBSOCKETS_DIR)/LICENSE, /usr/share/licenses/oss/license.libwebsockets_$(LIBWEBSOCKETS_VERSION).txt)

	@$(call install_finish, libwebsockets)

	@$(call touch)

# vim: syntax=make
