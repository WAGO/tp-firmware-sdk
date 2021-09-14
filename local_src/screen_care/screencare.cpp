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
/// \file    screencare.cpp
///
/// \version $Id: screencare.cpp 54497 2020-12-14 14:27:11Z wrueckl_elrest $
///
/// \brief   Application main window implementation
///
/// \author  Nicoleta Nething, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

#include "screencare.h"
#include <QDebug>
#include <QPainter>
#include <QTime>
#include <QPaintEvent>
#include <QTimer>
#include <QProcess>
#include <QX11Info>

ScreenCare::ScreenCare(long l_numberOfColorChanges, QWidget *parent) :
    QMainWindow(parent)
{
  idxNumberOfColorChanges = 0;
  numberOfColorChanges = l_numberOfColorChanges;
  bXdoDone = false;
  iCounter = 0;
}

ScreenCare::~ ScreenCare()
{
}

/// \brief on paint 
/// \param[in]  event pointer to QPaintEvent
void ScreenCare::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);

  //qDebug() << "start: " << QTime::currentTime().toString("hh:mm:ss.zzz") << ": (" << event->rect().left() << ", " << event->rect().top() << ", " << event->rect().right() << ", " << event->rect().bottom() << ")";

  if (idxNumberOfColorChanges % 2 == 0)
  {
      painter.fillRect(event->rect(), Qt::white);
  }
  else
  {
      painter.fillRect(event->rect(), Qt::black);
  }

  //qDebug() << "stop: " << QTime::currentTime().toString("hh:mm:ss.zzz");

  idxNumberOfColorChanges++;
  if (idxNumberOfColorChanges >=numberOfColorChanges)
  {
      //200ms black
      QTimer::singleShot(200, this, SLOT(OnCloseScreenCare()));
  }
  else
  {
      //200ms white
      QTimer::singleShot(200, this, SLOT(update()));
  }

}

void ScreenCare::showEvent(QShowEvent *event)
{
  QTimer::singleShot(20, this, SLOT(ActivateX11Window()));
}

/// \brief ensure X11 window to be shown in front
/// \param[in]  X11 window id
void ScreenCare::ActivateX11Window()
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
  bXdoDone = true;
}

void ScreenCare::OnCloseScreenCare()
{
  hide();
  if (bXdoDone)
  {
    close();
  }
  else
  {
    iCounter++;
    if (iCounter > 10)
    {
      //timeout
      close();
    }
    else
    {
      qDebug() << "RETRY";
      QTimer::singleShot(1000, this, SLOT(OnCloseScreenCare()));
    }
  }
}
