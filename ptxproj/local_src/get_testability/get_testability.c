//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019-2022 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
///
/// \file    get_testability.c
///
/// \version $Id: get_testability.c 65689 2022-03-11 14:37:43Z falk.werner@wago.com $
///
/// \brief   get testability settings / config-tools
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <msgtool.h>
#include "config_tool_lib.h"

//------------------------------------------------------------------------------
// Local macros
//------------------------------------------------------------------------------

#define CNF_FILE                  TESTABILITY_CONF_FILE

#define FILE_SCREENSHOT           "/tmp/screenshot.png"
#define FILE_DONE                 "/tmp/screenshot.done"
#define FILE_ERROR                "/tmp/screenshot.error"

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

void PrintScreenshotState()
{
  if ((Exists(FILE_DONE) == SUCCESS) && (Exists(FILE_SCREENSHOT) == SUCCESS))
  {
    printf("done");
  }
  else if (Exists(FILE_ERROR) == SUCCESS)
  {
    printf("error");
  }
  else
  {
    printf("none");
  }

}

/// \brief main function
/// \param[in]  argc number of arguments
/// \param[in]  argv arguments
/// \retval 0  SUCCESS
int main(int argc, char **argv)
{
  int status = ERROR;
  char szOut[256] = "";
  if (argc >= 2)
  {
    if ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0))
    {
      ShowHelpText();
      status = SUCCESS;
    }
    else
    {
      int i, ret;
      char * pStr;
      
      //create list
      g_pList = ConfCreateList();
      
      if (FileExists(CNF_FILE) != SUCCESS)
      {
        //resettofactory, create file
        system("/etc/config-tools/config_testability resettofactory > /dev/null 2>&1");  
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
      
      if ((strcmp(argv[1], "-a") == 0) || (strcmp(argv[1], "all") == 0) || (strcmp(argv[1], "all-json") == 0))
      {
        
        char szJson[4096] = "";
        char szLine[512] = "";
        char * psz = szJson;
        status = SUCCESS; 
        //print all values in JSON format
        strcat(szJson, "{");
        
        //read current state
        if ((Exists(FILE_DONE) == SUCCESS) && (Exists(FILE_SCREENSHOT) == SUCCESS))
        {
          char * ps = "done";
          sprintf(szLine, "\"screenshotstate\": \"%s\",", ps);          
          strcat(szJson, szLine);
        }
        else if (Exists(FILE_ERROR) == SUCCESS)
        {
          char * ps = "error";
          sprintf(szLine, "\"screenshotstate\": \"%s\",", ps);          
          strcat(szJson, szLine);
        }
        else
        {
          char * ps = "none";
          sprintf(szLine, "\"screenshotstate\": \"%s\",", ps);          
          strcat(szJson, szLine);
        }
        
        psz += strlen(szJson);
        ConfPrintAllValuesJson(g_pList, psz);
        ConfRemoveJsonSeparatorFromEnd(&szJson[0]);
        strcat(szJson, "}");
        printf(szJson);     
        
      }
      else
      {            
        for (i = 1; i < argc; i++)
        {
          pStr = argv[i];

          if (stricmp(pStr, "screenshot") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "screenshotstate") == 0) 
          {
            PrintScreenshotState();            
            status = SUCCESS;
          }
          else if (stricmp(pStr, "mousesim") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          
        }             
      }
    }
  }

  //clean up list memory
  ConfDestroyList(g_pList);

  return status;
}

/// \brief Print help text / usage to stdout
void ShowHelpText()
{
  printf("\n* Get testability settings *\n");
  printf("Usage: get_testability <parameter>\n");
  printf("options:\n");
  printf("all-json print all values in JSON format \n"); 
  printf("parameter: screenshot\n");   
  printf("output:    enabled | disabled\n");   
  printf("parameter: screenshotstate\n");   
  printf("output:    none | done | error\n");   
  printf("parameter: mousesim\n");   
  printf("output:    enabled | disabled\n\n");   
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

