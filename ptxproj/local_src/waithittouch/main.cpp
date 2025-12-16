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
/// \file    main.cpp
///
/// \version $Id$
///
/// \brief   WaitHitTouch tool
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------

#include "mainwindow.h"
#include <QApplication>
#if QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
#include <QScreen>
#else
#include <QDesktopWidget>
#endif
#include <QDebug>

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
int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  if (QApplication::arguments().count() > 1)
  {
    QString sTxt = QApplication::arguments().at(1);
    if ((sTxt == "-h") || (sTxt == "--help"))
    {
        ShowHelpText();
        return -1;
    }
  }

  if (QApplication::arguments().count() < 3)
  {
    qDebug() << "not enough parameter";
    return -1;
  }

  foreach(QChar c, QApplication::arguments().at(2))
  {
    if(!c.isNumber())
    {
       qDebug() << "parameter 2 is not a number";
       return -1;
    }
  }

  MainWindow w;

  w.m_sText = QApplication::arguments().at(1);
  w.m_iNumber = qMin(QApplication::arguments().at(2).toInt(), 99);

  w.Initialize();

  w.show();
  
  a.setOverrideCursor(Qt::BlankCursor);
  a.exec();

  return w.m_iReturnValue;
}

/// \brief Print help text / usage to stdout
///
void ShowHelpText()
{
  qDebug("\n* waithittouch * \n");
  qDebug("Usage: waithittouch <text> <timeout [s]>");
  qDebug() << "";
}

