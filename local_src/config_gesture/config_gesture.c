//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019 WAGO Kontakttechnik GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
/// \file     config_gesture.c
///
/// \version $Id: config_gesture.c 44064 2019-10-24 12:34:47Z wrueckl_elrest $
///
/// \brief    set gesture settings
///
/// \author   Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "msgtool.h"
#include "config_tool_lib.h"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

#define CNF_FILE            GESTURE_CONF_FILE

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

void ShowHelpText();
void AppendErrorText(int iStatusCode, char * pText);

//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// variable definitions
//------------------------------------------------------------------------------

/// configuration list start pointer
tConfList * g_pList = NULL;

/// \brief Config data in form of name, default value, value list comma separated (no spaces)
static tConfValues aConfigValues[] =
{
  { "state", "enabled", "enabled,disabled" },
  { "scroll", "yes", "yes,no" },
  { "menu", "yes", "yes,no" },
  { "startrange", "400", "" },
  { "arraysize", "60", "" },
  { "movelength", "24", "" },

  { "menu-timeout", "5", "" },
  { "menu-margin-left", "10", "" },
  { "menu-margin-right", "90", "" },  
  { "menu-orientation", "top", "top,right,bottom,left" },

  { "btn0", "1", "" },
  { "id0", "ID_PLCLIST", "" },  
  { "btn1", "1", "" },
  { "id1", "ID_DISPLAYCLEANING", "" },    
    
  // this line must always be the last one - don't remove it!
  { "", "" }
};

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

/// \brief Print help text / usage to stdout
void ShowHelpText()
{
  printf("\n* Gesture settings * \n\n");
  printf("Usage: config_gesture [state=state-value] [scroll=scroll-value] [menu=menu-value]\n\n");
  printf("state-value: enabled | disabled\n");
  printf("scroll-value: yes | no \n");  
  printf("menu-value: yes | no \n\n");  
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
      
      if (restart_service)
      {
        //f.e. restart 
        //TODO 
        //Displayorientierung umschalten
        //Pflegemodus aktivieren / deaktivieren
      }
            
    }
  }

  //clean up list memory
  ConfDestroyList(g_pList);

  return status;
}

