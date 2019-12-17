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
/// \file    screenshot.cpp
///
/// \version $Id: screenshot.cpp 43460 2019-10-09 13:25:56Z wrueckl_elrest $
///
/// \brief   make screenshots due to testability
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include <QtWidgets>

#include "screenshot.h"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
#define FILE_SCREENSHOT           "/tmp/screenshot.png"
#define FILE_DONE                 "/tmp/screenshot.done"
#define FILE_ERROR                "/tmp/screenshot.error"

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

Screenshot::Screenshot()
{
  pCmdThread = NULL;
  startCmdThread();
}

void Screenshot::CleanUp()
{
  QFile::remove(FILE_SCREENSHOT);
  QFile::remove(FILE_DONE);
  QFile::remove(FILE_ERROR);
}

/// \brief create an empty file
///
void Screenshot::CreateFile(QString sFile)
{
  QFile file(sFile);
  if (file.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    //QTextStream out(&file);
    //out << "done";
    file.close();
  }
}

/// \brief save screenshot to png file
///
void Screenshot::Save()
{
  //const QString format = "png";

  const QString fileName = FILE_SCREENSHOT;
  if (!desktopPixmap.save(fileName))
  {
    //QMessageBox::warning(this, tr("Save Error"), tr("The image could not be saved to \"%1\".").arg(QDir::toNativeSeparators(fileName)));
    CreateFile(FILE_ERROR);
  }
  else
  {
    //success create DONE file
    CreateFile(FILE_DONE);
  }
}


/// \brief make screenshot
///
bool Screenshot::Shoot()
{
  QScreen *screen = QGuiApplication::primaryScreen();
  if (const QWindow *window = windowHandle())
    screen = window->screen();
  if (!screen)
    return false;

  desktopPixmap = screen->grabWindow(0);
  return true;
}



/// \brief starting cmd thread
///
void Screenshot::startCmdThread()
{
  //call only once

  if (pCmdThread)
    return;

  pCmdThread = new cmdThread(this);

  if (! pCmdThread)
    return;

  connect(pCmdThread,
          SIGNAL(cmdSignalReceived(QString)),
          this,
          SLOT(cmdSlotReceived(QString)) );

  //start thread function run
  pCmdThread->start(QThread::NormalPriority);

}

/// \brief cmd message string received
///
void Screenshot::cmdSlotReceived(QString s)
{
  //qDebug() << "cmdSlotReceived: " << s;

  if (s.left(5).compare("shoot", Qt::CaseInsensitive) == 0)
  {
    CleanUp();
    if (Shoot() == true)
    {
      Save();
    }
    else
    {
      CreateFile(FILE_ERROR);
    }
  }
}
