# -*-makefile-*-
#
# Copyright (C) 2013 by <elrest>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_GET_AUDIO) += get_audio

#
# Paths and names
#
GET_AUDIO_VERSION	:= 0.0.1
GET_AUDIO_MD5		:=
GET_AUDIO		    := get_audio
GET_AUDIO_URL		:= file://local_src/$(GET_AUDIO)
GET_AUDIO_DIR		:= $(BUILDDIR)/$(GET_AUDIO)
GET_AUDIO_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_audio.extract:
#	@$(call targetinfo)
#	@$(call clean, $(GET_AUDIO_DIR))
#	@$(call extract, GET_AUDIO)
#	@$(call patchin, GET_AUDIO)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#GET_AUDIO_PATH	:= PATH=$(CROSS_PATH)
GET_AUDIO_CONF_TOOL	:= NO
GET_AUDIO_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/get_audio.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, GET_AUDIO)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_audio.compile:
#	@$(call targetinfo)
#	@$(call world/compile, GET_AUDIO)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_audio.install:
#	@$(call targetinfo)
#	@$(call world/install, GET_AUDIO)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/get_audio.targetinstall:
	@$(call targetinfo)

	@$(call install_init, get_audio)
	@$(call install_fixup, get_audio,PRIORITY,optional)
	@$(call install_fixup, get_audio,SECTION,base)
	@$(call install_fixup, get_audio,AUTHOR,"<elrest>")
	@$(call install_fixup, get_audio,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, get_audio, 0, 0, 0755, $(GET_AUDIO_DIR)/get_audio, /etc/config-tools/get_audio)

	@$(call install_finish, get_audio)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/get_audio.clean:
	@$(call targetinfo)
	cd $(GET_AUDIO_DIR) && rm -f *.o $(GET_AUDIO)	
	@-cd $(GET_AUDIO_DIR) && \
		$(GET_AUDIO_MAKE_ENV) $(GET_AUDIO_PATH) $(MAKE) clean
	@$(call clean_pkg, GET_AUDIO)

# vim: syntax=make
