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
/// \file    CustomSwipeGesture.cpp
///
/// \version $Id: CustomSwipeGesture.cpp 65689 2022-03-11 14:37:43Z falk.werner@wago.com $
///
/// \brief   use QGestureRecognizer to scroll
///
/// \author  Ralf Gliese, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include <QMouseEvent>
#include <QApplication>
#include <math.h>
#include "CustomSwipeGesture.h"
#include "globals.h"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
#define SCROLL_DISTANCE   10
#define UPPER_LIMIT_MENU_GESTURE 15

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

CustomSwipeGesture::CustomSwipeGesture(QObject *parent):QObject(parent)
{
  iMouseBtnState = 0;
  iMinDistance = SCROLL_DISTANCE;
}

/// \brief create a QSwipeGesture instance
///
QGesture *CustomSwipeGesture::create(QObject * pTarget)
{
  if ( pTarget && pTarget->isWidgetType() )
  {
    static_cast<QWidget *>(pTarget)->setAttribute(Qt::WA_AcceptTouchEvents);
  }
  QSwipeGesture *pGesture = new QSwipeGesture;
  //qDebug() << "CustomSwipeGesture::create() called " << pGesture;
  return pGesture;
}


/// \brief enable, disable menu
/// \param[in]  argc number of arguments
///
void CustomSwipeGesture::enableMenu(bool bEnabled)
{
  //bMenu = bEnabled;
}

/// \brief check if gesture is valid
/// \param[in]  dx  delta x
/// \param[in]  dy  delta y
/// \retval false  distance is too small
/// \retval true  distance is valid
bool CustomSwipeGesture::IsValidMove(int dx, int dy)
{    
   // The moved distance is too small to count as not just a glitch.
   if ((qAbs(dx) < iMinDistance) && (qAbs(dy) < iMinDistance))
   {
      return false;
   }
   return true;
}
 
/// \brief gesture recognize
/// \param[in]  pGesture  pointer to QGesture
/// \param[in]  target    pointer to QObject
/// \param[in]  event     pointer to QEvent
/// \retval QGestureRecognizer::Result
QGestureRecognizer::Result CustomSwipeGesture::recognize(QGesture* pGesture, QObject *target, QEvent *event)
{
  QGestureRecognizer::Result result = QGestureRecognizer::Ignore;

  switch(event->type())
  {
     case QEvent::MouseButtonPress:
     {
        QSwipeGesture *pSwipe = static_cast<QSwipeGesture*>(pGesture);        
        iMouseBtnState = QEvent::MouseButtonPress;

        QMouseEvent* pMouseEvent = static_cast<QMouseEvent*>(event);
        pSwipe->setProperty("startPoint", pMouseEvent->localPos());

        startPoint = pMouseEvent->localPos();
        globalStartPoint = pMouseEvent->globalPos();

        result = QGestureRecognizer::MayBeGesture;
        emit signalExecuteJsScrollbarVertCode();
     }
     break;
     case QEvent::MouseMove:
     {
        if (iMouseBtnState != QEvent::MouseButtonPress)
        {
          result = QGestureRecognizer::Ignore;
          break;
        }

        QMouseEvent* pMouseEvent = static_cast<QMouseEvent*>(event);
        endPoint = pMouseEvent->localPos();

        //qDebug() << "MouseMove " << endPoint.x() << ":" << endPoint.y();

        // process distance and direction
        int dx = endPoint.x() - startPoint.x();
        int dy = endPoint.y() - startPoint.y();

        if (!IsValidMove(dx, dy))
        {
           // Just a click, so no gesture.
           result = QGestureRecognizer::Ignore;
           //qDebug("MouseMove: Swipe gesture canceled");
        }
        else
        {
           QSwipeGesture *pSwipe = static_cast<QSwipeGesture*>(pGesture);
           // Compute the angle.
           qreal angle = ComputeAngle(dx, dy);
           //qDebug("swipe angle=%d \n", angle);
           //qDebug() << "angle = " << angle;

           pSwipe->setSwipeAngle(angle);
           result = QGestureRecognizer::Ignore;

           startPoint = pMouseEvent->localPos();

           Scroll(angle, dx, dy);

        }
     }
     break;
     case QEvent::MouseButtonRelease:
     {

        //qDebug("MouseButtonRelease");
        iMouseBtnState = QEvent::MouseButtonRelease;

        QSwipeGesture *pSwipe = static_cast<QSwipeGesture*>(pGesture);
        QMouseEvent* pMouseEvent = static_cast<QMouseEvent*>(event);
        const QVariant& propValue = pSwipe->property("startPoint");
        startPoint = propValue.toPointF();
        endPoint = pMouseEvent->localPos();
 
        // process distance and direction
        int dx = endPoint.x() - startPoint.x();
        int dy = endPoint.y() - startPoint.y();
 
        if (!IsValidMove(dx, dy))
        {
           // Just a click, so no gesture.
           result = QGestureRecognizer::CancelGesture;
        }
        else
        {
           // Compute the angle.
           qreal angle = ComputeAngle(dx, dy);
           result = QGestureRecognizer::FinishGesture;
        }

     }
     break;
     default:
       break;
  }
 
  //qDebug() << "recognize: " << event->type() << " result: " << result;

  return result;
}
 
