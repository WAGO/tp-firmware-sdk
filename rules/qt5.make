# -*-makefile-*-
#
# Copyright (C) 2012 by Michael Olbrich <m.olbrich@pengutronix.de>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_QT5) += qt5

#
# Paths and names
#
QT5_VERSION	:= 5.9.7
#QT5_MD5		:= 738d1b98106e1bd39f00cc228beb522a #QT5.9.2
#QT5_MD5		:= eae2435509493a2084405714e0a9bdf9	#QT5.9.3
#QT5_MD5		:= d95f6ecb491e731c2fca622ccc7b6bbd	#QT5.9.4
QT5_MD5			:= 70e617aeb1f9bbf84a12b8cf09b01ece	#QT5.9.7
QT5		:= qt-everywhere-opensource-src-$(QT5_VERSION)
QT5_SUFFIX	:= tar.xz
QT5_URL		:= \
	http://download.qt-project.org/official_releases/qt/$(basename $(QT5_VERSION))/$(QT5_VERSION)/single/$(QT5).$(QT5_SUFFIX) \
	http://download.qt-project.org/development_releases/qt/$(basename $(QT5_VERSION))/$(shell echo $(QT5_VERSION) | tr 'A-Z' 'a-z')/single/$(QT5).$(QT5_SUFFIX)
QT5_SOURCE	:= $(SRCDIR)/$(QT5).$(QT5_SUFFIX)
QT5_DIR		:= $(BUILDDIR)/$(QT5)
QT5_BUILD_OOT	:= YES
QT5_LICENSE	:= LGPL-2.1, Nokia-Qt-exception-1.1, LGPL-3.0, GFDL-1.3
QT5_LICENSE_FILES := \
	file://LICENSE.LGPLv21;md5=4bfd28363f541b10d9f024181b8df516 \
	file://LGPL_EXCEPTION.txt;md5=9625233da42f9e0ce9d63651a9d97654 \
	file://LICENSE.GPLv3;md5=88e2b9117e6be406b5ed6ee4ca99a705 \
	file://LICENSE.LGPLv3;md5=e0459b45c5c4840b353141a8bbed91f0 \
	file://LICENSE.FDL;md5=6d9f2a9af4c8b8c3c769f6cc1b6aaf7e
QT5_MKSPECS	:= $(shell ptxd_get_alternative config/qt5 linux-ptx-g++ && echo $$ptxd_reply)

ifdef PTXCONF_QT5
ifeq ($(strip $(QT5_MKSPECS)),)
$(error Qt5 mkspecs are missing)
endif
endif

# broken on PPC
ifdef PTXCONF_ARCH_PPC
PTXCONF_QT5_MODULE_QTCONNECTIVITY :=
PTXCONF_QT5_MODULE_QTCONNECTIVITY_QUICK :=
PTXCONF_QT5_MODULE_QTSCRIPT :=
PTXCONF_QT5_MODULE_QTSCRIPT_WIDGETS :=
PTXCONF_QT5_MODULE_QTWEBENGINE :=
PTXCONF_QT5_MODULE_QTWEBENGINE_WIDGETS :=
PTXCONF_QT5_MODULE_QTWEBVIEW :=
endif
# QtWebEngine needs at least ARMv6
ifdef PTXCONF_ARCH_ARM
ifndef PTXCONF_ARCH_ARM_V6
PTXCONF_QT5_MODULE_QTWEBENGINE :=
PTXCONF_QT5_MODULE_QTWEBENGINE_WIDGETS :=
PTXCONF_QT5_MODULE_QTWEBVIEW :=
endif
endif

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

# the extra section seems to confuse the Webkit JIT code
QT5_WRAPPER_BLACKLIST := \
	TARGET_COMPILER_RECORD_SWITCHES

# target options are provided via mkspecs
QT5_CONF_ENV := \
	$(CROSS_ENV_PKG_CONFIG) \
	MAKEFLAGS="$(PTXDIST_PARALLELMFLAGS)" \
	COMPILER_PREFIX=$(COMPILER_PREFIX) \
	PKG_CONFIG_SYSROOT_DIR=$(PTXCONF_SYSROOT_TARGET)
#	HOST_SYSROOT_DIR=$(PTXDIST_PLATFORMDIR)/sysroot-host \
#	PKG_CONFIG_LIBDIR=$(PTXCONF_SYSROOT_TARGET)/usr/lib/pkgconfig


QT5_MAKE_ENV := \
	$(CROSS_ENV_PKG_CONFIG) \
	PKG_CONFIG_SYSROOT_DIR=/.

define ptx/qt5-system
$(call ptx/ifdef, PTXCONF_$(strip $(1)),-system,-no)
endef

define ptx/qt5-module
$(call ptx/ifdef, PTXCONF_QT5_MODULE_$(strip $(1)),,-skip $(2))
endef

