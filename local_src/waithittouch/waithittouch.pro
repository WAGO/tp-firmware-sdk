#-------------------------------------------------
#
# Project created by QtCreator 2014-02-19T13:31:46
#
#-------------------------------------------------

include(../../sysroot-target/usr/lib/qt5/mkspecs/linux-ptx-g++/qmake.conf)

QT       += core gui x11extras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = waithittouch
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    tools.cpp

include(install.pri)

HEADERS  += mainwindow.h \
    globals.h \
    tools.h

