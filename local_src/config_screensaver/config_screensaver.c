//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019-2022 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
/// \file    config_screensaver.c
///
/// \version $Id: config_screensaver.c 65689 2022-03-11 14:37:43Z falk.werner@wago.com $
///
/// \brief   screensaver change settings / config-tools
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <glib.h>

#include "msgtool.h"
#include "config_tool_lib.h"


//------------------------------------------------------------------------------
// Local macros
//------------------------------------------------------------------------------

#define  CNF_FILE                  SCREEN_CONF_FILE
#define  MIN_SCREENSAVER_TIME      20  
#define  MAX_SCREENSAVER_TIME      86400  

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
  { "mode", "theme", "backlight,theme" },
  { "theme", "1", "" },
  { "image", "/home/user/logo.png", "" },
  { "text", "screensaver example text", "" },
  { "time", "600", "" },
  { "png_src_dir", "/tmp/png_uploads", "" },
  { "png_dest_dir", "/home/user", "" },

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
int GetPngDirs(char * src_buf, int src_buflen, char * dest_buf, int dest_buflen);
int IsPngFile(char * pPngStr);

/// \brief main function
/// \param[in]  argc number of arguments
/// \param[in]  argv arguments
/// \retval 0  SUCCESS
int main (int argc, char **argv)
{
  int status = ERROR;
  int restart_service = 0;
  char szArgv[256] = "";
  char szPngSrc[256] = "";
  char szPngDest[256] = "";
  int skipBCrestart = 0;
 
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
      
      if (GetPngDirs(szPngSrc, sizeof(szPngSrc), szPngDest, sizeof(szPngDest)) != SUCCESS)
      {
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
            
            if (stricmp(aConfigValues[k].nameStr, "time") == 0)
            {
              if (ConfIsNumber(pStr) == SUCCESS)
              {
                char szTmp[128] = "";
                int number = atoi(pStr);
                if (number < MIN_SCREENSAVER_TIME)
                {
                  number = MIN_SCREENSAVER_TIME;
                }
                if (number > MAX_SCREENSAVER_TIME)
                {
                  number = MAX_SCREENSAVER_TIME;
                }
                sprintf(szTmp,"%d", number);
                if (ConfSetValue(NULL, g_pList, aConfigValues[k].nameStr, szTmp)==SUCCESS)
                {
                  status = SUCCESS;
                }
              }
            }            
            else if (stricmp(aConfigValues[k].nameStr, "theme") == 0)
            {
              if (ConfIsNumber(pStr) == SUCCESS)
              {
                if (ConfSetValue(NULL, g_pList, aConfigValues[k].nameStr, pStr)==SUCCESS)
                {
                  status = SUCCESS;
                }
              }
            }   
            else if (stricmp(aConfigValues[k].nameStr, "image") == 0)
            {
              char szImageFileName[256] = "";
              if (strchr(pStr, '/') == NULL)
              {
                snprintf(szImageFileName, sizeof(szImageFileName), "%s%s", szPngDest, pStr);
              }
              else
              {
                snprintf(szImageFileName, sizeof(szImageFileName), "%s", pStr);
              }
              if (FileExists(szImageFileName) == SUCCESS)
              {
                if (ConfSetValue(NULL, g_pList, aConfigValues[k].nameStr, szImageFileName)==SUCCESS)
                {
                  status = SUCCESS;
                }
              }
            }
            else if (ConfSetValue(&aConfigValues[0], g_pList, aConfigValues[k].nameStr, pStr)==SUCCESS)
            {
              status = SUCCESS;
            }
            break;
          }        
          k++;
        }        

    }

    //add / remove png file
    if (status != SUCCESS)
    {
      if (stricmp(argv[1], "add") == 0) 
      {
        // install a new userdefined png file
        if (argc >= 3)
        {
          char * pSub;
          char szPngFile[256] = "";
          char * pFile = szPngFile;
          // URL decoding       
          strncpy(szPngFile, g_uri_unescape_string(argv[2], ""), sizeof(szPngFile));                       
                                          
          pSub = strrchr(pFile, '/');
          if (pSub)
          {
            pSub++;
            strcat(szPngSrc, pSub);
          }
          else
          {
            strcat(szPngSrc, pFile);
          }
                              
          if (FileExists(szPngSrc) == SUCCESS)
          {                        
            char szCmd[512] = "";
            
            if (IsPngFile(szPngSrc) != SUCCESS)
            {
              //file not found
              char szTxt[512];
              sprintf(szTxt, "config_screensaver add: no png file %s", szPngSrc);
              SetLastError(szTxt);  
              status = ERROR;
              setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt);
            }
            else
            {            
            
              //change file mode
              sprintf(szCmd, "/bin/chmod a+rw \'%s\'", szPngSrc);
              system(szCmd);
              
              //copy png to userdefined png directory
              sprintf(szCmd, "/usr/bin/cp -p \'%s\' \'%s\'", szPngSrc, szPngDest);
              //printf("call: %s\n", szCmd);
              if (system(szCmd) == 0)
              {
                //sync filesystem
                system("sync");
                status = SUCCESS;
                skipBCrestart = 1;
              }
              else
              {
                //cp failed
                char szTxt[512];
                sprintf(szTxt, "config_screensaver add: copy png file failed %s", szPngSrc);
                SetLastError(szTxt);  
                status = ERROR;
                setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt);
              }            
            }
          }
          else
          {
            //file not found
            char szTxt[512];
            sprintf(szTxt, "config_screensaver add: File not found %s", szPngSrc);
            SetLastError(szTxt);  
            status = ERROR;
            setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt);
          }
          
        }
        else
        {
          //not enough parameter
          char szTxt[256];
          sprintf(szTxt, "config_screensaver add png: not enough parameters");
          SetLastError(szTxt);  
          status = ERROR;
          setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt);
        }
      }
      else if (stricmp(argv[1], "remove") == 0) 
      {
        // remove userdefined png file from the system
        if (argc >= 3)
        {
          char szFileToRemove[256] = "";
          char * pSub;
          char szPngFile[256] = "";
          char * pFile = szPngFile;
          // URL decoding       
          strncpy(szPngFile, g_uri_unescape_string(argv[2], ""), sizeof(szPngFile)); 

          pSub = strrchr(pFile, '/');
          if (pSub)
          {
            pSub++;
            strncpy(szFileToRemove, szPngDest, sizeof(szFileToRemove));
            strcat(szFileToRemove, pSub);  
          }
          else
          {
            strncpy(szFileToRemove, szPngDest, sizeof(szFileToRemove));
            strcat(szFileToRemove, pFile);  
          }
          
          if (FileExists(szFileToRemove) == SUCCESS)
          {
            char szCmd[512] = "";
            
            if (IsPngFile(szFileToRemove) != SUCCESS)
            {
              //file not found
              char szTxt[512];
              sprintf(szTxt, "config_screensaver remove: no ttf file %s", szFileToRemove);
              SetLastError(szTxt);  
              status = ERROR;
              setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt);
            }
            else
            {
            
              //remove png from userdefined directory
              sprintf(szCmd, "/bin/rm -f \'%s\'", szFileToRemove);
              //printf("call: %s\n", szCmd);
              if (system(szCmd) == 0)
              {
                //sync filesystem
                system("sync");
                status = SUCCESS;
                skipBCrestart = 1; 
              }
              else
              {
                //rm failed
                char szTxt[512];
                sprintf(szTxt, "config_screensaver remove: rm png file failed %s", szFileToRemove);
                SetLastError(szTxt);  
                status = ERROR;
                setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt);
              }            
            }
          }
          else
          {
            //file not found
            char szTxt[512];
            sprintf(szTxt, "config_screensaver remove: file not found %s", szFileToRemove);
            SetLastError(szTxt);  
            status = ERROR;
            setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt);
          }
          
        }

      }
    }


      //save values to file
      if (status == SUCCESS)
      {
        if (ConfGetCount(g_pList) > 0)
        {
          ConfSaveValues(g_pList, CNF_FILE);
          //restart brightness_control
          if (!skipBCrestart)
          {
            system("/etc/config-tools/brightnesscontrol restart > /dev/null 2>&1");
          }
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
                  
    }
  }

  //clean up list memory
  ConfDestroyList(g_pList);

  return status;  
  
}