#
# autoconf
#
QT5_CONF_TOOL	:= autoconf

#
# Note: autoconf style options are not shown in '--help' but they can be used
# This also avoid the problem where e.g. '-largefile' also matches '-l<library>'
#
QT5_CONF_OPT	:= \
	$(if $(filter 1,$(PTXDIST_VERBOSE)),-v) \
	$(if $(filter 0,$(PTXDIST_VERBOSE)),-silent) \
	-prefix /usr \
	-headerdir /usr/include/qt5 \
	-archdatadir /usr/lib/qt5 \
	-datadir /usr/share/qt5 \
	-examplesdir /usr/lib/qt5/examples \
	-hostbindir /usr/bin/qt5 \
	-release \
	-opensource \
	-confirm-license \
	$(call ptx/ifdef, PTXCONF_QT5_ACCESSIBILITY,-accessibility,-no-accessibility) \
	-no-sql-db2 \
	-no-sql-ibase \
	$(call ptx/ifdef, PTXCONF_QT5_MODULE_QTBASE_SQL_MYSQL,-sql-mysql,-no-sql-mysql) \
	-no-sql-oci \
	-no-sql-odbc \
	-no-sql-psql \
	-no-sql-sqlite2 \
	-no-sql-tds \
	$(call ptx/ifdef, PTXCONF_QT5_MODULE_QTBASE_SQL_SQLITE,-sql-sqlite,-no-sql-sqlite) \
	$(call ptx/ifdef, PTXCONF_QT5_MODULE_QTDECLARATIVE_DEBUG,-qml-debug,-no-qml-debug) \
	-pkg-config \
	-force-pkg-config \
	\
	-system-zlib \
	-no-mtdev \
	$(call ptx/ifdef, PTXCONF_QT5_JOURNALD,-journald,-no-journald) \
	-no-syslog \
	$(call ptx/ifdef, PTXCONF_QT5_GIF,,-no-gif) \
	$(call ptx/qt5-system, QT5_LIBPNG)-libpng \
	$(call ptx/ifdef, QT5_LIBJPEG,-qt-libjpeg,-qt-libjpeg) \
	$(call ptx/qt5-system, QT5_GUI)-freetype \
	-qt-harfbuzz \
	$(call ptx/ifdef, PTXCONF_QT5_OPENSSL,-openssl,-no-openssl) \
	-no-libproxy \
	-qt-pcre \
	-system-xcb \
	$(call ptx/qt5-system, QT5_PLATFORM_XCB)-xkbcommon-x11 \
	$(call ptx/ifdef, PTXCONF_QT5_INPUT_LIBINPUT,-xkbcommon-evdev,-no-xkbcommon-evdev) \
	$(call ptx/ifdef, PTXCONF_QT5_XI,-xinput2,-no-xinput2) \
	$(call ptx/ifdef, PTXCONF_QT5_X11,-xcb-xlib,-no-xcb-xlib) \
	$(call ptx/ifdef, PTXCONF_QT5_GLIB,-glib,-no-glib) \
	\
	-make libs \
	-make tools \
	$(call ptx/ifdef, PTXCONF_QT5_PREPARE_EXAMPLES,-make examples) \
	$(call ptx/qt5-module, QT3D, qt3d) \
	-skip qtactiveqt \
	-skip qtandroidextras \
	$(call ptx/qt5-module, QTCANVAS3D, qtcanvas3d) \
	$(call ptx/qt5-module, QTCHARTS, qtcharts) \
	$(call ptx/qt5-module, QTCONNECTIVITY, qtconnectivity) \
	$(call ptx/qt5-module, QTDATAVIS3D, qtdatavis3d) \
	$(call ptx/qt5-module, QTDECLARATIVE, qtdeclarative) \
	-skip qtdoc \
	$(call ptx/qt5-module, QTGAMEPAD, qtgamepad) \
	$(call ptx/qt5-module, QTGRAPHICALEFFECTS, qtgraphicaleffects) \
	$(call ptx/qt5-module, QTIMAGEFORMATS, qtimageformats) \
	$(call ptx/qt5-module, QTLOCATION, qtlocation) \
	-skip qtmacextras \
	$(call ptx/qt5-module, QTMULTIMEDIA, qtmultimedia) \
	$(call ptx/qt5-module, QTNETWORKAUTH, qtnetworkauth) \
	$(call ptx/qt5-module, QTPURCHASING, qtpurchasing) \
	$(call ptx/qt5-module, QTQUICKCONTROLS, qtquickcontrols) \
	$(call ptx/qt5-module, QTQUICKCONTROLS2, qtquickcontrols2) \
	$(call ptx/qt5-module, QTREMOTEOBJECTS, qtremoteobjects) \
	$(call ptx/qt5-module, QTSCRIPT, qtscript) \
	$(call ptx/qt5-module, QTSCXML, qtscxml) \
	$(call ptx/qt5-module, QTSENSORS, qtsensors) \
	$(call ptx/qt5-module, QTSERIALBUS, qtserialbus) \
	$(call ptx/qt5-module, QTSERIALPORT, qtserialport) \
	$(call ptx/qt5-module, QTSPEECH, qtspeech) \
	$(call ptx/qt5-module, QTSVG, qtsvg) \
	$(call ptx/qt5-module, QTTOOLS, qttools) \
	$(call ptx/qt5-module, QTTRANSLATIONS, qttranslations) \
	$(call ptx/qt5-module, QTVIRTUALKEYBOARD, qtvirtualkeyboard) \
	$(call ptx/qt5-module, QTWAYLAND, qtwayland) \
	$(call ptx/qt5-module, QTWEBCHANNEL, qtwebchannel) \
	$(call ptx/qt5-module, QTWEBENGINE, qtwebengine) \
	$(call ptx/qt5-module, QTWEBSOCKETS, qtwebsockets) \
	$(call ptx/qt5-module, QTWEBVIEW, qtwebview) \
	-skip qtwinextras \
	$(call ptx/qt5-module, QTX11EXTRAS, qtx11extras) \
	$(call ptx/qt5-module, QTXMLPATTERNS, qtxmlpatterns) \
	$(call ptx/ifdef, PTXCONF_QT5_PREPARE_EXAMPLES,-compile-examples,-no-compile-examples) \
	$(call ptx/ifdef, PTXCONF_QT5_GUI,-gui,-no-gui) \
	$(call ptx/ifdef, PTXCONF_QT5_WIDGETS,-widgets,-no-widgets) \
	-no-rpath \
	-no-cups \
	--$(call ptx/endis, $(call ptx/ifdef, PTXCONF_QT5_ICU,,PTXCONF_ICONV))-iconv \
	$(call ptx/ifdef, PTXCONF_QT5_INPUT_EVDEV,-evdev,-no-evdev) \
	$(call ptx/ifdef, PTXCONF_QT5_INPUT_TSLIB,-tslib,-no-tslib) \
	$(call ptx/ifdef, PTXCONF_QT5_ICU,-icu,-no-icu) \
	$(call ptx/ifdef, PTXCONF_QT5_GUI,-fontconfig,-no-fontconfig) \
	-no-strip \
	-no-pch \
	-no-ltcg \
	-no-separate-debug-info \
	$(call ptx/ifdef, PTXCONF_QT5_PLATFORM_XCB,-xcb,-no-xcb) \
	$(call ptx/ifdef, PTXCONF_QT5_PLATFORM_EGLFS,-eglfs,-no-eglfs) \
	$(call ptx/ifdef, PTXCONF_QT5_PLATFORM_EGLFS_KMS,-kms,-no-kms) \
	$(call ptx/ifdef, PTXCONF_QT5_PLATFORM_EGLFS_KMS,-gbm,-no-gbm) \
	$(call ptx/ifdef, PTXCONF_QT5_PLATFORM_DIRECTFB,-directfb,-no-directfb) \
	$(call ptx/ifdef, PTXCONF_QT5_PLATFORM_LINUXFB,-linuxfb,-no-linuxfb) \
	-no-mirclient \
	$(call ptx/ifdef, PTXCONF_QT5_GUI,-qpa $(PTXCONF_QT5_PLATFORM_DEFAULT)) \
	-xplatform linux-ptx-g++ \
	-$(call ptx/ifdef, PTXCONF_QT5_OPENGL,opengl $(PTXCONF_QT5_OPENGL_API),no-opengl) \
	$(call ptx/ifdef, PTXCONF_QT5_INPUT_LIBINPUT,-libinput,-no-libinput) \
	-no-system-proxies \
	-v

