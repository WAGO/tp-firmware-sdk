//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019-2022 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
///
///  \file    get_audio.c
///
///  \version $Id: get_audio.c 65689 2022-03-11 14:37:43Z falk.werner@wago.com $
///
///  \brief   get audio settings / config-tools
///
///  \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "msgtool.h"
#include "config_tool_lib.h"

//------------------------------------------------------------------------------
// Local macros
//------------------------------------------------------------------------------

#define CNF_FILE            AUDIO_CONF_FILE

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
int GetVolumeValue();

/// \brief main function
/// \param[in]  argc number of arguments
/// \param[in]  argv arguments
/// \retval 0  SUCCESS
int main (int argc, char **argv)
{
  int status = ERROR;
  int iVolume = 0;
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
        system("/etc/config-tools/config_audio resettofactory > /dev/null 2>&1");  
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
        
        if (FileExistsWithoutSizeCheck("/dev/snd/controlC0") == SUCCESS)
        {
          sprintf(szLine, "\"state\": \"enabled\",");
          strcat(szJson, szLine);
          //printf("state=enabled\n");
        }
        else
        {
          sprintf(szLine, "\"state\": \"disabled\",");
          strcat(szJson, szLine);
          //printf("state=disabled\n");
        } 
        
        if (ConfGetValue(g_pList, "path", &szOut[0], sizeof(szOut)) == SUCCESS)                       
        {
          sprintf(szLine, "\"path\": \"%s\",", szOut);
          strcat(szJson, szLine);
          //printf("path=%s\n", szOut);  
        }
        
        //read volume using amixer
        iVolume = GetVolumeValue();
        if (iVolume >= 0)
        {
          sprintf(szLine, "\"volume\": \"%d\",", iVolume);
          strcat(szJson, szLine);
          //printf("volume=%d\n", iVolume);
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
          
          if (stricmp(pStr, "state") == 0) 
          {
            //check if audio is available
            // /dev/snd/controlC0 
            // /dev/snd/pcmC0D0c 
            // /dev/snd/pcmC0D0p 
            // /dev/snd/timer 

            if (FileExistsWithoutSizeCheck("/dev/snd/controlC0") == SUCCESS)
            {
              strcpy(szOut, "enabled");               
            }
            else
            {
              strcpy(szOut, "disabled");  
            }                        
            printf("%s", szOut);
            status = SUCCESS;
            
          }
          else if (stricmp(pStr, "volume") == 0) 
          {
            iVolume = GetVolumeValue();
            if (iVolume >= 0)
            {
              printf("%d", iVolume);
              status = SUCCESS;
            }            
            else
            {
              status = ERROR;
            }
          }
          else if (stricmp(pStr, "path") == 0) 
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
  printf("\n* Get audio settings * \n\n");
  printf("Usage: get_audio [options] <parameter>\n\n");
  printf("options:\n");
  printf("all-json print all values in JSON format \n"); 
  printf("-h print this help \n\n");
  printf("parameter: state | volume | path \n\n");
  
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

/// \brief Read audio volume value 
///
/// The function returns the current volume value,
/// if an error occurs the function returns -1
///
/// \retval [0..127]  volume value 
/// \retval -1        ERROR
int GetVolumeValue()
{
  int status = ERROR;
  char szOut[64] = "";
  char c;
  
  system("amixer get Headphone | grep \"Front Left: Playback\" | cut -d' ' -f6 > /tmp/volume_tmp.txt");
  if (FileExists("/tmp/volume_tmp.txt") == SUCCESS)
  {
    int fd = open("/tmp/volume_tmp.txt", O_RDONLY);
    if (fd >= 0)
    {
      if (read(fd, szOut, sizeof(szOut)) > 0)  
      { 
        //remove line feed etc.       
        while (strlen(szOut) > 0)
        {          
          c = szOut[strlen(szOut)-1];
          if ((c < 0x30) || (c > 0x39))
          {
            szOut[strlen(szOut)-1] = '\0';
          }
          else
            break;
        }
        
        if (ConfIsNumber(szOut) == SUCCESS)
        {
          status = atoi(szOut);                
        }
      }
      close(fd);
      system("rm /tmp/volume_tmp.txt");
    }    
  }  
  
  return status;
}

