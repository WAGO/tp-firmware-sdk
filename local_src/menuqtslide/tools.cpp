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
/// \file    tools.cpp
///
/// \version $Id: tools.cpp 43460 2019-10-09 13:25:56Z wrueckl_elrest $
///
/// \brief   tools
///
/// \author  Wolfgang Rückl : elrest GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include "tools.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include "globals.h"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
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


/// \brief Get display resolution information
/// get info if orientation is landscape or portrait
/// \retval DISPLAY_ORIENTATION
DISPLAY_ORIENTATION GetDisplayOrientation()
{
  QRect rScreen = QApplication::desktop()->screenGeometry();

  //Auflösungen der WAGO Displays
  // 4,3“  480 x 272 Pixel Small
  // 5,7“  640 x 480 Pixel Small
  // 7“    800 x 480 Pixel Medium
  // 10,4“ 800 x 600 Pixel Medium
  // 10,1“ 1024 x 600 Pixel Medium

  //      1280 x 720 Pixel

  DISPLAY_ORIENTATION ret = DISPLAY_LANDSCAPE;

  if (rScreen.height() > rScreen.width())
  {
    ret = DISPLAY_PORTRAIT;
  }

  //qDebug() << "DISPLAY ORIENTATION: " << ret;

  return ret;
}

/// \brief Get display size information
/// get info about display size small, medium, big
/// \retval DISPLAY_SIZE
DISPLAY_SIZE GetDisplaySize()
{
  QRect rScreen = QApplication::desktop()->screenGeometry();

  //Auflösungen der WAGO Displays
  // 4,3“  480 x 272 Pixel Small
  // 5,7“  640 x 480 Pixel Small
  // 7“    800 x 480 Pixel Medium
  // 10,1“ 1024 x 600 Pixel Medium
  // 10,4“ 800 x 600 Pixel Medium
  // 12,1“ 1024 x 768 Pixel Medium

  DISPLAY_SIZE ret = DISPLAY_MEDIUM;

  // 5,7" (4:3)
  if ((rScreen.width() == 640) && (rScreen.height() == 480))
  {
    ret = DISPLAY_SMALL;
  }
  else if ((rScreen.width() == 480) && (rScreen.height() == 640))
  {
    ret = DISPLAY_SMALL;
  }
  // 4,3" (widescreen 16:9)
  else if ((rScreen.width() == 480) && (rScreen.height() == 272))
  {
    ret = DISPLAY_SMALL;
  }
  else if ((rScreen.width() == 272) && (rScreen.height() == 480))
  {
    ret = DISPLAY_SMALL;
  }
  // 7" (widescreen 16:9)
  else if ((rScreen.width() == 800) && (rScreen.height() == 480))
  {
    ret = DISPLAY_MEDIUM;
  }
  else if ((rScreen.width() == 480) && (rScreen.height() == 800))
  {
    ret = DISPLAY_MEDIUM;
  }
  // 10,4" (4:3)
  else if ((rScreen.width() == 800) && (rScreen.height() == 600))
  {
    ret = DISPLAY_MEDIUM;
  }
  else if ((rScreen.width() == 600) && (rScreen.height() == 800))
  {
    ret = DISPLAY_MEDIUM;
  }
  // 10,1" (16:9)
  else if ((rScreen.width() == 1024) && (rScreen.height() == 600))
  {
    ret = DISPLAY_MEDIUM;
  }
  else if ((rScreen.width() == 600) && (rScreen.height() == 1024))
  {
    ret = DISPLAY_MEDIUM;
  }
  // 1280 x 800 (16:9)
  else if ((rScreen.width() == 1280) && (rScreen.height() == 800))
  {
    ret = DISPLAY_MEDIUM;
  }
  else if ((rScreen.width() == 800) && (rScreen.height() == 1280))
  {
    ret = DISPLAY_MEDIUM;
  }
  else if ((rScreen.width() > 1280) || (rScreen.height() > 1280))
  {
    ret = DISPLAY_BIG;
  }

  //qDebug() << "DISPLAY SIZE: " << ret;

  return ret;
}

/// \brief calculate string width depending on font
///
/// \param[in] sText    calculating width for this string
/// \param[in] fnt      calculating width using this font
/// \retval  pixel width of the string
int CalculatePixelWidth(QString sText, QFont fnt)
{
  QFontMetrics fm(fnt);
  int pixelsWide = fm.width(sText);
  return (pixelsWide);
}

/// \brief calculate height depending on font
///
/// \param[in] fnt      calculating width using this font
/// \retval  pixel height of the font
int CalculatePixelHeight(QFont fnt)
{
  QFontMetrics fm(fnt);
  int pixelsHigh = fm.height();
  return (pixelsHigh);
}

