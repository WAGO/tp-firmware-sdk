//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019-2022 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
///
/// \file    get_backlight.c
///
/// \version $Id$
///
/// \brief   get backlight settings / config-tools
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <msgtool.h>
#include "config_tool_lib.h"

//------------------------------------------------------------------------------
// Local macros
//------------------------------------------------------------------------------

#define CNF_FILE            BACKLIGHT_CONF_FILE

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
int SetBacklightValue(int iValue);
int GetBacklightValue();
void AppendErrorText(int iStatusCode, char * pText);

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
        system("/etc/config-tools/config_backlight resettofactory > /dev/null 2>&1");  
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
        
        //read current display backlight from driver
        int iVal = GetBacklightValue();
        if (iVal >= 0)
        {
          //printf("backlight=%d\n", iVal);          
          sprintf(szLine, "\"backlight\": \"%d\",", iVal);          
          strcat(szJson, szLine);
          status = SUCCESS;
        }
        else
        {
          status = ERROR; 
          SetLastError("Could not read backlight value.");  
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
          if (stricmp(pStr, "backlight") == 0) 
          {                   
            //read current display backlight from driver
            int iVal = GetBacklightValue();
            if (iVal >= 0)
            {
              //printf("%s=%d\n", pStr, iVal);
              printf("%d", iVal);
              status = SUCCESS;
            }
            else
            {
              SetLastError("Could not read backlight value.");  
            }
          }
          else if (stricmp(pStr, "backlighton") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "backlightonnight") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "backlightoffnight") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "backlightoff") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "backlightminvalue") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "mastermode") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "daynightswitch") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "daytime") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "nighttime") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "usesensor") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "sensorranges") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "testbacklight") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "plusminusstep") == 0) 
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
  printf("\n* Backlight settings * \n\n");
  
  printf("Usage: get_backlight [options] <parameter>\n");
  printf("\n");
  printf("options:\n");
  printf("all-json print all values in JSON format \n"); 
  printf("-h print this help\n");
  
  printf("Parameter:           backlight |\n");
  printf("                     backlighton | backlightoff |\n");
  printf("                     backlightonnight | backlightoffnight |\n");
  printf("                     daynightswitch | nighttime | daytime |\n");
  printf("                     mastermode | usesensor | sensorranges\n");
  printf("                     plusminusstep\n\n");
    
  printf("backlight            0..255 (current measurement)\n");
  printf("backlighton          0..255 (configured value)\n");
  printf("backlightoff         0..255 (configured value)\n");  
  printf("backlightonnight     0..255 (configured value)\n");
  printf("backlightoffnight    0..255 (configured value)\n");  
  printf("backlightminvalue    0..255 (configured value)\n");
  printf("nighttime            timestamp in format [hh:mm:ss] f.e. 21:00:00\n");
  printf("daytime              timestamp in format [hh:mm:ss] f.e. 06:00:00\n");
  printf("daynightswitch       enabled | disabled\n");  
  printf("mastermode:          master | slave | none\n");
  printf("usesensor:           yes | no\n");
  printf("testbacklight        backlight test time [s]\n");
  printf("sensorranges:        comma separated from,to,value,from,to,value etc.\n");
  printf("                     f.e. 0,50,30,50,100,70,100,255,234\n");  
  printf("plusminusstep        1..255 \n\n");    
}

/// \brief Set current touchscreen backlight value 
///
/// The function sets the current backlight value,
/// if an error occurs the function returns -1
///
/// \param[in] iValue  new backlight value [0..255]
/// \retval  0 SUCCESS
/// \retval -1 ERROR
int SetBacklightValue(int iValue)
{
  char szCmd[MAX_LENGTH_SYSTEM_CALL];
  sprintf(szCmd, "setup_display %d", iValue);
  system(szCmd);
  return SUCCESS;
}

/// \brief Read current touchscreen backlight value 
///
/// The function returns the current backlight value,
/// if an error occurs the function returns -1
///
/// \retval    backlight value [0..255]
/// \retval -1 ERROR
int GetBacklightValue()
{
  int status = ERROR;
  char szCmd[MAX_LENGTH_SYSTEM_CALL] = "";
  char * pOutput = NULL;

  sprintf(szCmd, "setup_display g");
  pOutput = SystemCall_GetOutput(szCmd);
  if (pOutput)
  {
    if (strlen(pOutput) > 0)
    {
      status = atoi(pOutput);
    }
    SystemCall_Destruct(&pOutput);
  }

  return status;
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

