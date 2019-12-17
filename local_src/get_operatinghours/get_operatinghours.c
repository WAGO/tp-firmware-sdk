//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019 WAGO Kontakttechnik GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
///
/// \file    get_operatinghours.c
///
/// \version $Id: get_operatinghours.c 43946 2019-10-23 11:10:18Z wrueckl_elrest $
///
/// \brief   get operating hours / config-tools
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <msgtool.h>
#include "config_tool_lib.h"


//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

#define TEMPERATURE_DATA_FILE                "/tmp/tempdata.dat"

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

void ShowHelpText();
unsigned int CalculateOperatingMinutes();
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
      
      ret = ConfReadValues(g_pList, TEMPERATURE_DATA_FILE);
      if (ret != SUCCESS)
      {
        char szTxt[256];
        sprintf(szTxt, "File open error %s", TEMPERATURE_DATA_FILE);
        setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt);

        AppendErrorText(FILE_OPEN_ERROR, TEMPERATURE_DATA_FILE);
        //terminate
        ConfDestroyList(g_pList);
        return ERROR;
      }
      
      if (ConfGetCount(g_pList) == 0)
      {
        char szTxt[256];
        sprintf(szTxt, "File inconsistent error %s", TEMPERATURE_DATA_FILE);
        setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt);
        
        AppendErrorText(CONFIG_FILE_INCONSISTENT, TEMPERATURE_DATA_FILE);
        //terminate
        ConfDestroyList(g_pList);
        return ERROR;
      }
      
      if ((strcmp(argv[1], "-a") == 0) || (strcmp(argv[1], "all") == 0) || (strcmp(argv[1], "all-json") == 0))
      {
        char szJson[4096] = "";
        char szLine[512] = "";
        int iLen;
         //print all values in JSON format
        strcat(szJson, "{");
         
        //printf("total=%u\n", CalculateOperatingHours()); 
        sprintf(szLine, "\"total\": \"%u\",", CalculateOperatingMinutes());          
        strcat(szJson, szLine);                

        //range < 10 minus
        if (ConfGetValue(g_pList, "bs_counter.dwRange_10m", &szOut[0], sizeof(szOut)) == SUCCESS)
        {
          //printf("range_10m=%s\n", szOut);
          sprintf(szLine, "\"range_10m\": \"%s\",", szOut);          
          strcat(szJson, szLine); 
        }          

        //range 10 minus to 0
        if (ConfGetValue(g_pList, "bs_counter.dwRange_10_0", &szOut[0], sizeof(szOut)) == SUCCESS)
        {
          //printf("range_10_0=%s\n", szOut);
          sprintf(szLine, "\"range_10_0\": \"%s\",", szOut);          
          strcat(szJson, szLine);
        } 

        //range 0 to 10
        if (ConfGetValue(g_pList, "bs_counter.dwRange_0_10", &szOut[0], sizeof(szOut)) == SUCCESS)
        {
          //printf("range_0_10=%s\n", szOut);
          sprintf(szLine, "\"range_0_10\": \"%s\",", szOut);          
          strcat(szJson, szLine);
        } 

        //range 10 to 20
        if (ConfGetValue(g_pList, "bs_counter.dwRange_10_20", &szOut[0], sizeof(szOut)) == SUCCESS)
        {
          //printf("range_10_20=%s\n", szOut);
          sprintf(szLine, "\"range_10_20\": \"%s\",", szOut);          
          strcat(szJson, szLine);
        } 

        //range 20 to 30
        if (ConfGetValue(g_pList, "bs_counter.dwRange_20_30", &szOut[0], sizeof(szOut)) == SUCCESS)
        {
          //printf("range_20_30=%s\n", szOut);
          sprintf(szLine, "\"range_20_30\": \"%s\",", szOut);          
          strcat(szJson, szLine);
        } 

        //range 30 to 40
        if (ConfGetValue(g_pList, "bs_counter.dwRange_30_40", &szOut[0], sizeof(szOut)) == SUCCESS)
        {
          //printf("range_30_40=%s\n", szOut);
          sprintf(szLine, "\"range_30_40\": \"%s\",", szOut);          
          strcat(szJson, szLine);
        } 

        //range 40 to 50
        if (ConfGetValue(g_pList, "bs_counter.dwRange_40_50", &szOut[0], sizeof(szOut)) == SUCCESS)
        {
          //printf("range_40_50=%s\n", szOut);
          sprintf(szLine, "\"range_40_50\": \"%s\",", szOut);          
          strcat(szJson, szLine);
        } 

        //range 50 to 60
        if (ConfGetValue(g_pList, "bs_counter.dwRange_50_60", &szOut[0], sizeof(szOut)) == SUCCESS)
        {
          //printf("range_50_60=%s\n", szOut);
          sprintf(szLine, "\"range_50_60\": \"%s\",", szOut);          
          strcat(szJson, szLine);
        } 

        //range 60 to 70
        if (ConfGetValue(g_pList, "bs_counter.dwRange_60_70", &szOut[0], sizeof(szOut)) == SUCCESS)
        {
          //printf("range_60_70=%s\n", szOut);
          sprintf(szLine, "\"range_60_70\": \"%s\",", szOut);          
          strcat(szJson, szLine);
        } 

        //range > 70
        if (ConfGetValue(g_pList, "bs_counter.dwRange_70p", &szOut[0], sizeof(szOut)) == SUCCESS)
        {
          //printf("range_70p=%s\n", szOut);
          sprintf(szLine, "\"range_70p\": \"%s\"", szOut);          
          strcat(szJson, szLine);
        } 
        
        ConfRemoveJsonSeparatorFromEnd(&szJson[0]);
        
        strcat(szJson, "}");
        printf(szJson);
                
        status = SUCCESS;
      }
      else
      {
      
        for (i = 1; i < argc; i++)
        {
          pStr = argv[i];
          if (stricmp(argv[i], "total") == 0)
          {
            //total amount of operating minutes
            //printf("total=%u\n", CalculateOperatingHours());
            printf("%u", CalculateOperatingMinutes());
            status = SUCCESS;
          }
          else if (stricmp(argv[i], "range_10m") == 0)
          {
            //range < 10 minus
            if (ConfGetValue(g_pList, "bs_counter.dwRange_10m", &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", argv[i], szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }          
          }
          else if (stricmp(argv[i], "range_10_0") == 0)
          {
            //range 10 minus to 0
            if (ConfGetValue(g_pList, "bs_counter.dwRange_10_0", &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", argv[i], szOut);
              printf("%s", szOut);
              status = SUCCESS;
            } 
          }
          else if (stricmp(argv[i], "range_0_10") == 0)
          {
            //range 0 to 10
            if (ConfGetValue(g_pList, "bs_counter.dwRange_0_10", &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", argv[i], szOut);
              printf("%s", szOut);
              status = SUCCESS;
            } 
          }
          else if (stricmp(argv[i], "range_10_20") == 0)
          {
            //range 10 to 20
            if (ConfGetValue(g_pList, "bs_counter.dwRange_10_20", &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", argv[i], szOut);
              printf("%s", szOut);
              status = SUCCESS;
            } 
          }
          else if (stricmp(argv[i], "range_20_30") == 0)
          {
            //range 20 to 30
            if (ConfGetValue(g_pList, "bs_counter.dwRange_20_30", &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", argv[i], szOut);
              printf("%s", szOut);
              status = SUCCESS;
            } 
          }
          else if (stricmp(argv[i], "range_30_40") == 0)
          {
            //range 30 to 40
            if (ConfGetValue(g_pList, "bs_counter.dwRange_30_40", &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", argv[i], szOut);
              printf("%s", szOut);
              status = SUCCESS;
            } 
          }
          else if (stricmp(argv[i], "range_40_50") == 0)
          {
            //range 40 to 50
            if (ConfGetValue(g_pList, "bs_counter.dwRange_40_50", &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", argv[i], szOut);
              printf("%s", szOut);
              status = SUCCESS;
            } 
          }
          else if (stricmp(argv[i], "range_50_60") == 0)
          {
            //range 50 to 60
            if (ConfGetValue(g_pList, "bs_counter.dwRange_50_60", &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", argv[i], szOut);
              printf("%s", szOut);
              status = SUCCESS;
            } 
          }
          else if (stricmp(argv[i], "range_60_70") == 0)
          {
            //range 60 to 70
            if (ConfGetValue(g_pList, "bs_counter.dwRange_60_70", &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", argv[i], szOut);
              printf("%s", szOut);
              status = SUCCESS;
            } 
          }
          else if (stricmp(argv[i], "range_70p") == 0)
          {
            //range > 70
            if (ConfGetValue(g_pList, "bs_counter.dwRange_70p", &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", argv[i], szOut);
              printf("%s", szOut);
              status = SUCCESS;
            } 
          }
        }
      }
      
      //free list memory
      ConfDestroyList(g_pList);
    }
  }

  return status;
}

/// \brief Print help text / usage to stdout
void ShowHelpText()
{
  printf("\n* Get operating hours * \n\n");
  printf("Usage:      get_operatinghours [options] <parameter>\n");
  printf("\n");
  printf("return value: operating time in minutes\n");
  printf("\n");
  printf("options:\n");
  printf("all-json print all values in JSON format \n"); 
  printf("-h print this help\n");
  printf("\n");
  printf("parameter:  total |\n");
  printf("            range_10m | range_10_0 | range_0_10 |\n");
  printf("            range_10_20 | range_20_30 | range_30_40 |\n");
  printf("            range_40_50 | range_50_60 | range_60_70 |\n");
  printf("            range_70p \n");
  printf("\n");
}

/// \brief get total operating time (minutes)
/// \retval operating minutes
unsigned int CalculateOperatingMinutes()
{
  unsigned int minutes = 0;
  char szOut[256] = "";
  int ret;
  
  if (ConfGetValue(g_pList, "bs_counter.dwRange_10m", &szOut[0], sizeof(szOut)) == SUCCESS)
  {
    ret = atoi(szOut);
    if (ret > 0)
      minutes += ret;
  }   
  
  if (ConfGetValue(g_pList, "bs_counter.dwRange_10_0", &szOut[0], sizeof(szOut)) == SUCCESS)
  {
    ret = atoi(szOut);
    if (ret > 0)
      minutes += ret;
  }   
  
  if (ConfGetValue(g_pList, "bs_counter.dwRange_0_10", &szOut[0], sizeof(szOut)) == SUCCESS)
  {
    ret = atoi(szOut);
    if (ret > 0)
      minutes += ret;
  }  
  
  if (ConfGetValue(g_pList, "bs_counter.dwRange_10_20", &szOut[0], sizeof(szOut)) == SUCCESS)
  {
    ret = atoi(szOut);
    if (ret > 0)
      minutes += ret;
  }  
  
  if (ConfGetValue(g_pList, "bs_counter.dwRange_20_30", &szOut[0], sizeof(szOut)) == SUCCESS)
  {
    ret = atoi(szOut);
    if (ret > 0)
      minutes += ret;
  }  
  
  if (ConfGetValue(g_pList, "bs_counter.dwRange_30_40", &szOut[0], sizeof(szOut)) == SUCCESS)
  {
    ret = atoi(szOut);
    if (ret > 0)
      minutes += ret;
  }  
  
  if (ConfGetValue(g_pList, "bs_counter.dwRange_40_50", &szOut[0], sizeof(szOut)) == SUCCESS)
  {
    ret = atoi(szOut);
    if (ret > 0)
      minutes += ret;
  }
  
  if (ConfGetValue(g_pList, "bs_counter.dwRange_50_60", &szOut[0], sizeof(szOut)) == SUCCESS)
  {
    ret = atoi(szOut);
    if (ret > 0)
      minutes += ret;
  }    
  
  if (ConfGetValue(g_pList, "bs_counter.dwRange_60_70", &szOut[0], sizeof(szOut)) == SUCCESS)
  {
    ret = atoi(szOut);
    if (ret > 0)
      minutes += ret;
  }  
  
  if (ConfGetValue(g_pList, "bs_counter.dwRange_70p", &szOut[0], sizeof(szOut)) == SUCCESS)
  {
    ret = atoi(szOut);
    if (ret > 0)
      minutes += ret;
  }  
  
  return minutes;
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

