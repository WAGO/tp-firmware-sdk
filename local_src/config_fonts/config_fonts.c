//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019 WAGO Kontakttechnik GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
/// \file     config_fonts.c
///
/// \version $Id: config_fonts.c 49277 2020-05-26 12:14:44Z wrueckl_elrest $
///
/// \brief    set font settings
///
/// \author   Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <glib.h>

#include "msgtool.h"
#include "config_tool_lib.h"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

#define CNF_FILE            FONTS_CONF_FILE

#define CMD_FONT_INSTALLED   "/usr/bin/dbus-send --system --type=signal /wago/diagnose/ID wago.diagnose.ID10001012 int32:$(date +%%s) int32:0 string:\"Font installed: %s\" boolean:true"
#define CMD_FONT_REMOVED     "/usr/bin/dbus-send --system --type=signal /wago/diagnose/ID wago.diagnose.ID10001013 int32:$(date +%%s) int32:0 string:\"Font removed: %s\" boolean:true"

//example
//  /usr/bin/dbus-send --system --type=signal /wago/diagnose/ID wago.diagnose.ID10001010 int32:$(date +%s) int32:0 string:"Perspecto system start-up process finished" boolean:true

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

void ShowHelpText();
void AppendErrorText(int iStatusCode, char * pText);
int IsTrueTypeFontFile(char * pFontStr);
int GetFontDirs(char * src_buf, int src_buflen, char * dest_buf, int dest_buflen);

//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// variable definitions
//------------------------------------------------------------------------------

/// configuration list start pointer
//tConfList * g_pList = NULL;

/// \brief Config data in form of name, default value, value list comma separated (no spaces)
static tConfValues aConfigValues[] =
{
  { "ttf_src_dir", "/tmp/font_uploads", "" },
  { "ttf_dest_dir", "/usr/share/fonts/X11/truetype/other", "" },
  
  // this line must always be the last one - don't remove it!
  { "", "" }
};

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

