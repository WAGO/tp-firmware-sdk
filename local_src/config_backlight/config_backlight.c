//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019 WAGO Kontakttechnik GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
///
/// \file    config_backlight.c
///
/// \version $Id: config_backlight.c 53179 2020-10-30 09:38:53Z wrueckl_elrest $
///
/// \brief   change backlight settings / config-tools
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
#include <sys/types.h>
#include <glib.h>
#include "msgtool.h"
#include "config_tool_lib.h"


//------------------------------------------------------------------------------
// Local macros
//------------------------------------------------------------------------------

#define CNF_FILE  BACKLIGHT_CONF_FILE

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
//tMsg2Server gMsg;

/// \brief Config data in form of name, default value, value list comma separated (no spaces)
static tConfValues aConfigValues[] =
{
  { "backlighton", "255", "" },
  { "backlightonnight", "50", "" },
  { "backlightoff", "0", "" },
  { "backlightoffnight", "0", "" },  
  { "mastermode", "none", "master,slave,none" },
  { "daynightswitch", "disabled", "enabled,disabled" },
  { "daytime", "06:00:00", "" },
  { "nighttime", "21:00:00", "" },
  { "usesensor", "no", "yes,no" },
  { "sensorranges", "0,264,50,265,65535,255", "" },
  { "testbacklight", "2", "" },
  { "plusminusstep", "10", "" },  
  { "hysteresisdiff", "15", "" },  
  // this line must always be the last one - don't remove it!
  { "", "", "" }
};

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
int GetBrightnessControlActivity();
int IsTimeStringValid(char * pTimeStr);
int IsBacklightValid(char * pStr);

