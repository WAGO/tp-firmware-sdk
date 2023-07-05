//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019-2022 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
///
/// \file    config_testability.c
///
/// \version $Id$
///
/// \brief   testability settings / config-tools
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <glib.h>
#include "msgtool.h"
#include "config_tool_lib.h"


//------------------------------------------------------------------------------
// Local macros
//------------------------------------------------------------------------------

#define CNF_FILE           TESTABILITY_CONF_FILE
#define DEV_SCREENSHOT     "/dev/screenshot"

//------------------------------------------------------------------------------
// External variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local typedefs
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local variables
//------------------------------------------------------------------------------

/// configuration list start pointer
tConfList * g_pList = NULL;

/// \brief Config data in form of name, default value, value list comma separated (no spaces)
static tConfValues aConfigValues[] =
{
  { "mousesim", "disabled", "enabled,disabled" },
  { "screenshot", "disabled", "enabled,disabled" },
  // this line must always be the last one - don't remove it!
  { "", "" }
};

//------------------------------------------------------------------------------
// external functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local functions
//------------------------------------------------------------------------------

void ShowHelpText();
void AppendErrorText(int iStatusCode, char * pText);

/// \brief check if file exists
/// \param[in]  pFilename absolute file name
/// \retval -1 ERROR
/// \retval 0 SUCCESS
int Exists(char * pFilename)
// Check if file is available
{
  struct stat sts;
  if (stat(pFilename, &sts) == -1 && errno == ENOENT)
  {
    return ERROR;
  }
  else
  {   
    return SUCCESS;
  }
}

int MakeScreenshot()
{
  int iRet = ERROR;
  if (Exists(DEV_SCREENSHOT) == SUCCESS)
  {
    int fd = open(DEV_SCREENSHOT, O_RDWR | O_NONBLOCK);
    if (fd >= 0)
    {
      char * pCmd = "shoot\n";
      if (write(fd, pCmd, strlen(pCmd)) > 0)
      {
        iRet = SUCCESS;
        //printf("write success: %s\n", DEV_SCREENSHOT);
      }
      else
      {
        printf("write failed: %s\n", DEV_SCREENSHOT);
      }
      close(fd);
    }
    else
    {
      printf("open failed: %s\n", DEV_SCREENSHOT);
    }
  }
  else
  {
    printf("file not found: %s\n", DEV_SCREENSHOT);
  }

  return iRet;
}


/// \brief main function
/// \param[in]  argc number of arguments
/// \param[in]  argv arguments
/// \retval 0  SUCCESS
int main(int argc, char **argv)
{
  int status = ERROR;
  //int restart_service = 0;
  char szArgv[256] = "";
  
  char szSearchStr[128] = "";
  int len = 0;
 
  if (argc >= 2)
  {
    if ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0))
    {
      ShowHelpText();
      status = SUCCESS;
    }
    else if (stricmp(argv[1], "resettofactory") == 0) 
    {
      if (ConfResetToDefaultValues(&aConfigValues[0], CNF_FILE) == SUCCESS)
        status = SUCCESS;
    }
    else if (stricmp(argv[1], "shoot") == 0) 
    {
      status = MakeScreenshot();
    }
    else
    {
      int i, k, ret;
      char * pStr;
      
      //create list
      g_pList = ConfCreateList();
      
      if (FileExists(CNF_FILE) != SUCCESS)
      {
        //create file
        ConfResetToDefaultValues(&aConfigValues[0], CNF_FILE);
      }
      
      ret = ConfReadValues(g_pList, CNF_FILE);
      if (ret != SUCCESS)
      {
        char szTxt[256];
        sprintf(szTxt, "File open error %s", CNF_FILE);
        setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt); 

        AppendErrorText(FILE_OPEN_ERROR, CNF_FILE);
        //terminate
        ConfDestroyList(g_pList);
        return ERROR;
      }
      
      if (ConfGetCount(g_pList) == 0)
      {
        char szTxt[256];
        sprintf(szTxt, "File inconsistent error %s", CNF_FILE);
        setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt); 
        
        AppendErrorText(CONFIG_FILE_INCONSISTENT, CNF_FILE);
        //terminate
        ConfDestroyList(g_pList);
        return ERROR;
      }
           
      for (i = 1; i < argc; i++)
      {
        // URL decoding       
        strncpy(szArgv, g_uri_unescape_string(argv[i], ""), sizeof(szArgv));     
        
        pStr =  &szArgv[0];
        
        k = 0;
        while (aConfigValues[k].nameStr[0])
        {          
          sprintf(szSearchStr, "%s=", aConfigValues[k].nameStr); 
          len = strlen(szSearchStr);
          
          if (strstr(szArgv, szSearchStr) && (strlen(pStr) > len))
          {
            pStr += len;
            
            if (stricmp(aConfigValues[k].nameStr, "mousesim") == 0)
            { 
              if (ConfSetValue(&aConfigValues[0], g_pList, aConfigValues[k].nameStr, pStr)==SUCCESS)
              {
                char szCmd[512] = "";
                if (stricmp(pStr, "enabled")==0)
                {
                  sprintf(szCmd, "chmod a+x /usr/bin/xdotool");
                }
                else
                {
                  sprintf(szCmd, "chmod a-x /usr/bin/xdotool");
                }  
                system(szCmd);
                status = SUCCESS;
              }                
              
            }
            else 
            {
              if (ConfSetValue(&aConfigValues[0], g_pList, aConfigValues[k].nameStr, pStr)==SUCCESS)
                status = SUCCESS;
            }
            break;
          }        
          k++;
        }        

      }

      //save values to file
      if (status == SUCCESS)
      {
        if (ConfGetCount(g_pList) > 0)
        {
          ConfSaveValues(g_pList, CNF_FILE);
        }
        else
        {
          char szTxt[256];
          sprintf(szTxt, "No items found in %s", CNF_FILE);
          SetLastError(szTxt); 
          status = ERROR; 
          setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt); 
        }
      }
      
      //if (restart_service)
      //{        
      //}
            
    }
  }

  //clean up list memory
  ConfDestroyList(g_pList);

  return status;
}

/// \brief Print help text / usage to stdout
void ShowHelpText()
{
  printf("* testability settings  * \n");
  printf("Usage: config_testability [mousesim=mousesim-value]\n");
  printf("mousesim-value:   enabled | disabled \n\n");
  printf("Usage: config_testability [screenshot=screenshot-value]\n");
  printf("screenshot-value:   enabled | disabled \n");
  printf("Usage: config_testability shoot\n");
  printf("make screenshot and save it to /tmp/screenshot.png \n\n");
}

/// \brief Print error text to /tmp/last_error.txt
///
/// \param[in]  iStatusCode  code defined in eStatusCode config_tool_lib.h
/// \param[in]  pText        additional text
void AppendErrorText(int iStatusCode, char * pText)
{
  char szSub[128] = "";
  char szError[256] = "";
  GetErrorText(iStatusCode, szSub, sizeof(szSub));
  sprintf(szError, "error %s %s", szSub, pText);
  SetLastError(szError);
}

