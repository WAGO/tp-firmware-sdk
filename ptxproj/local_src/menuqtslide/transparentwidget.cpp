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
/// \file    transparentwidget.cpp
///
/// \version $Id$
///
/// \brief   Thread for receiving command f.e. open menu
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "globals.h"
#include "transparentwidget.h"
#include <QResizeEvent>
#include <QWidget>
#if QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
#include <QScreen>
#else
#include <QDesktopWidget>
#endif

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

#define QT_TRANSPARENT_STYLE    "QWidget{background:transparent;background-color:transparent;}"
//#define QT_TRANSPARENT_STYLE    "background:transparent;background-color:rgba(255,255,255,0);"


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// variable definitions
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------


TransparentWidget::TransparentWidget(QWidget *parent) : QWidget(parent)
{
  if (m_toolBar.m_iBtnCount == 0)
  {
    //qDebug() << "debug: exit no buttons";
    QTimer::singleShot(1000, this, SLOT(slotOnClose()));
    return;
  }

  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

  //widget always on top and transparent
  if (g_bEventDetection)
  {
    //slide mode (kapazitiv)
    SetTransparent();   
  }
  else
  {
    //cmdlineoption -nodetection
    //no slide mode (resistiv) - gesture_control sends an open cmd to /dev/toolbarmenu
    //start listener thread to /dev/toolbarmenu
    m_toolBar.startCmdThread();
  }
  
#if QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
  QRect rScreen = QApplication::primaryScreen()->geometry();
#else
  QRect rScreen = QApplication::desktop()->screenGeometry();
#endif

  if (m_toolBar.m_menu.orientation == TOOLBAR_BOTTOM)
  {
    //m_WndRect.setY(rScreen.height() - wndHeight);

    m_WndRect.setTopLeft(QPoint(0, rScreen.height() - m_toolBar.m_iInitialWndHeight));
    m_WndRect.setHeight(m_toolBar.m_iInitialWndHeight);
    m_WndRect.setWidth(rScreen.width());

  }
  else if (m_toolBar.m_menu.orientation == TOOLBAR_LEFT)
  {
    m_WndRect.setTopLeft(QPoint(0,0));
    m_WndRect.setHeight(rScreen.height());
    m_WndRect.setWidth(m_toolBar.m_iInitialWndHeight);
  }
  else if (m_toolBar.m_menu.orientation == TOOLBAR_RIGHT)
  {
    m_WndRect.setTopLeft(QPoint(rScreen.width() - m_toolBar.m_iInitialWndHeight, 0));
    m_WndRect.setHeight(rScreen.height());
    m_WndRect.setWidth(m_toolBar.m_iInitialWndHeight);
  }
  else
  {
    //TOOLBAR_TOP
    m_WndRect.setTopLeft(QPoint(0,0));
    m_WndRect.setHeight(m_toolBar.m_iInitialWndHeight);
    m_WndRect.setWidth(rScreen.width());
  }

  //m_bInProgress = false;

  m_LastEnterEvent = QDateTime::currentDateTime();
  //m_LastLeaveEvent = QDateTime::currentDateTime();
  
  //set to minimized pixel height of f.e. 5 transparent pixels
  ResetWidgetSize();

  m_pTimer = new QTimer(this);
  if (m_pTimer)
  {
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(slotOnTimer()));
    //only TEST m_pTimer->start(2000);
  }

  connect(&m_toolBar, SIGNAL(signalMinimize()), this, SLOT(slotMinimize()));
  connect(&m_toolBar, SIGNAL(signalFocus()), this, SLOT(slotFocus()));

}

TransparentWidget::~TransparentWidget()
{
  if (m_pTimer)
  {
    m_pTimer->stop();
    delete m_pTimer;
  }
}