/// \brief Print help text / usage to stdout
void ShowHelpText()
{
  printf("\n* Fonts settings * \n\n");
  printf("Usage: config_fonts install | remove\n");
  //printf("ttf_src_dir-value: WBMs upload directory\n");
  //printf("ttf_dest_dir-value: f.e. /usr/share/fonts/X11/truetype/other \n");  
  printf("install ttf-font: config_fonts add MyFont.ttf \n");  
  printf("remove ttf-font: config_fonts remove MyFont.ttf \n\n");  
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
    else if (stricmp(argv[1], "add") == 0) 
    {
      // install a new userdefined truetype font 
      if (argc >= 3)
      {
        char * pSub;
        char szSrc[256] = "";
        char szDest[256] = "";
        if (GetFontDirs(szSrc, sizeof(szSrc), szDest, sizeof(szDest)) == SUCCESS)
        {  
          char szFontFile[256] = "";
          char * pFile = szFontFile;
          // URL decoding       
          strncpy(szFontFile, g_uri_unescape_string(argv[2], ""), sizeof(szFontFile));                       
                                          
          pSub = strrchr(pFile, '/');
          if (pSub)
          {
            pSub++;
            strcat(szSrc, pSub);
          }
          else
          {
            strcat(szSrc, pFile);
          }
                              
          if (FileExists(szSrc) == SUCCESS)
          {                        
            char szCmd[512] = "";
            
            if (IsTrueTypeFontFile(szSrc) != SUCCESS)
            {
              //file not found
              char szTxt[512];
              sprintf(szTxt, "config_font add: no ttf file %s", szSrc);
              SetLastError(szTxt);  
              status = ERROR;
              setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt);
            }
            else
            {            
            
              //change file mode
              sprintf(szCmd, "/bin/chmod a+rw \'%s\'", szSrc);
              system(szCmd);
              
              //copy font to userdefined ttf font directory
              sprintf(szCmd, "/usr/bin/cp -p \'%s\' \'%s\'", szSrc, szDest);
              //printf("call: %s\n", szCmd);
              if (system(szCmd) == 0)
              {
                char szCmdChdir[256];
                sprintf(szCmdChdir, "cd %s",szDest);
                
                //call mkfontscale -e
                sprintf(szCmd, "%s ; /usr/bin/mkfontscale -e \'%s\'", szCmdChdir, szDest);
                //printf("call: %s\n", szCmd);
                if (system(szCmd) == 0)
                {
                  //call mkfontdir -e
                  sprintf(szCmd, "%s ; /usr/bin/mkfontdir -e \'%s\'", szCmdChdir, szDest);
                  //printf("call: %s\n", szCmd);
                  if (system(szCmd) == 0)
                  {                
                    //sync filesystem
                    system("sync");                  
                    status = SUCCESS;
                    
                    //printf("NEW FONT ADDED:\n%s\n%s\n", szSrc, szDest);                  
                    //send dbus cmd to led-server / wago diagnostic
                    snprintf(szCmd, sizeof(szCmd), CMD_FONT_INSTALLED, szSrc);
                    system(szCmd);
                  }
                  else
                  {
                    //mkfontdir failed
                    char szTxt[512];
                    sprintf(szTxt, "config_font add: mkfontdir failed %s", szSrc);
                    SetLastError(szTxt);  
                    status = ERROR;
                    setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt);
                  }
                }
                else
                {
                  //mkfontscale failed
                  char szTxt[512];
                  sprintf(szTxt, "config_font add: mkfontscale failed %s", szSrc);
                  SetLastError(szTxt);  
                  status = ERROR;
                  setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt);
                }
              }
              else
              {
                //cp failed
                char szTxt[512];
                sprintf(szTxt, "config_font add: copy font file failed %s", szSrc);
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
            sprintf(szTxt, "config_font add: File not found %s", szSrc);
            SetLastError(szTxt);  
            status = ERROR;
            setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt);
          }
        
        }
        
      }
      else
      {
        //not enough parameter
        char szTxt[256];
        sprintf(szTxt, "config_fonts add font: not enough parameters");
        SetLastError(szTxt);  
        status = ERROR;
        setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt);
      }
    }
    else if (stricmp(argv[1], "remove") == 0) 
    {
      // remove userdefined truetype font from the system
      if (argc >= 3)
      {
        char szSrc[256] = "";
        char szDest[256] = "";
        char szFileToRemove[256] = "";
        if (GetFontDirs(szSrc, sizeof(szSrc), szDest, sizeof(szDest)) == SUCCESS)
        { 
          char * pSub;
          char szFontFile[256] = "";
          char * pFile = szFontFile;
          // URL decoding       
          strncpy(szFontFile, g_uri_unescape_string(argv[2], ""), sizeof(szFontFile)); 

          pSub = strrchr(pFile, '/');
          if (pSub)
          {
            pSub++;
            strncpy(szFileToRemove, szDest, sizeof(szFileToRemove));
            strcat(szFileToRemove, pSub);  
          }
          else
          {
            strncpy(szFileToRemove, szDest, sizeof(szFileToRemove));
            strcat(szFileToRemove, pFile);  
          }
          
          if (FileExists(szFileToRemove) == SUCCESS)
          {
            char szCmd[512] = "";
            
            if (IsTrueTypeFontFile(szFileToRemove) != SUCCESS)
            {
              //file not found
              char szTxt[512];
              sprintf(szTxt, "config_font remove: no ttf file %s", szFileToRemove);
              SetLastError(szTxt);  
              status = ERROR;
              setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt);
            }
            else
            {
            
              //remove font from userdefined ttf font directory
              sprintf(szCmd, "/bin/rm -f \'%s\'", szFileToRemove);
              //printf("call: %s\n", szCmd);
              if (system(szCmd) == 0)
              {
                char szCmdChdir[256];
                sprintf(szCmdChdir, "cd %s",szDest);
                
                //call mkfontscale -e
                sprintf(szCmd, "%s ; /usr/bin/mkfontscale -e \'%s\'",szCmdChdir, szDest);
                //printf("call: %s\n", szCmd);
                if (system(szCmd) == 0)
                {
                  //call mkfontdir -e
                  sprintf(szCmd, "%s ; /usr/bin/mkfontdir -e \'%s\'",szCmdChdir, szDest);
                  //printf("call: %s\n", szCmd);
                  if (system(szCmd) == 0)
                  {                
                    //sync filesystem
                    system("sync");                  
                    status = SUCCESS;                  
                    //printf("FONT removed:\n%s\n", szFileToRemove);
                    //send dbus cmd to led-server / wago diagnostic
                    snprintf(szCmd, sizeof(szCmd), CMD_FONT_REMOVED, szFileToRemove);
                    system(szCmd);
                  }
                  else
                  {
                    //mkfontdir failed
                    char szTxt[512];
                    sprintf(szTxt, "config_font remove: mkfontdir failed %s", szFileToRemove);
                    SetLastError(szTxt);  
                    status = ERROR;
                    setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt);
                  }
                }
                else
                {
                  //mkfontscale failed
                  char szTxt[512];
                  sprintf(szTxt, "config_font remove: mkfontscale failed %s", szFileToRemove);
                  SetLastError(szTxt);  
                  status = ERROR;
                  setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt);
                }
              }
              else
              {
                //rm failed
                char szTxt[512];
                sprintf(szTxt, "config_font remove: rm font file failed %s", szFileToRemove);
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
            sprintf(szTxt, "config_font remove: file not found %s", szFileToRemove);
            SetLastError(szTxt);  
            status = ERROR;
            setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt);
          }
        
        }
        
      }
      else
      {
        //not enough parameter
        char szTxt[256];
        sprintf(szTxt, "config_fonts remove font: not enough parameters");
        SetLastError(szTxt);  
        status = ERROR;
        setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt);
      }
    }
    
/*  WAT16156, WAT16157  
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

      }
            
    }    
*/    
    
  }

  //clean up list memory
  //ConfDestroyList(g_pList);

  return status;
}

int GetFontDirs(char * src_buf, int src_buflen, char * dest_buf, int dest_buflen)
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
    
  if (ConfGetValue(pList, "ttf_dest_dir", &dest_buf[0], dest_buflen) == SUCCESS)
  {
    if (ConfGetValue(pList, "ttf_src_dir", &src_buf[0], src_buflen) == SUCCESS)
    {
      AppendSlash(src_buf);
      AppendSlash(dest_buf);
      ret = SUCCESS;      
    }  
  }
  
  ConfDestroyList(pList);  
  return ret;
}


int IsTrueTypeFontFile(char * pFontStr)
{
  int ret = ERROR;  
  
  if (pFontStr)
  {
    char * pStr = pFontStr;
    int len = strlen(pFontStr);
    
    if (len > 4)
    {
      pStr += (len -4);
      if (strcasecmp(pStr, ".ttf") == 0)
      {
        ret = SUCCESS;
      }
    }
  }
  
  return ret;
}
