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
PACKAGES-$(PTXCONF_PDFJS) += pdfjs

#
# Paths and names
#
PDFJS_VERSION	:= 1.9.426
PDFJS_MD5	:= a07de53b9fae7be352514f48b40e0920
PDFJS		:= pdfjs-$(PDFJS_VERSION)
PDFJS_SUFFIX	:= tgz
PDFJS_URL  := https://svsv01003/svn/repo14/$(PDFJS).$(PDFJS_SUFFIX)
PDFJS_SOURCE	:= $(SRCDIR)/$(PDFJS).$(PDFJS_SUFFIX)
PDFJS_DIR	:= $(BUILDDIR)/$(PDFJS)


# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

$(PDFJS_SOURCE):
	@$(call targetinfo)
	@$(call get, PDFJS)
	@$(call touch)

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/PDFJS.extract:
#	@$(call targetinfo)
#	@$(call clean, $(PDFJS_DIR))
#	@$(call extract, PDFJS)
#	@$(call patchin, PDFJS)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

$(STATEDIR)/pdfjs.prepare:
	@$(call touch)


# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/pdfjs.compile:
	@$(call touch)


# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/pdfjs.install:
	@$(call touch)


# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/pdfjs.targetinstall:
	@$(call targetinfo)

	@$(call install_init, pdfjs)
	@$(call install_fixup, pdfjs,PRIORITY,optional)
	@$(call install_fixup, pdfjs,SECTION,base)
	@$(call install_fixup, pdfjs,AUTHOR,"<elrest>")
	@$(call install_fixup, pdfjs,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#

	@$(call install_archive, pdfjs, 0, 0, $(PDFJS_SOURCE), /etc/specific/webengine)
	@$(call install_finish, pdfjs)
	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

