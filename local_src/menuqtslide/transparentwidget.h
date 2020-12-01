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
/// \file    transparentwidget.h
///
/// \version $Id: transparentwidget.h 46481 2020-02-11 12:33:17Z wrueckl_elrest $
///
/// \brief   Thread for receiving command f.e. open menu
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------



#ifndef TRANSPARENTWIDGET_H
#define TRANSPARENTWIDGET_H

#include "toolbarmenu.h"

#include <QWidget>
#include <QPainter>
#include <QDateTime>

class TransparentWidget : public QWidget
{
  Q_OBJECT
public:
  explicit TransparentWidget(QWidget *parent = 0);
  ~TransparentWidget();

private:
  QRect m_WndRect;
  ToolBarMenu m_toolBar;
  //bool m_bInProgress;

  QDateTime m_LastEnterEvent;
  //QDateTime m_LastLeaveEvent;

  void ResetWidgetSize();
  void SetTransparent();

  QTimer * m_pTimer;

protected:
  bool event(QEvent *e);
  //void mousePressEvent(QMouseEvent *event);
  
  //void paintEvent(QPaintEvent* event);

signals:

public slots:
  void slotMinimize();
  void slotFocus();
  void slotOnTimer();
  void slotOnClose();

};

#endif // TRANSPARENTWIDGET_H
