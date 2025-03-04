//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019-2022 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
///
/// \file    get_eruntime.c
///
/// \version $Id$
///
/// \brief   get eruntime cfg contents / config-tools
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------
#define _GNU_SOURCE 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <msgtool.h>
#include "config_tool_lib.h"

//------------------------------------------------------------------------------
// Local macros
//------------------------------------------------------------------------------

#define CFG_FILE            "/home/codesys/CODESYSControl.cfg"
#define CNF_FILE            ERUNTIME_CONF_FILE

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
int GetBootAppState();

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
        system("/etc/config-tools/config_eruntime resettofactory > /dev/null 2>&1");  
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
               
        //print all values in JSON format        
        strcat(szJson, "{");
        
        if (GetBootAppState() == 0)
        {
          sprintf(szLine, "\"bootapp\": \"no\",");
        }
        else
        {
          sprintf(szLine, "\"bootapp\": \"yes\",");
        }
        strcat(szJson, szLine);
        
      
        if (ConfGetValue(g_pList, "multicore", &szOut[0], sizeof(szOut)) == SUCCESS)                       
        {
          sprintf(szLine, "\"multicore\": \"%s\",", szOut);
          strcat(szJson, szLine);
          //printf("path=%s\n", szOut);  
        }
        
        ConfRemoveJsonSeparatorFromEnd(&szJson[0]);
              
        strcat(szJson, "}");
        printf(szJson);
        
        //Path
        //ConfPrintAllValues(g_pList);  
        status = SUCCESS;
      }
      else
      {
              
        for (i = 1; i < argc; i++)
        {
          pStr = argv[i];
          
          if (stricmp(pStr, "bootapp") == 0) 
          {
            status = SUCCESS;
            if (GetBootAppState() == 0)
            {
              printf("no");              
            }
            else
            {
              printf("yes");
            }
          }
          else if (stricmp(pStr, "multicore") == 0) 
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
  printf("\n* Get e!Runtime infos *\n\n");
  printf("Usage: get_eruntime <parameter>\n");
  printf("options:\n");
  printf("all-json print all values in JSON format \n"); 
  printf("\n");
  printf("parameter: multicore value: yes | no\n");   
  printf("parameter: bootapp   value: yes | no\n\n");   
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

/// \brief Boot-Application availibility function
/// \retval 0  no boot application downloaded
/// \retval 1  boot application exists
int GetBootAppState()
{
  // [CmpApp]
  // Application.1=Application
  
  int ret = 0;
  FILE *stream;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  char * pszSearch1 = "Application.0";
  char * pszSearch2 = "Application.1";
  char * pszSearch3 = "Application.2";

  stream = fopen(CFG_FILE, "r");
  if (stream == NULL)
     return 0;

  while ((read = getline(&line, &len, stream)) != -1) 
  { 
    char * pLine = line;
    while (pLine[0] == ' ')
    {
      pLine++;
    }
    //check if is comment line
    if ((pLine[0] != '/') && (pLine[0] != '#') && (pLine[0] != '['))
    {
      if ((strcasestr(line, pszSearch1) != NULL) || (strcasestr(line, pszSearch2) != NULL) || (strcasestr(line, pszSearch3) != NULL))
      {
        ret = 1;
        break;
      }
    }
  }
  if (line)
    free(line);
  
  if (stream)  
    fclose(stream);

  return ret;
}
