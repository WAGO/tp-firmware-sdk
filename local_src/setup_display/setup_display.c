//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019-2022 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
/// \file    setup_display.c
///
/// \version $Id: setup_display.c 65689 2022-03-11 14:37:43Z falk.werner@wago.com $
///
/// \brief   set and get display brigthness
///
/// \author  Ralf Gliese, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include "confparse.h"

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

int main(int argc, char *argv[])
{
  int               fd, iBrightness;
  FILE              *fd_procfs;
  unsigned long    ulDisplayType;
  char              cValueBuf[20];
  char              *szInVal;
  
	if(argc > 1)
	{
    szInVal = argv[1];
    fd = open("/sys/class/backlight/backlight/brightness", O_RDWR);
    if (fd < 0)
    {
      perror("error open /sys/class/backlight/backlight/brightness");
      exit(EXIT_FAILURE);
    }
    
    if(*szInVal == 'g')
    {
      if (read(fd, cValueBuf, 3) <=0)
      {
        printf("read failed \n");
        exit(EXIT_FAILURE);
      }
      else
      {
        iBrightness = atoi(cValueBuf);
        //print to stdout without spaces and linefeed
        printf("%d", iBrightness); 
      }
    }
    else
    {
      iBrightness = atoi(argv[1]);
      printf("%d", iBrightness);
      if((iBrightness < 0) || (iBrightness > 255))
      {
        printf("invalid brigthness value\n");
        exit(EXIT_FAILURE);
      }

      if (write(fd, argv[1], sizeof(argv[1])) <=0)
      {
        perror("error write to /sys/class/backlight/backlight/brigthness failed");
      }

      if (close(fd) < 0)
      {
        perror("error closing sysfs fd");
        exit(EXIT_FAILURE);
      }
    }
  }
	exit(EXIT_SUCCESS);
}
