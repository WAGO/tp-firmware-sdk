//------------------------------------------------------------------------------
/// Copyright (c) 2019 WAGO Kontakttechnik GmbH & Co. KG
///
/// This program is free software: you can redistribute it and/or modify  
/// it under the terms of the GNU General Public License as published by  
/// the Free Software Foundation, version 3.
///
/// This program is distributed in the hope that it will be useful, but 
/// WITHOUT ANY WARRANTY; without even the implied warranty of 
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
/// General Public License for more details.
///
/// You should have received a copy of the GNU General Public License 
/// along with this program. If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
/// \file    cmdthread.cpp
///
/// \version $Id: cmdthread.cpp 43771 2019-10-18 09:36:27Z wrueckl_elrest $
///
/// \brief   Thread for receiving command f.e. open menu
///
/// \author  WRü: elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <QDebug>

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

#include "cmdthread.h"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
#define CMD_DEVFILE    "/dev/virtualkeyboard"

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
  bInitialized = false;
#ifdef DEBUG_MSG
  qDebug() << "menuqt: " << "cmdThread CONSTRUCTOR ";
#endif

  unlink(CMD_DEVFILE);
  if (mkfifo(CMD_DEVFILE, 0664) >= 0)
  {
    if (chmod(CMD_DEVFILE, 0664) >= 0)
    {
      bInitialized = true;
    }
  }
}

cmdThread::~cmdThread()
{
  unlink(CMD_DEVFILE);

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
    fp = fopen(CMD_DEVFILE, "r+");
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

