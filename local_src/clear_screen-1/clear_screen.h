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
/// \file clear_screen.h
///
/// \version $Id: clear_screen.h 44064 2019-10-24 12:34:47Z wrueckl_elrest $
///
/// \brief touchscreen cleaning tool
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

#ifndef CLEAR_SCREEN_H
#define CLEAR_SCREEN_H

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------

#include <QMainWindow>
#include <QLCDNumber>
#include <QObject>
#include <QTimer>
//#include <QDebug>
#include <QEvent>
#include <QMouseEvent>


class ClearScreen: public QMainWindow
{
  Q_OBJECT

public:
  ClearScreen():number(10)
  {
      m_pLcd = NULL;
      m_pTimer = NULL;
      m_pCentralWidget = NULL;
  };

  ~ ClearScreen()
  {
    if (m_pLcd)
      delete (m_pLcd);

    if (m_pTimer)
      delete (m_pTimer);

    if (m_pCentralWidget)
      delete (m_pCentralWidget);
  };

  int ReadTimeoutFromConfigTool();

  void decorate()
  {
    m_pCentralWidget = new QWidget(this);
    if (m_pCentralWidget)
    {
      m_pLcd = new QLCDNumber(m_pCentralWidget);
      if (m_pLcd)
      {
        m_pLcd->setDigitCount(2);
        m_pLcd->setFixedSize(m_iScreenWidth, m_iScreenHeight);
        m_pLcd->display(number);
      }

      setCentralWidget(m_pCentralWidget);
    }

    installEventFilter(this);

    //Set QTimer timeout to 1000 milliseconds (Update Number display each second)
    m_pTimer = new QTimer(this);
    if (m_pTimer)
    {
      QObject::connect(m_pTimer ,  SIGNAL(timeout ()),this,SLOT(setNumber()));
      m_pTimer->start(1000);
    }

  };

  bool eventFilter(QObject *obj, QEvent *event)
  {
    if (event->type() == QEvent::MouseButtonPress)
    {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
      //emit finished();
    }
    return false;
  }

  void setCount(int iValue)
  {
    number = iValue;
    return;
  }

  public slots:
    void ActivateX11Window();

    void setNumber()
    {
      number--;
      if(number < 0)
      {
        if (m_pTimer)
          m_pTimer->stop();

        emit finished();
      }
      else
      {
        if (m_pLcd)
        {
          m_pLcd->display(number);
          //qDebug() << number;
        }
      }
    };

 public:

    QLCDNumber * m_pLcd;
    QTimer * m_pTimer;
    QWidget * m_pCentralWidget;
    int number;

    int m_iScreenWidth;
    int m_iScreenHeight;

  protected:
    void showEvent(QShowEvent *event);

  signals:
    void finished();

};
#endif // CLEAR_SCREEN_H
