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
///
/// \file    CustomSwipeGesture.h
///
/// \version $Id$
///
/// \brief   use QGestureRecognizer to scroll
///
/// \author  Ralf Gliese, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

#include <QWidget>
#include <QGesture>
#include <QGestureEvent>
#include <QGestureRecognizer>
#include <QObject>
#include <QEvent>
#include <QKeyEvent>
#include <QDebug>

class CustomSwipeGesture : public QObject, public QGestureRecognizer
{
  Q_OBJECT

public:
  CustomSwipeGesture(QObject *parent = 0);
  QGesture* create(QObject *pTarget);
  QGestureRecognizer::Result recognize(QGesture* pGesture, QObject *target, QEvent *event);  
  void reset(QGesture *pGesture);

  void enableMenu(bool bEnabled);

private:
  int iMinDistance;
  bool IsValidMove(int dx, int dy);
  qreal ComputeAngle(int dx, int dy);
  void Scroll(int angle, int dx, int dy);
  //void CheckMenu(int angle, int dx, int dy);

  QPointF startPoint;
  QPointF endPoint;

  QPoint globalStartPoint;

  int iMouseBtnState;

  //bool bMenu;

signals:
  void signalScrollVDown(QPoint);
  void signalScrollVUp(QPoint);
  void signalScrollHRight(QPoint);
  void signalScrollHLeft(QPoint);
  void signalExecuteJsScrollbarVertCode();
  void signalExecuteJsScrollbarHorizCode();
  //void signalOpenMenu();
};
