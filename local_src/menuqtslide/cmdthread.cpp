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
/// \file    cmdthread.cpp
///
/// \version $Id: cmdthread.cpp 44064 2019-10-24 12:34:47Z wrueckl_elrest $
///
/// \brief   Thread for receiving command f.e. open menu
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <QDebug>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
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
#define CMD_DEVFILE    "/dev/toolbarmenu"

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
  if (mkfifo(CMD_DEVFILE, 0666) >= 0)
  {
    if (chmod(CMD_DEVFILE, 0666) >= 0)
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

        //close screensaver if running
        if (FindProcess("screensaverqt") == true)
        {          
          system("killall -9 screensaverqt > /dev/null 2>&1");
        }
        
        //cmd received (without null termination)
        sParameter = QByteArray(lineptr, linelen);
        emit cmdSignalReceived(sParameter);
      }

      fclose(fp);
    }
  }

  exec();
}

bool cmdThread::FindProcess(const char * pszName)
{
   bool bRet = false;
   const char* directory = "/proc";
   size_t      taskNameSize = 1024;
   char*       taskName = (char*) calloc(1, taskNameSize);

   DIR* dir = opendir(directory);

   if (dir)
   {
      struct dirent* de = 0;

      while ((de = readdir(dir)) != 0)
      {
         if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;

         int pid = -1;
         int res = sscanf(de->d_name, "%d", &pid);

         if (res == 1)
         {
            // we have a valid pid

            // open the cmdline file to determine what's the name of the process running
            char cmdline_file[1024] = {0};
            sprintf(cmdline_file, "%s/%d/cmdline", directory, pid);

            FILE* cmdline = fopen(cmdline_file, "r");
            if (cmdline)
            {
              if (getline(&taskName, &taskNameSize, cmdline) > 0)
              {
                // is it the process we care about?
                if (strstr(taskName, pszName) != 0)
                {
                  //fprintf(stdout, "A %s process, with PID %d, has been detected.\n", pszName, pid);
                  //found
                  bRet = true;
                }
              }
              
              fclose(cmdline);
            }
         }
         if (bRet)
           break;
      }

      closedir(dir);
   }

   free(taskName);

   return bRet;
}
