TEMPLATE = app
TARGET = webenginebrowser
QT += core gui dbus webenginewidgets network widgets printsupport x11extras
CONFIG += c++11

LIBS += -lmsgtool -lm

#DEFINES += QWEBENGINEINSPECTOR

#test
#CONFIG += static
#static {
#    QTPLUGIN += qtvirtualkeyboardplugin
#    QT += svg
#}

qtHaveModule(uitools):!embedded: QT += uitools
else: DEFINES += QT_NO_UITOOLS

FORMS += \
    addbookmarkdialog.ui \
    bookmarks.ui \
    cookies.ui \
    cookiesexceptions.ui \
    downloaditem.ui \
    downloads.ui \
    history.ui \
    passworddialog.ui \
    printtopdfdialog.ui \
    proxy.ui \
    savepagedialog.ui \
    settings.ui

HEADERS += \
    autosaver.h \
    bookmarks.h \
    browserapplication.h \
    browsermainwindow.h \
    chasewidget.h \
    downloadmanager.h \
    edittableview.h \
    edittreeview.h \
    featurepermissionbar.h\
    history.h \
    modelmenu.h \
    printtopdfdialog.h \
    savepagedialog.h \
    searchlineedit.h \
    settings.h \
    squeezelabel.h \
    tabwidget.h \
    toolbarsearch.h \
    urllineedit.h \
    webview.h \
    xbel.h \
    cmdthread.h \
    alarmindication.h \
    globals.h \
    tools.h \
    CustomSwipeGesture.h

SOURCES += \
    autosaver.cpp \
    bookmarks.cpp \
    browserapplication.cpp \
    browsermainwindow.cpp \
    chasewidget.cpp \
    downloadmanager.cpp \
    edittableview.cpp \
    edittreeview.cpp \
    featurepermissionbar.cpp\
    history.cpp \
    modelmenu.cpp \
    printtopdfdialog.cpp \
    savepagedialog.cpp \
    searchlineedit.cpp \
    settings.cpp \
    squeezelabel.cpp \
    tabwidget.cpp \
    toolbarsearch.cpp \
    urllineedit.cpp \
    webview.cpp \
    xbel.cpp \
    main.cpp \
    cmdthread.cpp \
    alarmindication.cpp \
    globals.cpp \
    tools.cpp \
    CustomSwipeGesture.cpp

RESOURCES += data/data.qrc htmls/htmls.qrc

#build_all:!build_pass {
#    CONFIG -= build_all
#    CONFIG += release
#}

#win32 {
#   RC_FILE = demobrowser.rc
#}

#mac {
#    ICON = demobrowser.icns
#    QMAKE_INFO_PLIST = Info_mac.plist
#    TARGET = Demobrowser
#}

EXAMPLE_FILES = \
    Info_mac.plist demobrowser.icns demobrowser.ico demobrowser.rc \
    cookiejar.h cookiejar.cpp  # FIXME: these are currently unused.

# install
#target.path = $$[QT_INSTALL_EXAMPLES]/webenginewidgets/demobrowser
#INSTALLS += target

include(install.pri)


