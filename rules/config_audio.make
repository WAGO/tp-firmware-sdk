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
PACKAGES-$(PTXCONF_CONFIG_AUDIO) += config_audio

#
# Paths and names
#
CONFIG_AUDIO_VERSION	:= 0.0.1
CONFIG_AUDIO_MD5		:=
CONFIG_AUDIO		    := config_audio
CONFIG_AUDIO_URL		:= file://local_src/$(CONFIG_AUDIO)
CONFIG_AUDIO_DIR		:= $(BUILDDIR)/$(CONFIG_AUDIO)
CONFIG_AUDIO_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_audio.extract:
#	@$(call targetinfo)
#	@$(call clean, $(CONFIG_AUDIO_DIR))
#	@$(call extract, CONFIG_AUDIO)
#	@$(call patchin, CONFIG_AUDIO)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#CONFIG_AUDIO_PATH	:= PATH=$(CROSS_PATH)
CONFIG_AUDIO_CONF_TOOL	:= NO
CONFIG_AUDIO_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/config_audio.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, CONFIG_AUDIO)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_audio.compile:
#	@$(call targetinfo)
#	@$(call world/compile, CONFIG_AUDIO)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_audio.install:
#	@$(call targetinfo)
#	@$(call world/install, CONFIG_AUDIO)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/config_audio.targetinstall:
	@$(call targetinfo)

	@$(call install_init, config_audio)
	@$(call install_fixup, config_audio,PRIORITY,optional)
	@$(call install_fixup, config_audio,SECTION,base)
	@$(call install_fixup, config_audio,AUTHOR,"<elrest>")
	@$(call install_fixup, config_audio,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, config_audio, 0, 0, 0755, $(CONFIG_AUDIO_DIR)/config_audio, /etc/config-tools/config_audio)

	@$(call install_finish, config_audio)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/config_audio.clean:
	@$(call targetinfo)
	cd $(CONFIG_AUDIO_DIR) && rm -f *.o $(CONFIG_AUDIO)	
	@-cd $(CONFIG_AUDIO_DIR) && \
		$(CONFIG_AUDIO_MAKE_ENV) $(CONFIG_AUDIO_PATH) $(MAKE) clean
	@$(call clean_pkg, CONFIG_AUDIO)

# vim: syntax=make
