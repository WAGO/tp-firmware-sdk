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
/// \file    screen_care.cpp
///
/// \version $Id: screen_care.cpp 54497 2020-12-14 14:27:11Z wrueckl_elrest $
///
/// \brief   main file
///
/// \author  Nicoleta Nething, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

#include <stdio.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QTimer>
#include <QDebug>
#include <QTime>
#include "screencare.h"

/// \brief main function
/// \param[in]  argc number of arguments
/// \param[in]  argv arguments
/// \retval 0  SUCCESS
int main(int argc, char **argv)
{
  bool bDebug = false;

  QApplication app(argc, argv);

  //screen width and height
  QRect rDesk = QApplication::desktop()->screenGeometry();

  if (argc > 0)
  {
      ScreenCare *window;
      long numberOfColorChanges = NROF_COLORCHANGES;

      for (int j = 0; j < argc; j++)
      {
          if (QString(argv[j])==QString("-n"))
          {
              j = j+1;
              if (j < argc)
              {
                  numberOfColorChanges = QString(argv[j]).toLong();
              }
          }
           else if (QString(argv[j])==QString("-d"))
              bDebug = true;
      }

      window = new ScreenCare(numberOfColorChanges);
      if (window)
      {
          window->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
          window->resize(rDesk.width(),rDesk.height());

          if (bDebug)
          {
              qDebug() << "Start -n " << window->numberOfColorChanges << "=>" << QTime::currentTime().toString("hh:mm:ss.zzz");
          }

          window->show();
          
          app.setOverrideCursor(Qt::BlankCursor);
          app.exec();

          delete (window);

      }
      if (bDebug)
      {
          qDebug() << "Exit: " << QTime::currentTime().toString("hh:mm:ss.zzz");;
      }

      return (0);
  }

  return -1;
}