ifdef PTXCONF_QT5_MODULE_QTMULTIMEDIA
QT5_CONF_OPT	+= \
	--disable-pulseaudio \
	--$(call ptx/endis, PTXCONF_QT5_MODULE_QTMULTIMEDIA)-alsa \
	$(call ptx/ifdef, PTXCONF_QT5_MODULE_QTMULTIMEDIA_GST,-gstreamer 1.0,-no-gstreamer)
endif
ifdef PTXCONF_QT5_MODULE_QTWEBENGINE
QT5_CONF_OPT	+= \
	--$(call ptx/endis, PTXCONF_QT5_MODULE_QTWEBENGINE)-alsa \
	--$(call ptx/endis, PTXCONF_QT5_PROPRIETARY_CODECS)-proprietary-codecs \
	-qt-webengine-icu \
	-qt-ffmpeg \
	$(call ptx/qt5-system, QT5_MODULE_QTWEBENGINE)-opus \
	-qt-webp \
	--disable-pepper-plugins \
	--disable-printing-and-pdf \
	--disable-pulseaudio \
	--disable-spellchecker \
	--disable-webrtc
endif

ifdef PTXCONF_QT5_GUI
ifndef PTXCONF_QT5_PLATFORM_DEFAULT
$(error Qt5: select at least one GUI platform!)
endif
endif

