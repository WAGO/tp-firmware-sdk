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
/// \file clear_screen.cpp
///
/// \version $Id: clear_screen.cpp 44064 2019-10-24 12:34:47Z wrueckl_elrest $
///
/// \brief touchscreen cleaning tool
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------
#include <QWidget>
#include <QMainWindow>
#include <QApplication>
#include <QDesktopWidget>
#include <QRect>
#include <QSettings>
#include <QProcess>
#include <QX11Info>
#include <QDebug>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "globals.h"
#include "tools.h"
#include "clear_screen.h"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

#define CONF_FILE            "/etc/specific/qtstyle.conf"
#define CONF_FILE_DISPLAY    "/etc/specific/display.conf"
#define DEV_VIRTUALKEYBOARD  "/dev/virtualkeyboard"

#ifndef ERROR
#define ERROR   -1
#endif

#ifndef SUCCESS
#define SUCCESS   0
#endif

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

/// \brief check if file exists
/// \param[in]  pFilename absolute file name
/// \retval -1 ERROR
/// \retval 0 SUCCESS
int fileexists(char * pFilename)
// Check if file is available
{
  struct stat sts;
  if (stat(pFilename, &sts) == -1 && errno == ENOENT)
  {
    return ERROR;
  }
  else
  {   
    return SUCCESS;
  }
}

/// \brief main function
/// \param[in]  argc number of arguments
/// \param[in]  argv arguments
/// \retval 0  SUCCESS
int main(int argc, char **argv)
{
  int iTimeOut = 15;
  QApplication app(argc, argv);

  //close virtualkeyboard 
  if (fileexists(DEV_VIRTUALKEYBOARD) == SUCCESS)
  {
    int fd = open(DEV_VIRTUALKEYBOARD, O_RDWR | O_NONBLOCK);
    if (fd >= 0)
    {
      char * pCmd = "close\n";
      write(fd, pCmd, strlen(pCmd));
      close(fd);
    }
  }
  
  //screen width and height
  QRect rDesk = QApplication::desktop()->screenGeometry();


  QString sQtStyleForm = DEFAULT_FORM_STYLE;

  DISPLAY_SIZE displaySize = GetDisplaySize();
  QString sPrefix = "small_";
  if (displaySize == DISPLAY_MEDIUM)
  {
    sPrefix = "medium_";
  }
  else if (displaySize == DISPLAY_BIG)
  {
    sPrefix = "big_";
  }

  QString sIdFormStyle = sPrefix + "form_style_clean";

  QSettings settings(CONF_FILE, QSettings::IniFormat);
  const QStringList allKeys = settings.allKeys();
  foreach (const QString &key, allKeys)
  {
      QString var = settings.value(key).toString();

      if (var.length() > 0)
      {
        if (key == sIdFormStyle)
        {
          sQtStyleForm = var;
        }

      }
      //qDebug() << "webkit: " << key << "  " << var;
  }  

  ClearScreen *window = new ClearScreen();
  if (window)
  {
      iTimeOut = window->ReadTimeoutFromConfigTool();

      //qDebug() << "ClearScreen: " << iTimeOut;

      window->setStyleSheet(sQtStyleForm);
      window->m_iScreenWidth = rDesk.width();
      window->m_iScreenHeight = rDesk.height();
      //qDebug("width %d height %d\n",window->m_iScreenWidth, window->m_iScreenHeight );

      window->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
      window->setWindowTitle(QString::fromUtf8("MainWindow"));

      QObject::connect(window, SIGNAL(finished()), &app, SLOT(quit()));
      window->setCount(iTimeOut);
      window->setFocusPolicy(Qt::ClickFocus);
      window->decorate();

      window->resize(window->m_iScreenWidth,window->m_iScreenHeight);
      window->setFocus();
      window->show();

      app.setOverrideCursor(Qt::BlankCursor);
      app.exec();

      delete (window);

      return (0);
  }

  return -1;
}

int ClearScreen::ReadTimeoutFromConfigTool()
{
  int iRet = 15;

  QSettings settings(CONF_FILE_DISPLAY, QSettings::IniFormat);
  const QStringList allKeys = settings.allKeys();
  foreach (const QString &key, allKeys)
  {
    QString var = settings.value(key).toString();

    if (var.length() > 0)
    {
      if (key == "cleaningtimeout")
      {
        iRet = var.toInt();
        break;
      }
    }
  }

  return iRet;
}

void ClearScreen::showEvent(QShowEvent *event)
{
  QTimer::singleShot(250, this, SLOT(ActivateX11Window()));
}

/// \brief ensure X11 window to be shown in front
/// \param[in]  X11 window id
void ClearScreen::ActivateX11Window()
{
  if (QX11Info::isPlatformX11())
  {
    int id = QWidget::winId ();
    if (id > 0)
    {
      QProcess proc;
      proc.start("/usr/bin/xdotool", QStringList() << "windowactivate" << QString::number(id) );
      proc.waitForFinished(5000);
      proc.close();
      //qDebug() << "ActivateX11Window: " << id;
    }
  }
}