/// \brief Print help text / usage to stdout
void ShowHelpText()
{
  printf("\n* Screensaver settings * \n\n");
  printf("Usage: config_screensaver [state=state-value] [mode=mode-value] [theme=theme-value] [text=text-value] [time=time-value]\n\n");
  printf("state-value: enabled | disabled\n");
  printf("mode-value:  backlight | theme\n");
  printf("theme-value: number of the theme ID (1=image 2=time 3=text) \n");
  printf("image-value: complete image filepath (jpg, png, bmp) \n");  
  printf("text-value:  text to be shown \n");  
  printf("time-value:  time in seconds \n\n");  
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

int GetPngDirs(char * src_buf, int src_buflen, char * dest_buf, int dest_buflen)
{
  tConfList * pList = NULL;
  int ret = ERROR;       
  if (FileExists(CNF_FILE) != SUCCESS)
  {
    //file not found  
    char szTxt[256];
    sprintf(szTxt, "File open error %s", CNF_FILE);
    setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt);   
    AppendErrorText(FILE_OPEN_ERROR, CNF_FILE);    
    
    return ERROR;
  }
  
  //create list
  pList = ConfCreateList();

  ret = ConfReadValues(pList, CNF_FILE);
  if (ret != SUCCESS)
  {
    char szTxt[256];
    sprintf(szTxt, "File open error %s", CNF_FILE);
    setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt);

    AppendErrorText(FILE_OPEN_ERROR, CNF_FILE);
    //terminate
    ConfDestroyList(pList);
    return ERROR;
  }
  
  if (ConfGetCount(pList) == 0)
  {
    char szTxt[256];
    sprintf(szTxt, "File inconsistent error %s", CNF_FILE);
    setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt);

    AppendErrorText(CONFIG_FILE_INCONSISTENT, CNF_FILE);
    //terminate
    ConfDestroyList(pList);
    return ERROR;
  }
    
  if (ConfGetValue(pList, "png_dest_dir", &dest_buf[0], dest_buflen) == SUCCESS)
  {
    if (ConfGetValue(pList, "png_src_dir", &src_buf[0], src_buflen) == SUCCESS)
    {
      AppendSlash(src_buf);
      AppendSlash(dest_buf);
      ret = SUCCESS;      
    }  
  }
  
  ConfDestroyList(pList);  
  return ret;
}

int IsPngFile(char * pPngStr)
{
  int ret = ERROR;  
  
  if (pPngStr)
  {
    char * pStr = pPngStr;
    int len = strlen(pPngStr);
    
    if (len > 4)
    {
      pStr += (len -4);
      if (strcasecmp(pStr, ".png") == 0)
      {
        ret = SUCCESS;
      }
    }
  }
  
  return ret;
}
