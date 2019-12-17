//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019 WAGO Kontakttechnik GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
/// \file    config_touchbeeper.c
///
/// \version $Id: config_touchbeeper.c 43946 2019-10-23 11:10:18Z wrueckl_elrest $
///
/// \brief   touchbeeper change settings / config-tools
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <unistd.h>
#include <sys/types.h>

#include "msgtool.h"
#include "config_tool_lib.h"

//------------------------------------------------------------------------------
// Local macros
//------------------------------------------------------------------------------

#define  CNF_FILE            TOUCHBEEPER_CONF_FILE
#define  SYS_PATH_BEEPER     "/sys/class/leds/beeper/"

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
  { "state", "disabled", "enabled,disabled" },

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
int EnableTouchBeeper(int enable);

/// \brief main function
/// \param[in]  argc number of arguments
/// \param[in]  argv arguments
/// \retval 0  SUCCESS
int main (int argc, char **argv)
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
            if (ConfSetValue(&aConfigValues[0], g_pList, aConfigValues[k].nameStr, pStr)==SUCCESS)
            {
              restart_service = 1;                       
              status = SUCCESS;
            }
            break;
          }        
          k++;
        }        
        
        if (stricmp(szArgv, "init")==0)
        {          
          restart_service = 1;                       
          status = SUCCESS;          
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
      
      if (restart_service)
      {
        char szState[128] = "";
        
        status = ERROR;
        if (ConfGetValue(g_pList, "state", &szState[0], sizeof(szState)) == SUCCESS)
        {
          status = EnableTouchBeeper(!stricmp(szState, "enabled"));
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
  printf("\n* Touchbeeper settings * \n\n");
  printf("Usage: config_touchbeeper [state=state-value]\n\n");
  printf("state-value: enabled | disabled\n");
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


/// \brief Enable touch beeper driver
///
/// The function enables beeper driver
/// if an error occurs the function returns -1
///
/// \param[in] enable	  guess what
/// \retval  0 SUCCESS
/// \retval -1 ERROR
int EnableTouchBeeper(int enable)
{
  int ret;

  WriteSysFs(SYS_PATH_BEEPER, "trigger", "oneshot");
  if (enable)
    ret = WriteSysFs(SYS_PATH_BEEPER, "delay_on", "100");
  else
    ret = WriteSysFs(SYS_PATH_BEEPER, "delay_on", "0");

  return ret;
}
