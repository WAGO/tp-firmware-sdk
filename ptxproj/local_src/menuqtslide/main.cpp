//------------------------------------------------------------------------------
/// Copyright (c) 2000 - 2022 WAGO GmbH & Co. KG
///
/// PROPRIETARY RIGHTS of WAGO GmbH & Co. KG are involved in
/// the subject matter of this material. All manufacturing, reproduction,
/// use, and sales rights pertaining to this subject matter are governed
/// by the license agreement. The recipient of this software implicitly
/// accepts the terms of the license.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/// \file main.cpp
///
/// \version $Id: main.cpp 65689 2022-03-11 14:37:43Z falk.werner@wago.com $
///
/// \brief Thread for receiving command f.e. open menu
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
//------------------------------------------------------------------------------

#include <signal.h>
#include <unistd.h>

#include "transparentwidget.h"
#include "toolbarmenu.h"
#include "globals.h"

#include <QApplication>
#include <QDebug>

TransparentWidget * g_pWidget = NULL;

void signalhandler(int sig)
{
  if (sig == SIGINT)
  {
    qDebug() << "will quit by SIGINT";
    if (g_pWidget)
    {
      g_pWidget->close();
    }
  }
  else if (sig == SIGTERM)
  {
    qDebug() << "will quit by SIGTERM";
    if (g_pWidget)
    {
      g_pWidget->close();
    }
  }
}


int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  
  g_bEventDetection = true;
  g_bWbmAllowed = true;
  //parse cmdline arguments
  int i=1;
  while (i < QApplication::arguments().count())
  {
    QString sTxt = QApplication::arguments().at(i);
    if (QString::compare(sTxt, "-nodetection", Qt::CaseInsensitive)==0)
    {      
      //no event detection in case of resisive panels f.e.
      //write "open" to /dev/toolbarmenu in this case from external application
      //slide mode disabled here
      g_bEventDetection = false;
    }
    else if (QString::compare(sTxt, "-nowbm", Qt::CaseInsensitive)==0)
    {
      //user is not allowed to show WBM not even login dialog
      g_bWbmAllowed = false;
    }
    i++;
  }
  
  //MainWindow w;
  TransparentWidget w;
  //ToolBarMenu w;

  signal(SIGINT,signalhandler);
  signal(SIGTERM,signalhandler);

  g_pWidget = &w;

  if (g_bEventDetection)
  {
    w.show();
  }

  return a.exec();
}