/// \brief main function
/// \param[in]  argc number of arguments
/// \param[in]  argv arguments
/// \retval 0  SUCCESS
int main(int argc, char **argv)
{
  int status = ERROR;
  int restart_service = 0;
  char szArgv[256] = "";
 
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
    else
    {
      int i, k, ret;
      char * pStr;
      char szSearchStr[128] = "";
      int len = 0;
      
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
        sprintf(szTxt, "File open error: %s", CNF_FILE);
        setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt); 
        
        AppendErrorText(FILE_OPEN_ERROR, CNF_FILE);
        //terminate
        ConfDestroyList(g_pList);
        return ERROR;
      }
      
      if (ConfGetCount(g_pList) == 0)
      {
        char szTxt[256];
        sprintf(szTxt, "File inconsistent error: %s", CNF_FILE);
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
            
            if (stricmp(aConfigValues[k].nameStr, "daytime") == 0)
            { 
              if (IsTimeStringValid(pStr) == SUCCESS)
              {
                if (ConfSetValue(&aConfigValues[0], g_pList, aConfigValues[k].nameStr, pStr)==SUCCESS)
                {
                  status = SUCCESS;
                }                
              }
              else
              {
                status = ERROR;
              }
            }
            else if (stricmp(aConfigValues[k].nameStr, "nighttime") == 0)
            { 
              if (IsTimeStringValid(pStr) == SUCCESS)
              {
                if (ConfSetValue(&aConfigValues[0], g_pList, aConfigValues[k].nameStr, pStr)==SUCCESS)
                {
                  status = SUCCESS;
                }                
              }
              else
              {
                status = ERROR;
              }
            }
            else if ((stricmp(aConfigValues[k].nameStr, "backlighton") == 0) || \
                     (stricmp(aConfigValues[k].nameStr, "backlightoff") == 0) || \
                     (stricmp(aConfigValues[k].nameStr, "backlightonnight") == 0) || \
                     (stricmp(aConfigValues[k].nameStr, "backlightoffnight") == 0))
            { 
              if (IsBacklightValid(pStr) == SUCCESS)
              {
                if (ConfSetValue(&aConfigValues[0], g_pList, aConfigValues[k].nameStr, pStr)==SUCCESS)
                {
                  status = SUCCESS;
                }                
              }
              else
              {
                status = ERROR;
              }
            }
            else
            {
              if (ConfSetValue(&aConfigValues[0], g_pList, aConfigValues[k].nameStr, pStr)==SUCCESS)
              {
                status = SUCCESS;
              }              
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
          restart_service = 1;
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
      
      if (restart_service)
      {
        //f.e. restart application brightness control     
        system("/etc/config-tools/brightnesscontrol restart > /dev/null 2>&1");                
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
  printf("Usage: config_backlight [backlighton=0..255] [backlightoff=0..255]\n");
  printf("                        [backlightonnight=0..255] [backlightoffnight=0..255]\n");
  printf("                        [daynightswitch=daynightswitch-value]\n");
  printf("                        [nighttime=nighttime-value]\n");
  printf("                        [daytime=daytime-value]\n");
  printf("                        [mastermode=mastermode-value]\n");
  printf("                        [usesensor=usesensor-value]\n");
  printf("                        [sensorranges=sensorranges-value]\n");
  printf("                        [plusminusstep=plusminusstep-value]\n\n");
  printf("daynightswitch-value:   enabled | disabled\n");
  printf("mastermode-value:       master | slave | none\n");
  printf("usesensor-value:        yes | no\n");
  printf("sensorranges-value:     comma separated from,to,value,from,to,value etc.\n");
  printf("                        f.e. 0,50,30,50,100,70,100,255,234\n");  
  printf("plusminusstep-value:    1..255 \n\n");

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
      status = atoi(pOutput);
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

/// \brief Read if brightness_control is active at this moment 
///
/// The function returns the current brightness_control activity
///
/// \retval  0 brightness_control is running 
/// \retval -1 brightness_control is not running 
int GetBrightnessControlActivity()
{
  int status = ERROR;  
  if (system("pidof brightness_control > /dev/null 2>&1") == 0)
  {
    status = SUCCESS;  
  }
  return status;
}

/// \brief Check if time string format is correct 
///
/// \param[in]  pTimeStr        time string
/// \retval  0  string is correct
/// \retval -1  string is not correct
int IsTimeStringValid(char * pTimeStr)
{
  int status = ERROR;  
  char * pStr = pTimeStr;
   
  if (strlen(pStr) == 8)
  {
    if ((pStr[2]==':') && (pStr[5]==':'))
    {
      char szHour[10];
      char szMin[10];
      char szSec[10];
      strncpy(szHour, pStr, 2);    
      szHour[2] = 0;    
      pStr+=3;
      
      //printf("szHour %s\n", szHour);
      
      strncpy(szMin, pStr, 2);
      szMin[2] = 0;    
      pStr+=3;
      
      //printf("szMin %s\n", szMin);
      
      strncpy(szSec, pStr, 2);
      szSec[2] = 0;    
      
      //printf("szSec %s\n", szSec);
      
      if (ConfIsNumber(szHour) == SUCCESS)
      { 
        int n = atoi(szHour);
        if ((n >= 0) && (n < 24))
        {
          if (ConfIsNumber(szMin) == SUCCESS)
          {
            n = atoi(szMin);
            if ((n >= 0) && (n < 60))
            {
              if (ConfIsNumber(szSec) == SUCCESS)
              {
                 n = atoi(szSec);
                 if ((n >= 0) && (n < 60))
                 {
                   status = SUCCESS;  
                 }
              }            
            }            
          }          
        }        
      }    
    }
  }

  if (status == ERROR)
    AppendErrorText(INVALID_PARAMETER, "Wrong time format, use hh:mm:ss f.e. 23:59:59");
      
  return status;    
}

/// \brief Check if backlight value is valid
///
/// The function checks the current backlight value,
/// if an error occurs the function returns -1
///
/// \param[in]  pStr        backlight value 0..255
/// \retval  0 SUCCESS
/// \retval -1 ERROR
int IsBacklightValid(char * pStr)
{
  int iRet = ERROR;
  if (ConfIsNumber(pStr) == SUCCESS)
  {
    long int iValue = strtol(pStr, NULL, 10);
    if ((iValue >= 0) && (iValue <=255))
    {
      iRet = SUCCESS;
    }
  }
  return iRet;
}
