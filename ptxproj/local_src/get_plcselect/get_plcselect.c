//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019-2022 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
///
/// \file    get_plcselect.c
///
/// \version $Id$
///
/// \brief   read backlight settings / config-tools
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
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

#define MAX_PLC_NUMBER      12 
#define CNF_FILE            PLCSELECT_CONF_FILE

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

void ShowHelpText();
int GetPlcCount();
void AppendErrorText(int iStatusCode, char * pText);

//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables’ and constants’ definitions
//------------------------------------------------------------------------------

/// configuration list start pointer
tConfList * g_pList = NULL;

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

/// \brief main function
/// \param[in]  argc number of arguments
/// \param[in]  argv arguments
/// \retval 0  SUCCESS
int main(int argc, char **argv)
{
  int status = ERROR;
  int index = 0;
  char szOut[256] = "";
  
  int i, ret;
  char * pStr;
  
  if (argc >= 2)
  {
    if ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0))
    {
      ShowHelpText();
      status = SUCCESS;
    }
    else if ((strcmp(argv[1], "-a") == 0) || (strcmp(argv[1], "all") == 0) || (strcmp(argv[1], "all-json") == 0))
    {            
      char szJson[8192] = "";
      char szLine[512] = "";
      char szConverted[512] = "";
      char szName[128] = "";
      char *psz = szJson;
      pStr = szName;
      //print all values in JSON format
      strcat(szJson, "{");
      
      //create list
      g_pList = ConfCreateList();
      
      if (FileExists(CNF_FILE) != SUCCESS)
      {
        //resettofactory, create file
        system("/etc/config-tools/config_plcselect resettofactory > /dev/null 2>&1");  
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
                           
      strcpy(pStr, "reconnect_interval");        
      if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
      {
        //JSON
        sprintf(szLine, "\"%s\": \"%s\",", pStr, szOut);          
        strcat(szJson, szLine);
        status = SUCCESS;
      }
      
      strcpy(pStr, "v2updatecheck_interval");        
      if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
      {
        //JSON
        sprintf(szLine, "\"%s\": \"%s\",", pStr, szOut);          
        strcat(szJson, szLine);
        status = SUCCESS;
      }            
      
      strcpy(pStr, "plc_count");      
      //JSON
      sprintf(szLine, "\"%s\": \"%d\",", pStr, GetPlcCount());          
      strcat(szJson, szLine);
      status = SUCCESS;
      
      
      strcpy(pStr, "plc_selected");
      if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
      {
        //JSON
        sprintf(szLine, "\"%s\": \"%s\",", pStr, szOut);          
        strcat(szJson, szLine);
        status = SUCCESS;
      }
      
      strcpy(pStr, "statemonitor");
      if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
      {
        //JSON
        sprintf(szLine, "\"%s\": \"%s\",", pStr, szOut);          
        strcat(szJson, szLine);
        status = SUCCESS;
      }
      
      //JSON URL ARRAY
      strcat(szJson, "\"url\": [");
      for (index=0; index < MAX_PLC_NUMBER; index++)
      {
        sprintf(szName, "url%02d", index);  
        if (ConfGetValue(g_pList, szName, &szOut[0], sizeof(szOut)) == SUCCESS)
        {         
          ConfExpandEscapes(szOut, szConverted);
          sprintf(szLine, "\"%s\", ", szConverted);          
          strcat(szJson, szLine);
          status = SUCCESS;
        }                        
      }
      ConfRemoveJsonSeparatorFromEnd(&szJson[0]);
      strcat(szJson, "], ");
      
      //JSON TXT ARRAY
      strcat(szJson, "\"txt\": [");
      for (index=0; index < MAX_PLC_NUMBER; index++)
      {
        sprintf(szName, "txt%02d", index);  
        if (ConfGetValue(g_pList, szName, &szOut[0], sizeof(szOut)) == SUCCESS)
        {
          ConfExpandEscapes(szOut, szConverted);
          sprintf(szLine, "\"%s\", ", szConverted);          
          strcat(szJson, szLine);
          status = SUCCESS;
        }                        
      }
      ConfRemoveJsonSeparatorFromEnd(&szJson[0]);
      strcat(szJson, "], ");
      
      //JSON VKB ARRAY
      strcat(szJson, "\"vkb\": [");
      for (index=0; index < MAX_PLC_NUMBER; index++)
      {
        sprintf(szName, "vkb%02d", index);  
        if (ConfGetValue(g_pList, szName, &szOut[0], sizeof(szOut)) == SUCCESS)
        {
          sprintf(szLine, "\"%s\", ", szOut);          
          strcat(szJson, szLine);
          status = SUCCESS;
        }                        
      }
      ConfRemoveJsonSeparatorFromEnd(&szJson[0]);
      strcat(szJson, "], ");

      //JSON MON ARRAY
      strcat(szJson, "\"mon\": [");
      for (index=0; index < MAX_PLC_NUMBER; index++)
      {
        sprintf(szName, "mon%02d", index);  
        if (ConfGetValue(g_pList, szName, &szOut[0], sizeof(szOut)) == SUCCESS)
        {
          sprintf(szLine, "\"%s\", ", szOut);          
          strcat(szJson, szLine);
          status = SUCCESS;
        }                        
      }
      ConfRemoveJsonSeparatorFromEnd(&szJson[0]);
      strcat(szJson, "], ");
      
      //JSON MIC ARRAY
      strcat(szJson, "\"mic\": [");
      for (index=0; index < MAX_PLC_NUMBER; index++)
      {
        sprintf(szName, "mic%02d", index);  
        if (ConfGetValue(g_pList, szName, &szOut[0], sizeof(szOut)) == SUCCESS)
        {
          sprintf(szLine, "\"%s\", ", szOut);          
          strcat(szJson, szLine);
          status = SUCCESS;
        }                        
      }
      
      ConfRemoveJsonSeparatorFromEnd(&szJson[0]);
      strcat(szJson, "]}");
      printf(szJson); 
      
    }
    else
    {
     
      //create list
      g_pList = ConfCreateList();
      
      if (FileExists(CNF_FILE) != SUCCESS)
      {
        //resettofactory, create file
        system("/etc/config-tools/config_plcselect resettofactory > /dev/null 2>&1");  
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
      
      if (ConfIsNumber(argv[1]) == SUCCESS)
      {
        index = atoi(argv[1]);           
        if ((index < 0) || (index >= MAX_PLC_NUMBER))
        {
          char szTxt[256];
          sprintf(szTxt, "Invalid Parameter %s", argv[0]);
          setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt);
          
          //wrong index terminate
          AppendErrorText(INVALID_PARAMETER, argv[0]);
          ConfDestroyList(g_pList);
          return ERROR;  
        }
      }
            
      for (i = 1; i < argc; i++)
      {
        pStr = argv[i];

        if (stricmp(pStr, "reconnect_interval") == 0) 
        {
          if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
          {
            //printf("%s=%s\n", pStr, szOut);
            printf("%s", szOut);
            status = SUCCESS;
          }
        }
        else if (stricmp(pStr, "v2updatecheck_interval") == 0) 
        {
          if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
          {
            //printf("%s=%s\n", pStr, szOut);
            printf("%s", szOut);
            status = SUCCESS;
          }
        }
        else if (stricmp(pStr, "plc_count") == 0) 
        {          
          //printf("%s=%s\n", pStr, szOut);
          printf("%d", GetPlcCount());
          status = SUCCESS;          
        }
        else if (stricmp(pStr, "plc_selected") == 0) 
        {
          if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
          {
            //printf("%s=%s\n", pStr, szOut);
            printf("%s", szOut);
            status = SUCCESS;
          }
        }
        else if (stricmp(pStr, "statemonitor") == 0) 
        {
          if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
          {
            //printf("%s=%s\n", pStr, szOut);
            printf("%s", szOut);
            status = SUCCESS;
          }
        }
        else
        {
          char szUrl[64] = "";
          char szTxt[64] = "";
          char szVkb[64] = "";
          char szMon[64] = "";
          char szMic[64] = "";
          if (index >= 0)
          {
            //check lines url1= url2= url3= etc.
            //check lines txt1= txt2= txt3= etc.
            
            sprintf(szUrl, "url%02d", index);  
            sprintf(szTxt, "txt%02d", index);  
            sprintf(szVkb, "vkb%02d", index);  
            sprintf(szMon, "mon%02d", index);  
            sprintf(szMic, "mic%02d", index);  
            
            if (stricmp(pStr, "url") == 0) 
            {
              if (ConfGetValue(g_pList, szUrl, &szOut[0], sizeof(szOut)) == SUCCESS)
              {
                //printf("%s=%s\n", pStr, szOut);
                printf("%s",szOut);
                status = SUCCESS;
              }
              break;
            }            
            
            if (stricmp(pStr, "txt") == 0) 
            {
              if (ConfGetValue(g_pList, szTxt, &szOut[0], sizeof(szOut)) == SUCCESS)
              {
                //printf("%s=%s\n", pStr, szOut);
                printf("%s",szOut);
                status = SUCCESS;
              }
              break;
            }                                      
            
            if (stricmp(pStr, "vkb") == 0) 
            {
              if (ConfGetValue(g_pList, szVkb, &szOut[0], sizeof(szOut)) == SUCCESS)
              {
                //printf("%s=%s\n", pStr, szOut);
                printf("%s",szOut);
                status = SUCCESS;
              }
              break;
            }
            
            if (stricmp(pStr, "mon") == 0) 
            {
              if (ConfGetValue(g_pList, szMon, &szOut[0], sizeof(szOut)) == SUCCESS)
              {
                //printf("%s=%s\n", pStr, szOut);
                printf("%s",szOut);
                status = SUCCESS;
              }
              break;
            }
            
            if (stricmp(pStr, "mic") == 0) 
            {
              if (ConfGetValue(g_pList, szMic, &szOut[0], sizeof(szOut)) == SUCCESS)
              {
                //printf("%s=%s\n", pStr, szOut);
                printf("%s",szOut);
                status = SUCCESS;
              }
              break;
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
  printf("\n* Read PLC selection settings * \n\n");
  printf("options:\n");
  printf("all-json print all values in JSON format \n"); 
  printf("-h print this help \n\n");
  printf("Usage1:    get_plcselect <parameter> \n");
  printf("parameter: statemonitor | reconnect_interval | plc_selected | plc_count\n");
  printf("\n");
  printf("Usage2:    get_plcselect <index> <parameter>\n");
  printf("index:     0..%d (0=WBM, 1=PLC select list)\n", MAX_PLC_NUMBER-1);
  printf("parameter: url | txt | vkb | mon |mic\n");
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

/// \brief get number of configured PLCs
/// \retval number of PLCs
int GetPlcCount()
{
  int i;
  int iPlcCounter = 0;
  char szUrl[64] = "";
  char szTxt[64] = "";
  char szOutUrl[256] = "";
  char szOutTxt[256] = "";
  for (i=2; i < MAX_PLC_NUMBER; i++)
  {
    //check lines url1= url2= url3= etc.
    //check lines txt1= txt2= txt3= etc.
    
    sprintf(szUrl, "url%02d", i);  
    sprintf(szTxt, "txt%02d", i);  
  
    szOutUrl[0] = 0;
    szOutTxt[0] = 0;
  
    if (ConfGetValue(g_pList, szUrl, &szOutUrl[0], sizeof(szOutUrl)) == SUCCESS)
    {
      if (ConfGetValue(g_pList, szTxt, &szOutTxt[0], sizeof(szOutTxt)) == SUCCESS)
      {
        if ((strlen(szOutUrl) > 0) && (strlen(szOutTxt) > 0))
        {
          iPlcCounter++;
        }       
      }               
    }                
             
  }
  return iPlcCounter;
}

