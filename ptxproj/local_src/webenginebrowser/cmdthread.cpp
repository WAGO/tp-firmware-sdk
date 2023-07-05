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
/// \file    cmdthread.cpp
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <QString>
#include <QDebug>
#include <QThread>
#include "msgtool.h"
#include "cmdthread.h"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
#define FIFO_DEVFILE    DEV_WEBENGINEBROWSER

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

cmdThread::cmdThread(QObject *parent) :
  QThread(parent)
{
  setTerminationEnabled();
  bInitialized = false;
#ifdef DEBUG_MSG
  qDebug() << "menuqt: " << "cmdThread CONSTRUCTOR ";
#endif

  unlink(FIFO_DEVFILE);
  if (mkfifo(FIFO_DEVFILE, 0666) >= 0)
  {
    if (chmod(FIFO_DEVFILE, 0666) >= 0)
    {
      bInitialized = true;
    }
  }
}

cmdThread::~cmdThread()
{
  unlink(FIFO_DEVFILE);

#ifdef DEBUG_MSG
  qDebug() << "menuqt: " << "cmdThread DESTRUCTOR ";
#endif
}

/// \brief thread function run
///
void cmdThread::run()
{
  QString sParameter;
  isRunning = 1;
  char *lineptr = NULL;
  size_t linelen;
  FILE *fp;
  int n;

  if (bInitialized)
  {
    fp = fopen(FIFO_DEVFILE, "r+");
    if (fp)
    {
      while (isRunning > 0)
      {
        if ((n = getline(&lineptr, &linelen, fp)) < 0)
          continue;

        //cmd received (without null termination)
        sParameter = QByteArray(lineptr, linelen);
        emit cmdSignalReceived(sParameter);
      }

      fclose(fp);
    }
  }

  exec();
}

