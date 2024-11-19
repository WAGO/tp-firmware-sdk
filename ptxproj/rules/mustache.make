# -*-makefile-*-
#
# Copyright (C) 2024 by <elrest>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_MUSTACHE) += mustache

#
# Paths and names
#
MUSTACHE_VERSION	:= 2.14.2
MUSTACHE_MD5	:= 1785a660c744faf587154bd67daa6897
MUSTACHE		:= mustache-$(MUSTACHE_VERSION)
MUSTACHE_SUFFIX	:= tgz
MUSTACHE_URL  := https://svsv01003/svn/repo14/$(MUSTACHE).$(MUSTACHE_SUFFIX)
MUSTACHE_SOURCE	:= $(SRCDIR)/$(MUSTACHE).$(MUSTACHE_SUFFIX)
MUSTACHE_DIR	:= $(BUILDDIR)/$(MUSTACHE)


# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

$(MUSTACHE_SOURCE):
	@$(call targetinfo)
	@$(call get, MUSTACHE)
	@$(call touch)

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/MUSTACHE.extract:
#	@$(call targetinfo)
#	@$(call clean, $(MUSTACHE_DIR))
#	@$(call extract, MUSTACHE)
#	@$(call patchin, MUSTACHE)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/mustache.prepare:
	@$(call touch)


# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/mustache.compile:
	@$(call touch)


# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/mustache.install:
	@$(call touch)


# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/mustache.targetinstall:
	@$(call targetinfo)

	@$(call install_init, mustache)
	@$(call install_fixup, mustache,PRIORITY,optional)
	@$(call install_fixup, mustache,SECTION,base)
	@$(call install_fixup, mustache,AUTHOR,"<elrest>")
	@$(call install_fixup, mustache,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#

	@$(call install_archive, mustache, 0, 0, $(MUSTACHE_SOURCE), /usr/share/)
	@$(call install_finish, mustache)
	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

