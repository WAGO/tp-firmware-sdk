#-------------------------------------------------
#
# Project created by QtCreator 2014-02-12T09:29:21
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets x11extras

TARGET = dialogbox
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    tools.cpp

include(install.pri)

HEADERS  += mainwindow.h \
    globals.h \
    tools.h

FORMS    += mainwindow.ui