# Note: these options are not listed in '--help' but they exist
QT5_CONF_OPT += \
	-no-sm \
	$(call ptx/ifdef, PTXCONF_QT5_GUI,-libudev,-no-libudev) \
	$(call ptx/ifdef, PTXCONF_QT5_OPENGL,-egl,-no-egl) \
	$(call ptx/ifdef, PTXCONF_QT5_PLATFORM_XCB,-xkb,-no-xkb)

ifdef PTXCONF_QT5_MODULE_QTBASE_SQL_MYSQL
QT5_CONF_OPT += -mysql_config $(SYSROOT)/usr/bin/mysql_config
endif

QT5_QMAKE_OPT := CONFIG+=release CONFIG-=debug

ifdef PTXCONF_QT5_MODULE_QTWEBENGINE
QT5_QMAKE_OPT += "PTX_QMAKE_CFLAGS=$(shell ptxd_cross_cc_v | sed -n "s/^COLLECT_GCC_OPTIONS=\(.*\)/\1/p" | tail -n1)"
QT5_MAKE_OPT += NINJAFLAGS="$(PTXDIST_PARALLELMFLAGS) $(PTXDIST_LOADMFLAGS)"
endif
ifdef PTXCONF_QT5_MODULE_QTMULTIMEDIA_GST
QT5_MAKE_OPT += "GST_VERSION=1.0"
else
QT5_QMAKE_OPT += "QT_CONFIG-=gstreamer-0.10 gstreamer-1.0"
endif

$(STATEDIR)/qt5.prepare:
	@$(call targetinfo)
	sed -i "s|USER_SYSROOT_DIR|$(PTXCONF_SYSROOT_TARGET)|g" "$(QT5_DIR)/qtbase/mkspecs/linux-ptx-g++/qmake.conf" && \
	sed -i "s|CROSS_PREFIX|$(COMPILER_PREFIX)|g" "$(QT5_DIR)/qtbase/mkspecs/linux-ptx-g++/qmake.conf" && \
	rm -Rf $(PTXCONF_SYSROOT_TARGET)/usr/lib/qt5 && \
	rm -Rf $(PTXCONF_SYSROOT_TARGET)/usr/lib/libQt5*
	@$(call world/prepare, QT5)
	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/qt5.install:
	@$(call targetinfo)
	@$(call world/install, QT5)
	@find $(QT5_PKGDIR) -name '*.qmltypes' | xargs -r rm
	@$(call touch)

QT5_QT_CONF := $(PTXDIST_SYSROOT_CROSS)/bin/qt5/qt.conf

$(STATEDIR)/qt5.install.post:
	@$(call targetinfo)
	@$(call world/install.post, QT5)
	@rm -rf $(PTXDIST_SYSROOT_CROSS)/bin/qt5
	@cp -a $(SYSROOT)/usr/bin/qt5 $(PTXDIST_SYSROOT_CROSS)/bin/qt5
	@echo "[Paths]"						>  $(QT5_QT_CONF)
	@echo "HostPrefix=$(SYSROOT)/usr"			>> $(QT5_QT_CONF)
	@echo "HostData=$(SYSROOT)/usr/lib/qt5"			>> $(QT5_QT_CONF)
	@echo "HostBinaries=$(PTXDIST_SYSROOT_CROSS)/bin/qt5"	>> $(QT5_QT_CONF)
	@echo "Prefix=$(SYSROOT)/usr"				>> $(QT5_QT_CONF)
	@echo "Headers=$(SYSROOT)/usr/include/qt5"		>> $(QT5_QT_CONF)
	@echo "Imports=/usr/lib/qt5/imports"			>> $(QT5_QT_CONF)
	@echo "Qml2Imports=/usr/lib/qt5/qml"			>> $(QT5_QT_CONF)
	@echo ""						>> $(QT5_QT_CONF)
	# following code moved from targetinstall
#	ifeq (,$(wildcard $(PTXCONF_SYSROOT_TARGET)/usr/bin/qmake))
		@ln -sf $(PTXCONF_SYSROOT_TARGET)/usr/bin/qt5/qmake $(PTXCONF_SYSROOT_TARGET)/usr/bin/qmake 
#	endif

#	ifeq (,$(wildcard $(PTXCONF_SYSROOT_CROSS)/bin/qmake))
		@ln -sf $(PTXCONF_SYSROOT_CROSS)/bin/qt5/qmake $(PTXCONF_SYSROOT_CROSS)/bin/qmake
