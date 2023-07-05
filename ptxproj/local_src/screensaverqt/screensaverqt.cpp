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
/// \file    screensaverqt.cpp
///
/// \version $Id$
///
/// \brief   screensaver Qt tool
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------

#include "screensaver.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QRect>
#include <QDebug>
#include <QSharedMemory>


//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

void ShowHelpText();

/// \brief main function
/// \param[in]  argc number of arguments
/// \param[in]  argv arguments
/// \retval 0  SUCCESS
int main(int argc, char **argv)
{
  QApplication a(argc, argv);

/*
  //check if screensaver is already running
  QSharedMemory sharedMemory;
  sharedMemory.setKey("01234567890");
  if(sharedMemory.attach()) {
      qDebug("program is already running");
      return -1;
  }

  if (!sharedMemory.create(1)) {
      qDebug("program is already running");
      return -1;
  }
*/

  ScreenSaver w;

  if (QApplication::arguments().count() > 1)
  {
    QString sTxt = QApplication::arguments().at(1);
    if ((sTxt == "-h") || (sTxt == "--help"))
    {
      ShowHelpText();
      return -1;
    }
    else
    {
      int count = QApplication::arguments().count();
      w.m_iThemeId = sTxt.toInt();
      if (count > 2)
      {
         for (int i=2; i < count; i++)
         {
           if (w.m_sParameter.length() > 0) w.m_sParameter += " ";
           w.m_sParameter += QApplication::arguments().at(i);
         }
      }
    }
  }

  //qDebug() << w.m_sParameter;

  w.Initialize();
  w.show();

  a.setOverrideCursor(Qt::BlankCursor);
  return a.exec();
}

/// \brief show help text
///
void ShowHelpText()
{
  qDebug("\n* screensaverqt * \n");
  qDebug("Usage: screensaverqt <themeID> <string>");
  qDebug("themeID: 0=backlight, 1=image, 2=time, 3=text");
  qDebug("string ID 1: image filename with complete path");
  qDebug("string ID 3: text to be shown");
}




