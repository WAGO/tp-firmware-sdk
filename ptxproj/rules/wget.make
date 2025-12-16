# -*-makefile-*-
#
# Copyright (C) 2005-2008 by Robert Schwebel
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_WGET) += wget

#
# Paths and names
#
WGET_VERSION	:= 1.25.0
WGET_MD5	:= c70ba58b36f944e8ba1d655ace552881
WGET		:= wget-$(WGET_VERSION)
WGET_SUFFIX	:= tar.gz
WGET_URL	:= $(call ptx/mirror, GNU, wget/$(WGET).$(WGET_SUFFIX))
WGET_SOURCE	:= $(SRCDIR)/$(WGET).$(WGET_SUFFIX)
WGET_DIR	:= $(BUILDDIR)/$(WGET)
WGET_LICENSE	:= GPL-3.0-or-later
WGET_LICENSE_FILES := \
	file://COPYING;md5=6f65012d1daf98cb09b386cfb68df26b \
	file://AUTHORS;md5=73ec735b2ed0c22cf0ee49a0a5b3a62f \
	file://src/main.c;startline=1;endline=28;md5=2b7b073526085b41e4d396c90ebadf47

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

WGET_CONF_ENV := \
	$(CROSS_ENV) \
	ac_cv_prog_MAKEINFO=: \
	ac_cv_path_PERL=: \
	ac_cv_path_POD2MAN=:

#
# autoconf
#
WGET_CONF_TOOL := autoconf
WGET_CONF_OPT := \
	$(CROSS_AUTOCONF_USR) \
	--enable-opie \
	--enable-digest \
	--disable-ntlm \
	--enable-debug \
	--disable-valgrind-tests \
	--enable-assert \
	$(GLOBAL_LARGE_FILE_OPTION) \
	--enable-threads=posix \
	--disable-nls \
	--disable-rpath \
	--disable-code-coverage \
	$(GLOBAL_IPV6_OPTION) \
	--disable-iri \
	--disable-pcre2 \
	--disable-pcre \
	--disable-xattr \
	--without-libpsl \
	--with-ssl=$(call remove_quotes, $(PTXCONF_WGET_SSL)) \
	--$(call ptx/wwo, PTXCONF_WGET_ZLIB)-zlib \
	--with-metalink \
	--without-cares \
	--$(call ptx/wwo, PTXCONF_WGET_SSL_OPENSSL)-openssl \
	--with-included-libunistring \
	--without-included-regex \
	--without-libuuid


# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/wget.targetinstall:
	@$(call targetinfo)

	@$(call install_init, wget)
	@$(call install_fixup, wget,PRIORITY,optional)
	@$(call install_fixup, wget,SECTION,base)
	@$(call install_fixup, wget,AUTHOR,"Robert Schwebel <r.schwebel@pengutronix.de>")
	@$(call install_fixup, wget,DESCRIPTION,missing)

	@$(call install_copy, wget, 0, 0, 0755, -, /usr/bin/wget)

	@$(call install_finish, wget)

	@$(call touch)

# vim: syntax=make