#	endif

#	ifeq (,$(wildcard $(PTXCONF_SYSROOT_CROSS)/bin/qt.conf))
		@ln -sf $(PTXCONF_SYSROOT_CROSS)/bin/qt5/qt.conf $(PTXCONF_SYSROOT_CROSS)/bin/qt.conf
#	endif
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

QT5_LIBS-y							:=
QT5_QML-y							:=

### Qt3d ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QT3D)				+= Qt53DAnimation Qt53DCore Qt53DExtras Qt53DInput Qt53DLogic Qt53DRender
QT5_LIBS-$(PTXCONF_QT5_MODULE_QT3D_QUICK)			+= Qt53DQuick Qt53DQuickAnimation Qt53DQuickExtras Qt53DQuickInput Qt53DQuickRender Qt53DQuickScene2D
QT5_QML-$(PTXCONF_QT5_MODULE_QT3D_QUICK)			+= Qt3D
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QT3D)				+= geometryloaders/libdefaultgeometryloader
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QT3D)				+= geometryloaders/libgltfgeometryloader
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QT3D)				+= sceneparsers/libgltfsceneexport
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QT3D)				+= sceneparsers/libgltfsceneimport
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QT3D_QUICK)			+= renderplugins/libscene2d

### QtBase ###
QT5_LIBS-y							+= Qt5Core
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTBASE)				+= Qt5Concurrent
QT5_LIBS-$(PTXCONF_QT5_DBUS)					+= Qt5DBus
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTBASE_GUI)			+= Qt5Gui
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTBASE)				+= Qt5Network
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTBASE_OPENGL)			+= Qt5OpenGL
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTBASE_WIDGETS)			+= Qt5PrintSupport
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTBASE_SQL)			+= Qt5Sql
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTBASE)				+= Qt5Test
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTBASE_WIDGETS)			+= Qt5Widgets
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTBASE)				+= Qt5Xml
QT5_LIBS-$(PTXCONF_QT5_PLATFORM_EGLFS)				+= Qt5EglFSDeviceIntegration
QT5_LIBS-$(PTXCONF_QT5_PLATFORM_EGLFS_KMS)			+= Qt5EglFsKmsSupport
QT5_LIBS-$(PTXCONF_QT5_PLATFORM_XCB)				+= Qt5XcbQpa
QT5_PLUGINS-$(PTXCONF_QT5_DBUS)					+= bearer/libqconnmanbearer
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTBASE)			+= bearer/libqgenericbearer
QT5_PLUGINS-$(PTXCONF_QT5_DBUS)					+= bearer/libqnmbearer
QT5_PLUGINS-$(PTXCONF_QT5_INPUT_EVDEV)				+= generic/libqevdevkeyboardplugin
QT5_PLUGINS-$(PTXCONF_QT5_INPUT_EVDEV)				+= generic/libqevdevmouseplugin
QT5_PLUGINS-$(PTXCONF_QT5_INPUT_EVDEV)				+= generic/libqevdevtabletplugin
QT5_PLUGINS-$(PTXCONF_QT5_INPUT_EVDEV)				+= generic/libqevdevtouchplugin
QT5_PLUGINS-$(PTXCONF_QT5_INPUT_LIBINPUT)			+= generic/libqlibinputplugin
QT5_PLUGINS-$(PTXCONF_QT5_INPUT_TSLIB)				+= generic/libqtslibplugin
QT5_PLUGINS-$(PTXCONF_QT5_GIF)					+= imageformats/libqgif
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTBASE_GUI)			+= imageformats/libqico
QT5_PLUGINS-$(PTXCONF_QT5_LIBJPEG)				+= imageformats/libqjpeg
QT5_PLUGINS-$(PTXCONF_QT5_PLATFORM_XCB)				+= platforminputcontexts/libcomposeplatforminputcontextplugin
ifdef PTXCONF_QT5_MODULE_QTBASE_GUI
QT5_PLUGINS-$(PTXCONF_QT5_DBUS)					+= platforminputcontexts/libibusplatforminputcontextplugin
endif
QT5_PLUGINS-$(PTXCONF_QT5_PLATFORM_XCB)				+= platforms/libqxcb
QT5_PLUGINS-$(PTXCONF_QT5_PLATFORM_DIRECTFB)			+= platforms/libqdirectfb
QT5_PLUGINS-$(PTXCONF_QT5_PLATFORM_EGLFS)			+= platforms/libqeglfs
QT5_PLUGINS-$(PTXCONF_QT5_PLATFORM_LINUXFB)			+= platforms/libqlinuxfb
QT5_PLUGINS-$(PTXCONF_QT5_PLATFORM_VNC)				+= platforms/libqvnc
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTBASE_GUI)			+= platforms/libqminimal
QT5_PLUGINS-$(PTXCONF_QT5_PLATFORM_EGLFS)			+= platforms/libqminimalegl
QT5_PLUGINS-$(PTXCONF_QT5_PLATFORM_EGLFS_KMS)			+= egldeviceintegrations/libqeglfs-kms-integration
QT5_PLUGINS-$(PTXCONF_QT5_PLATFORM_EGLFS_X11)			+= egldeviceintegrations/libqeglfs-x11-integration
QT5_PLUGINS-$(PTXCONF_QT5_PLATFORM_XCB)				+= xcbglintegrations/libqxcb-egl-integration
QT5_PLUGINS-$(PTXCONF_QT5_PLATFORM_XCB)				+= xcbglintegrations/libqxcb-glx-integration
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTBASE_SQL_MYSQL)		+= sqldrivers/libqsqlmysql
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTBASE_SQL_SQLITE)		+= sqldrivers/libqsqlite

