//------------------------------------------------------------------------------
// Copyright 2019 WAGO Kontakttechnik GmbH & Co. KG
//
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
///
/// \file    globals.h
///
/// \version $Id: globals.h 43460 2019-10-09 13:25:56Z wrueckl_elrest $
///
/// \brief   global definitions
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

#ifndef GLOBALS_H
#define GLOBALS_H

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <QString>
#include <QWebEngineHistory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
#define CONF_FILE_WEBENGINE          "/etc/specific/webengine/webengine.conf"
#define CONF_FILE_GESTURE            "/etc/specific/gesture.conf"
#define CONF_FILE_PLCSELECT          "/etc/specific/plcselect.conf"
#define CONF_FILE_DISPLAY            "/etc/specific/display.conf"
#define CONF_FILE_STYLE              "/etc/specific/qtstyle.conf"

#define CONF_FILE_SECURITY           "/etc/specific/browsersecurity.conf"
#define CONF_FILE_SSLALLOWED         "/etc/specific/sslallowed.conf"

#define DEFAULT_FORM_STYLE        "color:#c00000; background-color:white; font-size=16; font-family=Helvetica"

#define FILE_TMP_BROWSER_URL         "/tmp/webenginebrowser_url.txt"

#ifndef ERROR
#define ERROR -1
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

//show debug messages
//#define DEBUG_MSG

typedef struct WEBENGINE_DEFAULTS
{
  bool bMenubar;
  bool bNavbar;
  bool bBookmarkbar;
  bool bTabbar;
  bool bReconnect;
  bool bZoom;                 //zoom gesture
  int iReconnectInterval;     //seconds
  int  iTabIndex;
  int iPlcSelected;           //index of selected URL
  QStringList slUrls;
  QStringList slTitles;
  QStringList slMonitor;
  QStringList slVirtualKeyboard;
  QString sJsScriptFilename;  //script injection
  QStringList slError5Text;
  QString sConnectedText;
  QString sDisconnectedText;

} tWEBENGINE_DEFAULTS;


enum DISPLAY_SIZE { DISPLAY_SMALL, DISPLAY_MEDIUM, DISPLAY_BIG };
enum DISPLAY_ORIENTATION { DISPLAY_LANDSCAPE, DISPLAY_PORTRAIT };
enum BROWSER_SECURITY_LEVEL { BROWSER_SEC_LOW, BROWSER_SEC_HIGH };

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variable definitions
//------------------------------------------------------------------------------
extern QString g_sUrlPLCList;
extern bool g_bMouseAvailable;
extern QWebEngineHistory * g_pHistory;
extern tWEBENGINE_DEFAULTS g_webengine;

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------


#endif // GLOBALS_H
