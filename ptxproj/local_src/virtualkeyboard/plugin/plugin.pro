#########
# Qt 5.8
#########

QT += dbus gui-private widgets
QT += quick quickwidgets
QT += webengine
QT += webenginewidgets

LIBS += -lmsgtool

TARGET = vkimplatforminputcontextplugin

DEFINES += QT_DEPRECATED_WARNINGS

QTDIR_build {
PLUGIN_TYPE = platforminputcontexts
PLUGIN_CLASS_NAME = QVkImPlatformInputContextPlugin
PLUGIN_EXTENDS = -
load(qt_plugin)
CONFIG += install_ok

TEMPLATE = lib
CONFIG += plugin
#target.path += $$[QT_INSTALL_PLUGINS]/platforminputcontexts
#INSTALLS += target

}
else
{
TEMPLATE = lib
CONFIG += plugin
#target.path += $$[QT_INSTALL_PLUGINS]/platforminputcontexts
#INSTALLS += target
}

SOURCES += qvkimplatforminputcontext.cpp \
           main.cpp

HEADERS += qvkimplatforminputcontext.h

OTHER_FILES += vkim.json 

include(install.pri)