### QtCanvas3d ###
QT5_QML-$(PTXCONF_QT5_MODULE_QTCANVAS3D_QUICK)			+= QtCanvas3D

### QtCharts ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTCHARTS)				+= Qt5Charts
QT5_QML-$(PTXCONF_QT5_MODULE_QTCHARTS_QUICK)			+= QtCharts

### QtConnectivity ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTCONNECTIVITY)			+= Qt5Bluetooth
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTCONNECTIVITY)			+= Qt5Nfc
QT5_QML-$(PTXCONF_QT5_MODULE_QTCONNECTIVITY_QUICK)		+= QtBluetooth
QT5_QML-$(PTXCONF_QT5_MODULE_QTCONNECTIVITY_QUICK)		+= QtNfc

### QtDataVisualization ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTDATAVIS3D)			+= Qt5DataVisualization
QT5_QML-$(PTXCONF_QT5_MODULE_QTDATAVIS3D_QUICK)			+= QtDataVisualization

### QtDeclarative ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTDECLARATIVE)			+= Qt5Qml
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTDECLARATIVE_QUICK)		+= Qt5Quick
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTDECLARATIVE_QUICK_WIDGETS)	+= Qt5QuickWidgets
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTDECLARATIVE_QUICK)		+= Qt5QuickParticles
QT5_LIBS-							+= Qt5QuickTest
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTDECLARATIVE_DEBUG)		+= qmltooling/libqmldbg_debugger
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTDECLARATIVE_DEBUG)		+= qmltooling/libqmldbg_local
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTDECLARATIVE_DEBUG)		+= qmltooling/libqmldbg_messages
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTDECLARATIVE_DEBUG)		+= qmltooling/libqmldbg_native
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTDECLARATIVE_DEBUG)		+= qmltooling/libqmldbg_nativedebugger
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTDECLARATIVE_DEBUG)		+= qmltooling/libqmldbg_profiler
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTDECLARATIVE_QUICK_DEBUG)	+= qmltooling/libqmldbg_quickprofiler
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTDECLARATIVE_DEBUG)		+= qmltooling/libqmldbg_server
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTDECLARATIVE_DEBUG)		+= qmltooling/libqmldbg_tcp
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTDECLARATIVE_QUICK_DEBUG)	+= qmltooling/libqmldbg_inspector
QT5_QML-$(PTXCONF_QT5_MODULE_QTDECLARATIVE)			+= Qt
QT5_QML-$(PTXCONF_QT5_MODULE_QTDECLARATIVE)			+= QtQuick
QT5_QML-$(PTXCONF_QT5_MODULE_QTDECLARATIVE)			+= QtQuick.2
QT5_QML-$(PTXCONF_QT5_MODULE_QTDECLARATIVE)			+= QtQml
QT5_QML-							+= QtTest

### QtGamepad ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTGAMEPAD)			+= Qt5Gamepad
QT5_QML-$(PTXCONF_QT5_MODULE_QTGAMEPAD)				+= QtGamepad
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTGAMEPAD)			+= gamepads/libevdevgamepad

### QtGraphicalEffects ###
QT5_QML-$(PTXCONF_QT5_MODULE_QTGRAPHICALEFFECTS)		+= QtGraphicalEffects

### QtImageFormats ###
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTIMAGEFORMATS)		+= imageformats/libqicns
QT5_PLUGINS-$(PTXCONF_QT5_LIBMNG)				+= imageformats/libqmng
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTIMAGEFORMATS)		+= imageformats/libqtga
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTIMAGEFORMATS)		+= imageformats/libqtiff
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTIMAGEFORMATS)		+= imageformats/libqwbmp
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTIMAGEFORMATS)		+= imageformats/libqwebp


