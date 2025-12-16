TEMPLATE = app
TARGET = waithittouch
DEPENDPATH += .
INCLUDEPATH += .

QT += core gui widgets

SOURCES += main.cpp\
        mainwindow.cpp \
    tools.cpp

include(install.pri)

HEADERS  += mainwindow.h \
    globals.h \
    tools.h
