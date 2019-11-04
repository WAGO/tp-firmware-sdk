//------------------------------------------------------------------------------
/// Copyright (c) 2000 - 2019 WAGO Kontakttechnik GmbH & Co. KG
///
/// PROPRIETARY RIGHTS of WAGO Kontakttechnik GmbH & Co. KG are involved in
/// the subject matter of this material. All manufacturing, reproduction,
/// use, and sales rights pertaining to this subject matter are governed
/// by the license agreement. The recipient of this software implicitly
/// accepts the terms of the license.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/// \file main.cpp
///
/// \version $Id: main.cpp 44064 2019-10-24 12:34:47Z wrueckl_elrest $
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