### QtLocation ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTLOCATION)			+= Qt5Positioning Qt5Location
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTLOCATION)			+= position/libqtposition_positionpoll
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTLOCATION)			+= geoservices/libqtgeoservices_osm
QT5_QML-$(PTXCONF_QT5_MODULE_QTLOCATION_QUICK)			+= QtLocation
QT5_QML-$(PTXCONF_QT5_MODULE_QTLOCATION_QUICK)			+= QtPositioning

### QtMultimedia ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTMULTIMEDIA)			+= Qt5Multimedia
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTMULTIMEDIA_QUICK)		+= Qt5MultimediaQuick_p
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTMULTIMEDIA_WIDGETS)		+= Qt5MultimediaWidgets
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTMULTIMEDIA_GST)			+= qgsttools_p
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTMULTIMEDIA_GST)		+= audio/libqtaudio_alsa
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTMULTIMEDIA_GST)		+= mediaservice/libgstaudiodecoder
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTMULTIMEDIA_GST)		+= mediaservice/libgstcamerabin
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTMULTIMEDIA_GST)		+= mediaservice/libgstmediacapture
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTMULTIMEDIA_GST)		+= mediaservice/libgstmediaplayer
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTMULTIMEDIA)			+= playlistformats/libqtmultimedia_m3u
ifdef PTXCONF_QT5_OPENGL_ES2
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTMULTIMEDIA)			+= video/videonode/libeglvideonode
endif
QT5_QML-$(PTXCONF_QT5_MODULE_QTMULTIMEDIA_QUICK)		+= QtMultimedia

### QtNetworkAuth ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTNETWORKAUTH)			+= Qt5NetworkAuth

### QtQuickControls ###
# all in QT5_QML- added by QtDeclarative

### QtQuickControls2 ###
#QT5_LIBS-$(PTXCONF_QT5_MODULE_QTQUICKCONTROLS2)			+= Qt5QuickTemplates2	# RG TODO
#QT5_LIBS-$(PTXCONF_QT5_MODULE_QTQUICKCONTROLS2)			+= Qt5QuickControls2	# RG TODO

### QtRemoteObjects ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTREMOTEOBJECTS)			+= Qt5RemoteObjects

### QtScript ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTSCRIPT)				+= Qt5Script
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTSCRIPT_WIDGETS)			+= Qt5ScriptTools

### QtScxml ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTSCXML)				+= Qt5Scxml
QT5_QML-$(PTXCONF_QT5_MODULE_QTSCXML_QUICK)			+= QtScxml

### QtSensors ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTSENSORS)			+= Qt5Sensors
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTSENSORS)			+= sensorgestures/libqtsensorgestures_counterplugin
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTSENSORS)			+= sensorgestures/libqtsensorgestures_plugin
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTSENSORS)			+= sensorgestures/libqtsensorgestures_shakeplugin
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTSENSORS)			+= sensors/libqtsensors_generic
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTSENSORS)			+= sensors/libqtsensors_iio-sensor-proxy
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTSENSORS)			+= sensors/libqtsensors_linuxsys
QT5_QML-$(PTXCONF_QT5_MODULE_QTSENSORS_QUICK)			+= QtSensors

### QtSerialBus ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTSERIALBUS)			+= Qt5SerialBus
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTSERIALBUS)			+= canbus/libqtsocketcanbus

### QtSerialPort ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTSERIALPORT)			+= Qt5SerialPort

### QtSpeech ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTSPEECH)				+= Qt5TextToSpeech

### QtSvg ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTSVG)				+= Qt5Svg
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTSVG_WIDGETS)			+= iconengines/libqsvgicon
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTSVG)				+= imageformats/libqsvg

### QtTools ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTTOOLS_WIDGETS)			+= Qt5Designer
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTTOOLS_WIDGETS)			+= Qt5DesignerComponents
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTTOOLS_WIDGETS)			+= Qt5Help

### QtWayland ###
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTVIRTUALKEYBOARD)		+= platforminputcontexts/libqtvirtualkeyboardplugin

### QtWayland ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTWAYLAND)			+= Qt5WaylandClient
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTWAYLAND)			+= Qt5WaylandCompositor
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTWAYLAND)			+= platforms/libqwayland-generic
ifdef PTXCONF_QT5_OPENGL
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTWAYLAND_MESA)		+= platforms/libqwayland-egl
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTWAYLAND_MESA)		+= wayland-graphics-integration-client/libwayland-egl
endif
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTWAYLAND_MESA)		+= wayland-graphics-integration-client/libdrm-egl-server
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTWAYLAND_MESA)		+= wayland-graphics-integration-server/libwayland-egl
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTWAYLAND_MESA)		+= wayland-graphics-integration-server/libdrm-egl-server
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTWAYLAND_MESA)		+= wayland-shell-integration/libivi-shell
QT5_PLUGINS-$(PTXCONF_QT5_MODULE_QTWAYLAND)			+= wayland-decoration-client/libbradient

