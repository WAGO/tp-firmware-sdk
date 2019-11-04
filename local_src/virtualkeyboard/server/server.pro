
QT += core gui widgets dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = virtualkeyboard
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

#Q_IMPORT_PLUGIN(vkimplatforminputcontextplugin)


INCLUDEPATH += .

HEADERS += keyboard.h \
    virtualkeyb.h \
    cmdthread.h
SOURCES += keyboard.cpp main.cpp \
    virtualkeyb.cpp \
    cmdthread.cpp

include(install.pri)
