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
/// \file    screenshot.h
///
/// \version $Id: screenshot.h 44064 2019-10-24 12:34:47Z wrueckl_elrest $
///
/// \brief   make screenshots due to testability
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <QPixmap>
#include <QWidget>

#include "cmdthread.h"


class Screenshot : public QWidget
{
  Q_OBJECT

public:
  Screenshot();

  cmdThread * pCmdThread;

private:
  void Save();
  bool Shoot();

  void startCmdThread();
  void CleanUp();

  void CreateFile(QString sFile);

  QPixmap desktopPixmap;


public slots:
  void cmdSlotReceived(QString s);

};

#endif // SCREENSHOT_H