QT5_QML-$(PTXCONF_QT5_MODULE_QTWAYLAND)				+= QtWayland

### QtWebChannel ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTWEBCHANNEL)			+= Qt5WebChannel
QT5_QML-$(PTXCONF_QT5_MODULE_QTWEBCHANNEL)			+= QtWebChannel

### QtWebEngine ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTWEBENGINE)			+= Qt5WebEngine
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTWEBENGINE)			+= Qt5WebEngineCore
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTWEBENGINE_WIDGETS)		+= Qt5WebEngineWidgets
QT5_QML-$(PTXCONF_QT5_MODULE_QTWEBENGINE)			+= QtWebEngine

### QtWebSockets ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTWEBSOCKETS)			+= Qt5WebSockets
QT5_QML-$(PTXCONF_QT5_MODULE_QTWEBSOCKETS_QUICK)		+= QtWebSockets

### QtWebView ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTWEBVIEW)			+= Qt5WebView
QT5_QML-$(PTXCONF_QT5_MODULE_QTWEBVIEW)				+= QtWebView

### QtX11Extras ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTX11EXTRAS)			+= Qt5X11Extras

### QtXmlPatterns ###
QT5_LIBS-$(PTXCONF_QT5_MODULE_QTXMLPATTERNS)			+= Qt5XmlPatterns



$(STATEDIR)/qt5.targetinstall:
	@$(call targetinfo)

	@$(call install_init, qt5)
	@$(call install_fixup, qt5,PRIORITY,optional)
	@$(call install_fixup, qt5,SECTION,base)
	@$(call install_fixup, qt5,AUTHOR,"Michael Olbrich <m.olbrich@pengutronix.de>")
	@$(call install_fixup, qt5,DESCRIPTION,missing)

#ifeq (,$(wildcard $(PTXCONF_SYSROOT_TARGET)/usr/bin/qmake))
#	@ln -s $(PTXCONF_SYSROOT_TARGET)/usr/bin/qt5/qmake $(PTXCONF_SYSROOT_TARGET)/usr/bin/qmake 
#endif

#ifeq (,$(wildcard $(PTXCONF_SYSROOT_CROSS)/bin/qmake))
#	@ln -s $(PTXCONF_SYSROOT_CROSS)/bin/qt5/qmake $(PTXCONF_SYSROOT_CROSS)/bin/qmake
#endif

#ifeq (,$(wildcard $(PTXCONF_SYSROOT_CROSS)/bin/qt.conf))
#	@ln -s $(PTXCONF_SYSROOT_CROSS)/bin/qt5/qt.conf $(PTXCONF_SYSROOT_CROSS)/bin/qt.conf
#endif

	@$(foreach lib, $(QT5_LIBS-y), \
		$(call install_lib, qt5, 0, 0, 0644, lib$(lib));)

ifdef PTXCONF_QT5_MODULE_QTWEBENGINE
	@$(call install_copy, qt5, 0, 0, 0755, -, \
		/usr/lib/qt5/libexec/QtWebEngineProcess)
	@$(call install_copy, qt5, 0, 0, 0644, -, \
		/usr/share/qt5/resources/icudtl.dat)
	@$(call install_copy, qt5, 0, 0, 0644, -, \
		/usr/share/qt5/resources/qtwebengine_devtools_resources.pak)
	@$(call install_copy, qt5, 0, 0, 0644, -, \
		/usr/share/qt5/resources/qtwebengine_resources.pak)
	@$(call install_copy, qt5, 0, 0, 0644, -, \
		/usr/share/qt5/resources/qtwebengine_resources_100p.pak)
	@$(call install_copy, qt5, 0, 0, 0644, -, \
		/usr/share/qt5/resources/qtwebengine_resources_200p.pak)
	@$(call install_alternative_tree, qt5, 0, 0,  /usr/share/qt5/translations)
endif

	@$(foreach plugin, $(QT5_PLUGINS-y), \
		$(call install_copy, qt5, 0, 0, 0644, -, \
			/usr/lib/qt5/plugins/$(plugin).so);)

	@$(foreach import, $(QT5_IMPORTS-y), \
		$(call install_tree, qt5, 0, 0, -, \
		/usr/lib/qt5/imports/$(import));)

	@$(foreach qml, $(QT5_QML-y), \
		$(call install_tree, qt5, 0, 0, -, \
		/usr/lib/qt5/qml/$(qml));)

ifdef PTXCONF_QT5_MODULE_QTDECLARATIVE_QMLSCENE
	@$(call install_copy, qt5, 0, 0, 0755, -, /usr/bin/qmlscene)
endif

	@$(call install_finish, qt5)

	@$(call touch)

# vim: syntax=make
