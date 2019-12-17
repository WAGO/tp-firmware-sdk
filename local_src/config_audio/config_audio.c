//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019 WAGO Kontakttechnik GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
/// \file     config_audio.c
///
/// \version $Id: config_audio.c 43946 2019-10-23 11:10:18Z wrueckl_elrest $
///
/// \brief    audio change settings / config-tools
///
/// \author   Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "msgtool.h"
#include "config_tool_lib.h"

//------------------------------------------------------------------------------
// Local macros
//------------------------------------------------------------------------------

#define CNF_FILE            AUDIO_CONF_FILE
#define SND_DEVICE          "/dev/snd/controlC0"

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
  { "path", "/home/user/audio", "" },
  { "volume", "80", "" },
  
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
int SetVolumeValue(int iVolume);

/// \brief main function
/// \param[in]  argc number of arguments
/// \param[in]  argv arguments
/// \retval 0  SUCCESS
int main (int argc, char **argv)
{
  int status = ERROR;
  int restart_service = 0;
  char szArgv[256] = "";
  int k = 0;
 
  if (argc >= 2)
  {
    if ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0))
    {
      ShowHelpText();
      status = SUCCESS;
    }
    else if (stricmp(argv[1], "resettofactory") == 0) 
    {
      //reset to factory
      if (ConfResetToDefaultValues(&aConfigValues[0], CNF_FILE) == SUCCESS)
        status = SUCCESS;
      
      //SetVolume to default value
      k=0;
      while (aConfigValues[k].nameStr[0])
      {  
        if (stricmp(aConfigValues[k].nameStr, "volume") == 0)
        {
          SetVolumeValue(atoi(aConfigValues[k].defaultStr));
          break;
        }
        k++;
      }
      
    }
    else
    {
      int i, ret;
      char * pStr;
      char szSearchStr[128] = "";
      int len = 0;
      
      //check if audio is available
      // /dev/snd/controlC0 
      // /dev/snd/pcmC0D0c 
      // /dev/snd/pcmC0D0p 
      // /dev/snd/timer 

      if (FileExistsWithoutSizeCheck(SND_DEVICE) != SUCCESS)
      {
        //LED
        char szErrTxt[256];
        sprintf(szErrTxt, "File open error: %s", SND_DEVICE);
        setRgbLed(RGB_LED_STATE_RE_BLINK, szErrTxt);
        AppendErrorText(FILE_OPEN_ERROR, SND_DEVICE);
        return ERROR;
      }
     
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
        //LED
        char szErrTxt[256];
        sprintf(szErrTxt, "File open error: %s", CNF_FILE);
        setRgbLed(RGB_LED_STATE_RE_BLINK, szErrTxt);
        
        AppendErrorText(FILE_OPEN_ERROR, CNF_FILE);
        //terminate
        ConfDestroyList(g_pList);
        return ERROR;
      }
      
      if (ConfGetCount(g_pList) == 0)
      {
        //LED
        char szErrTxt[256];
        sprintf(szErrTxt, "File inconsistent error: %s", CNF_FILE);
        setRgbLed(RGB_LED_STATE_RE_BLINK, szErrTxt);
        
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
                        
            if (stricmp(aConfigValues[k].nameStr, "volume") == 0)
            { 
              status = ERROR;
              if (ConfIsNumber(pStr) == SUCCESS)
              {
                //set new volume permanently
                //amixer set Digital (0 - 127)
                //alsactl store 0 
                if (SetVolumeValue(atoi(pStr)) == SUCCESS)
                {
                  ConfSetValue(&aConfigValues[0], g_pList, aConfigValues[k].nameStr, pStr);
                  status = SUCCESS;                                                  
                }
              }
              else
              {
                setRgbLed(RGB_LED_STATE_RE_BLINK, "Audio volume is not valid");
                AppendErrorText(INVALID_PARAMETER, "Audio volume is not valid");
                status = INVALID_PARAMETER;  
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
  printf("\n* Audio settings * \n\n");
  printf("Usage: config_audio [volume=volume-value] [path=path-value]\n\n");
  printf("volume-value: 0..100\n");
  printf("path-value: /home/user/audiofiles \n\n");  
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

/// \brief Set audio volume value 
///
/// The function sets the audio volume value,
/// if an error occurs the function returns -1
///
/// \param[in]        iVolume    [0..127]        
/// \retval -1        ERROR
/// \retval 0         SUCCESS
int SetVolumeValue(int iVolume)
{
  int status = ERROR;
  char szCmd[128] = "";
  
  // first call 
  //sprintf(szCmd, "amixer set Digital %d", MIN(iVolume, 127));
  sprintf(szCmd, "amixer -c 0 sset Headphone,0 %d", MIN(iVolume, 127));
  if ( system(szCmd) == 0 )
  {
    status = SUCCESS;
  }
  
  if (status == SUCCESS)
  {
    // second call, save permanently
    sprintf(szCmd, "alsactl store 0");
    system(szCmd);    
  }
  
  return status;
}


