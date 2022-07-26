//------------------------------------------------------------------------------
// Copyright (c) 2019-2022 WAGO GmbH & Co. KG
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
/// \file    globals.h
///
/// \version $Id: globals.h 65689 2022-03-11 14:37:43Z falk.werner@wago.com $
///
/// \brief   global settings / variables
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

#ifndef GLOBALS_H
#define GLOBALS_H

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <QString>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
#define CONF_FILE_GESTURE            "/etc/specific/gesture.conf"
//#define CONF_FILE_PLCSELECT          "/etc/specific/plcselect.conf"
//#define CONF_FILE_DISPLAY            "/etc/specific/display.conf"
#define CONF_FILE_STYLE              "/etc/specific/qtstyle.conf"
//#define CONF_FILE_SECURITY           "/etc/specific/browsersecurity.conf"
//#define CONF_FILE_SYSTEM             "/etc/specific/system.conf"
#define CONF_FILE_WEBENGINE          "/etc/specific/webengine/webengine.conf"

#define DEFAULT_FORM_STYLE        "color:#004494; background-color:white; font-size=16; font-family=Helvetica"

#ifndef ERROR
#define ERROR -1
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

//show debug messages
//#define DEBUG_MSG

enum DISPLAY_SIZE { DISPLAY_SMALL, DISPLAY_MEDIUM, DISPLAY_BIG };
enum DISPLAY_ORIENTATION { DISPLAY_LANDSCAPE, DISPLAY_PORTRAIT };
enum BROWSER_SECURITY_LEVEL { BROWSER_SEC_LOW, BROWSER_SEC_HIGH };

enum TOOLBAR_ORIENTATION { TOOLBAR_TOP, TOOLBAR_BOTTOM, TOOLBAR_LEFT, TOOLBAR_RIGHT };

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// variable definitions
//------------------------------------------------------------------------------
extern bool g_bEventDetection;
extern bool g_bWbmAllowed;

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------


#endif // GLOBALS_H
