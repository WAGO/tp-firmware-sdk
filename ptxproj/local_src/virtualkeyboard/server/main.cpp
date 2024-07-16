///------------------------------------------------------------------------------
/// \file    main.cpp
///
/// \brief   platform input context
///
/// \author  WRü: elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include <QApplication>
#include <QDBusConnection>
#include <QPluginLoader>

#include <QDebug>

#include "keyboard.h"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// variable definitions
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

#ifdef DEBUG_MSG
    foreach (const QString &path, app.libraryPaths())
        qDebug() <<  "LPath: " << path;
#endif

    if (!QDBusConnection::sessionBus().isConnected())
    {
        qFatal("SERVER: Cannot connect to the D-Bus session bus. To start it, run: eval `dbus-launch --auto-syntax`");
        return 1;
    }

    if (!QDBusConnection::sessionBus().registerService("com.kdab.inputmethod"))
    {
        qFatal("SERVER:  Unable to register at DBus - com.kdab.inputmethod");
        return 1;
    }

    Keyboard * pKeyboard = new Keyboard();
    if (pKeyboard == NULL)
    {
        qFatal("SERVER:  Create Keyboard instance failed.");
        return 1;
    }

    if (!QDBusConnection::sessionBus().registerObject("/VirtualKeyboard", pKeyboard,
            QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllSlots))
    {
        qFatal("SERVER:  Unable to register object at DBus - /VirtualKeyboard");
        delete pKeyboard;
        return 1;
    }

#ifdef DEBUG_MSG
    qDebug() << "SERVER: virtual keyboard started";
#endif

    int rc = app.exec();
    // clean up
    if (pKeyboard)
        delete pKeyboard;
    return rc;
}


/*

# start the keyboard UI application
./server

# set the environment variable
export QT_IM_MODULE=vkim

# run any Qt application
designer

*/