void TransparentWidget::ResetWidgetSize()
{
  bool bMinimize = false;
  if ((m_toolBar.m_menu.orientation == TOOLBAR_LEFT) || (m_toolBar.m_menu.orientation == TOOLBAR_RIGHT))
  {
    if (width() != m_toolBar.m_iInitialWndHeight)
      bMinimize = true;
  }
  else
  {
    if (height() != m_toolBar.m_iInitialWndHeight)
      bMinimize = true;
  }

  if (bMinimize)
  {
    QRect r = m_WndRect;

    if ((m_toolBar.m_menu.orientation == TOOLBAR_LEFT) || (m_toolBar.m_menu.orientation == TOOLBAR_RIGHT))
    {
      r.setWidth(m_toolBar.m_iInitialWndHeight);
    }
    else
    {
      r.setHeight(m_toolBar.m_iInitialWndHeight);
    }

    setGeometry(r);
    //m_bInProgress = false;
  }
     
  //hold transparent window workable
  if (g_bEventDetection == true)
  {
    show();  
  }
  else
  {
    hide();
  }
}

/*
void TransparentWidget::paintEvent(QPaintEvent *) {
  //QColor backgroundColor = QColor(0,0,0,0);
  QColor backgroundColor = Qt::transparent;
  //backgroundColor.setAlpha(255);
  QPainter customPainter(this);
  customPainter.fillRect(rect(),backgroundColor);
}
*/

/// \brief Transparency
///
void TransparentWidget::SetTransparent()
{ 
  /* only testing some settings
  QPalette pal;
  pal.setColor(QPalette::Background, Qt::transparent);    
  setPalette(pal);
  
  //setAutoFillBackground(false);  
  //setWindowOpacity(0.0000);
  //setAttribute(Qt::WA_NoSystemBackground, true);  
  //setAttribute(Qt::WA_TranslucentBackground, true);  
   
  //setAttribute(Qt::WA_PaintOnScreen); // not needed in Qt 5.2 and up 
  //setAttribute(Qt::WA_OpaquePaintEvent, false);   
  //setAttribute(Qt::WA_NoSystemBackground, true);  
  
  //setAttribute(Qt::WA_PaintOnScreen, true);
  
  //To load values from Qt Stylesheet you should call this methods
  this->setStyleSheet(QT_TRANSPARENT_STYLE);
  this->style()->unpolish(this);
  this->style()->polish(this);        
  update();
  */
  
  //this code works on PC / Linux VM but not on the panel (background is black instead of transparent)
  setPalette(Qt::transparent);
  setAttribute( Qt::WA_TranslucentBackground, true );
  setAttribute( Qt::WA_OpaquePaintEvent, true );
  setAutoFillBackground(false);
  setStyleSheet("QWidget{background-color: transparent;}");
  setAttribute(Qt::WA_NoSystemBackground);
    
  this->style()->unpolish(this);
  this->style()->polish(this);        
  update();
}

/// \brief receive events
///
bool TransparentWidget::event(QEvent *e)
{
  //qDebug() << "Transparent: " << e ;

  switch (e->type())
  {
  case QEvent::MouseButtonPress:
  case QEvent::Enter:

    m_LastEnterEvent = QDateTime::currentDateTime();
    break;
  case QEvent::Leave:
  case QEvent::MouseMove:
  {
    unsigned int msecs = m_LastEnterEvent.time().msecsTo(QDateTime::currentDateTime().time());
    if (msecs < 300)
    {
      if (m_toolBar.isVisible() == false)
      {
        m_toolBar.Show();
      }
    }
  }
  break;

  default:
    break;
  }


  return QWidget::event(e);
}

void TransparentWidget::slotMinimize()
{
  if (g_bEventDetection)
    ResetWidgetSize();
}

void TransparentWidget::slotFocus()
{
  if (g_bEventDetection)
    setFocus();
}

void TransparentWidget::slotOnTimer()
{
  qDebug() << "slotOnTimer";
  //only TEST this->activateWindow();
}

void TransparentWidget::slotOnClose()
{
  close();
}
