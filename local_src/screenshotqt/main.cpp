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
/// \file    main.cpp
///
/// \version $Id: main.cpp 43460 2019-10-09 13:25:56Z wrueckl_elrest $
///
/// \brief   make screenshots due to testability
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

#include <signal.h>
#include <unistd.h>

#include "screenshot.h"

#include <QApplication>
#include <QDebug>

Screenshot * g_pScreen = NULL;

void signalhandler(int sig)
{
  if (sig == SIGINT)
  {
    qDebug() << "will quit by SIGINT";
    if (g_pScreen)
    {
      g_pScreen->pCmdThread->isRunning = 0;
      g_pScreen->close();
    }
  }
  else if (sig == SIGTERM)
  {
    qDebug() << "will quit by SIGTERM";
    if (g_pScreen)
    {
      g_pScreen->pCmdThread->isRunning = 0;
      g_pScreen->close();
    }
  }
}


int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  //MainWindow w;
  Screenshot screen;

  signal(SIGINT,signalhandler);
  signal(SIGTERM,signalhandler);

  g_pScreen = &screen;

  //w.Show();
  screen.hide();

  return a.exec();
}
