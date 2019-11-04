//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019 WAGO Kontakttechnik GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
///
///  \file     get_screensaver.c
///
///  \version  $Id: get_screensaver.c 44064 2019-10-24 12:34:47Z wrueckl_elrest $
///
///  \brief    get screensaver settings / config-tools
///
///  \author   Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "msgtool.h"
#include "config_tool_lib.h"

//------------------------------------------------------------------------------
// Local macros
//------------------------------------------------------------------------------

#define CNF_FILE            SCREEN_CONF_FILE

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
int GetScreensaverActivity();

/// \brief main function
/// \param[in]  argc number of arguments
/// \param[in]  argv arguments
/// \retval 0  SUCCESS
int main (int argc, char **argv)
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
        system("/etc/config-tools/config_screensaver resettofactory > /dev/null 2>&1");  
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
        int activity = GetScreensaverActivity();
        
        char szJson[4096] = "";
        char szLine[512] = "";
        char *psz = szJson;
        //print all values in JSON format
        strcat(szJson, "{");                
        status = SUCCESS;                               
        psz += strlen(szJson);
        ConfPrintAllValuesJson(g_pList, psz);        
        if (activity == 0)
        {          
          //printf("active=false\n");
          sprintf(szLine, "\"active\": \"false\",");          
          strcat(szJson, szLine);
          status = SUCCESS;
        }
        else if (activity == 1)
        {
          //printf("active=true\n");
          sprintf(szLine, "\"active\": \"true\",");          
          strcat(szJson, szLine);
          status = SUCCESS;
        }
        else
        {
          status = ERROR;
        }
        ConfRemoveJsonSeparatorFromEnd(&szJson[0]);        
        strcat(szJson, "}");
        printf(szJson);
        
      }
      else
      {
              
        for (i = 1; i < argc; i++)
        {
          pStr = argv[i];
          
          if (stricmp(pStr, "state") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "mode") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "theme") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "image") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "time") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "text") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "active") == 0) 
          {                        
            int activity = GetScreensaverActivity();
            if (activity == 0)
            {
              printf("false");
              status = SUCCESS;
            }
            if (activity == 1)
            {
              printf("true");
              status = SUCCESS;
            }
            else
            {
              status = ERROR;
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
  printf("\n* Get screensaver settings * \n\n");
  printf("Usage: get_screensaver [options] <parameter>\n");
  printf("\n");
  printf("options:\n");
  printf("all-json print all values in JSON format \n"); 
  printf("-h print this help\n");
  printf("\n");
  printf("parameter: state | active | mode | theme | image | text | time\n");
  printf("\n");
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

/// \brief Read if screensaver is active at this moment 
///
/// The function returns the current screensaver activity,
/// if an error occurs the function returns -1
///
/// \retval  1 screensaver is just active on screen
/// \retval  0 screensaver is not active on screen
/// \retval -1 ERROR
int GetScreensaverActivity()
{
  int status = ERROR;
  char szCmd[MAX_LENGTH_SYSTEM_CALL] = "";
  char * pOutput = NULL;

  sprintf(szCmd, "pidof screensaverqt");
  pOutput = SystemCall_GetOutput(szCmd);
  if (pOutput)
  {
    if (strlen(pOutput) > 0)
    {
      //status = atoi(pOutput);
      status = 1;
    }
    else
    {
      status = 0;
    }
    SystemCall_Destruct(&pOutput);
  }

  return status;
}

