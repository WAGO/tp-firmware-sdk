//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019 WAGO Kontakttechnik GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
/// \file     config_motionsensor.c
///
/// \version $Id: config_motionsensor.c 43946 2019-10-23 11:10:18Z wrueckl_elrest $
///
/// \brief    motionsensor change settings / config-tools
///
/// \author   Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include <glib.h>

#include "msgtool.h"
#include "config_tool_lib.h"

//------------------------------------------------------------------------------
// Local macros
//------------------------------------------------------------------------------

#define CNF_FILE           MOTION_CONF_FILE

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
  { "state", "enabled", "enabled,disabled" },
  { "direction", "all", "left,right,all"  },

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

/// \brief main function
/// \param[in]  argc number of arguments
/// \param[in]  argv arguments
/// \retval 0  SUCCESS
int main (int argc, char **argv)
{
  int status = ERROR;
  char szValue[128] = "";                
  int startsensor = 0;
 
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
      int i, ret;
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
        char *key, *value;
        // URL decoding
        if ((key = g_uri_unescape_string(argv[i], ""))) {
	  if ((value = strchr(key, '='))) {
            *value++ = 0;
            status = ConfSetValue(aConfigValues, g_pList, key, value);
          }
	  g_free(key);
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

      system("killall brightness_control && sleep 1 && brightness_control");
    }
  }

  //clean up list memory
  ConfDestroyList(g_pList);

  return status;
}

/// \brief Print help text / usage to stdout
void ShowHelpText()
{
  printf("\n* Motion sensor settings * \n\n");
  printf("Usage: config_motionsensor [state=state-value] [direction=direction-value]\n");
  printf("\n");
  printf("state-value:      enabled | disabled\n");
  printf("direction-value:  right | left | all\n");
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

