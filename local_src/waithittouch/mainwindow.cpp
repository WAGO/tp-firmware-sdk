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
/// \file    mainwindow.cpp
///
/// \version $Id: mainwindow.cpp 63357 2021-12-02 09:01:18Z wrueckl_elrest $
///
/// \brief   WaitHitTouch tool
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------

#include <QApplication>
#include <QMainWindow>
#include <QDesktopWidget>
#include <QSettings>
//#include <QProcess>
//#include <QX11Info>

#include "globals.h"
#include "tools.h"

#include "mainwindow.h"


//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

#define CONF_FILE     "/etc/specific/qtstyle.conf"

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
  //setWindowState(Qt::WindowFullScreen);
  //setWindowModality(Qt::ApplicationModal);
  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);

  m_iNumber = 10;
  m_iReturnValue = 0;

  //screen dimensions
  QRect r = QApplication::desktop()->screenGeometry();
  m_iScreenWidth = r.width();
  m_iScreenHeight = r.height();

  setGeometry(QRect(QPoint(0,0), QSize(m_iScreenWidth, m_iScreenHeight)));
}

MainWindow::~MainWindow()
{
  if (m_pTimer)
    delete (m_pTimer);

  if (m_pLcdNum)
    delete (m_pLcdNum);

  if (m_pLabel)
    delete (m_pLabel);

}

/// \brief init form data
///
void MainWindow::Initialize()
{
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

  QString sIdFormStyle = sPrefix + "form_style_waithittouch";

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
  }

  setStyleSheet(sQtStyleForm);

  m_pLabel = new QLabel(this);
  if (m_pLabel)
  {
    m_pLabel->setWordWrap(true);
    m_pLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_pLabel->setGeometry(QRect(QPoint(10, 10), QSize(m_iScreenWidth - 15, m_iScreenHeight/2)));
    m_pLabel->setText(m_sText);
  }

  m_pLcdNum = new QLCDNumber(this);
  if (m_pLcdNum)
  {
    m_pLcdNum->display(m_iNumber);
    m_pLcdNum->setDigitCount(2);

    int lcdwidth, lcdheight;
    lcdwidth = m_iScreenWidth / 3;
    lcdheight = m_iScreenHeight / 3;

    m_pLcdNum->setGeometry(QRect(QPoint((m_iScreenWidth - lcdwidth)/2, (m_iScreenHeight - lcdheight)*3/4), QSize(lcdwidth, lcdheight)));

  }

  //Set QTimer timeout to 1000 milliseconds (Update Number display each second)
  m_pTimer =new QTimer(this);
  if (m_pTimer)
  {
    QObject::connect(m_pTimer ,  SIGNAL(timeout ()),this,SLOT(setNumber()));
    m_pTimer->start(1000);
  }

  installEventFilter(this);

  setFocus();
}

/// \brief receive events f.e. mouse button pressed
///
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
  if (event->type() == QEvent::MouseButtonPress)
  {
    //QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
    m_iReturnValue = 1;
    QApplication::exit(m_iReturnValue);
  }
  return false;
}

/// \brief set LCD number on screen / countdown
///
void MainWindow::setNumber()
{
  if (m_pLcdNum)
  {
    m_pLcdNum->display(--m_iNumber);

    if (m_iNumber < 0)
        QApplication::exit(m_iReturnValue);
  }
};

void MainWindow::showEvent(QShowEvent *event)
{
  //DEPRECATED QTimer::singleShot(250, this, SLOT(ActivateX11Window()));
}