/// \brief reset start point
/// \param[in]  pGesture pointer to QGesture
///
void CustomSwipeGesture::reset(QGesture *pGesture)
{
  pGesture->setProperty("startPoint", QVariant(QVariant::Invalid));

  QGestureRecognizer::reset( pGesture );
}
 
/// \brief calculate angle
/// \param[in]  dx  delta x
/// \param[in]  dy  delta y
/// \retval angle value
qreal CustomSwipeGesture::ComputeAngle(int dx, int dy)
{
  double PI = 3.14159265;
 
  // Need to convert from screen coordinates direction
  // into classical coordinates direction.
  dy = -dy;
 
  double result = atan2((double)dy, (double)dx) ;
  result = (result * 180) / PI;
 
  // Always return positive angle.
  if (result < 0)
  {
     result += 360;
  }
 
  return result;
}

/// \brief scroll gesture
/// \param[in]  dx  delta x
/// \param[in]  dy  delta y
void CustomSwipeGesture::Scroll(int angle, int dx, int dy)
{
  int Horizontal_Tol = 20;
  int Vertical_Tol = 20;
  int delta;

  // horizontal to right
  if((angle > (360 - Horizontal_Tol)) || (angle < Horizontal_Tol))
  {
    //qDebug("Scroll horizontal to right angle=%d dx=%d\n", angle, dx);
    delta = qAbs(dx);
    emit signalExecuteJsScrollbarHorizCode();
    emit signalScrollHLeft(endPoint.toPoint());
    return;
  }  

  // horizontal to left
  if((angle < (180 + Horizontal_Tol)) && (angle > (180 - Horizontal_Tol)))
  {
    //qDebug("Scroll horizontal to left angle=%d dx=%d\n", angle, dx);
    delta = qAbs(dx);
    emit signalExecuteJsScrollbarHorizCode();
    emit signalScrollHRight(endPoint.toPoint());
    return;
  }  
  
  // vertical down
  if((angle > (270 - Vertical_Tol)) && (angle < (270 + Vertical_Tol)))
  {
    //qDebug("Scroll vertical down angle=%d dy=%d\n", angle, dy);
    delta = qAbs(dy);
    emit signalScrollVUp(endPoint.toPoint()); //Down();
    return;
  }
  
  // vertical up
  if((angle > (90 - Vertical_Tol)) && (angle < (90 + Vertical_Tol)))
  {
    //qDebug("Scroll vertical up angle=%d dy=%d\n", angle, dy);
    delta = qAbs(dy);
    emit signalScrollVDown(endPoint.toPoint()); //Up();
    return;
  }
  
}

/*
/// \brief open menu per gesture
/// \param[in]  angle
/// \param[in]  dx  delta x
/// \param[in]  dy  delta y
void CustomSwipeGesture::CheckMenu(int angle, int dx, int dy)
{

  int Horizontal_Tol = 20;
  int Vertical_Tol = 20;
  int delta;

  //qDebug() << "globalStartPoint " << globalStartPoint.y() ;

  // vertical down
  if((angle > (270 - Vertical_Tol)) && (angle < (270 + Vertical_Tol)))
  {
    //qDebug("Scroll vertical down angle=%d dy=%d\n", angle, dy);
    delta = qAbs(dy);    

    if (globalStartPoint.y() < UPPER_LIMIT_MENU_GESTURE)
    {
      //emit signalOpenMenu();
    }

  }

}
*/

