//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019-2022 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
/// \file    get_fonts.c
///
/// \version $Id: get_fonts.c 65689 2022-03-11 14:37:43Z falk.werner@wago.com $
///
/// \brief   get font settings
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
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

#define CNF_FILE            FONTS_CONF_FILE

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

void ShowHelpText();
void AppendErrorText(int iStatusCode, char * pText);
int ReadJSONFontList(char * pszOut, int bufsize);

//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variable definitions
//------------------------------------------------------------------------------

/// configuration list start pointer
tConfList * g_pList = NULL;

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

/// \brief Print help text / usage to stdout
void ShowHelpText()
{
  printf("\n* Get font settings * \n\n");
  printf("Usage: get_font [options] <parameter>\n\n");
  printf("options:\n");
  printf("all-json print all values in JSON format \n"); 
  printf("-h print this help \n\n");
  printf("parameter: ttf \n\n");
  
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
        system("/etc/config-tools/config_fonts resettofactory > /dev/null 2>&1");  
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
        char *psz = szJson;
        //print all values in JSON format
        strcat(szJson, "{");
        psz = szJson + strlen(szJson);
        ConfPrintAllValuesJson(g_pList, psz);
                
        strcat(szJson, "\"ttf_list\": [");
        psz = szJson + strlen(szJson);
        
        ReadJSONFontList(psz, sizeof(szJson) - strlen(psz) - 3);                
        
        strcat(szJson, "]");        
        //ConfRemoveJsonSeparatorFromEnd(&szJson[0]);
        strcat(szJson, "}");
        printf(szJson);
        status = SUCCESS;
      }
      else
      {
              
        for (i = 1; i < argc; i++)
        {
          pStr = argv[i];
          
          if (stricmp(pStr, "ttf_src_dir") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "ttf_dest_dir") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "ttf_list") == 0) 
          {
            //print a comma separated list of user defined truetype fonts
            char szSrcDir[512] = "";
            char szCmd[512] = "";
            if (ConfGetValue(g_pList, "ttf_dest_dir", &szSrcDir[0], sizeof(szSrcDir)) == SUCCESS)
            {
              FILE * pf = NULL;
              char buffer[4096];
              char * pLine = NULL;
              int len = strlen(szSrcDir);
              if (len > 0)
              {                            
                if (szSrcDir[len-1] != '/')
                    strcat(szSrcDir, "/");
                
                //ls /usr/share/fonts/X11/truetype/other | awk '/.[tT][tT][fF]/ {printf $NF ","}'
                sprintf(szCmd, "/bin/ls -1 %s | awk '/.[tT][tT][fF]/ {printf $0 \",\"}'", szSrcDir );
                //printf("cmd: %s \n", szCmd);
                pf = popen(szCmd, "r");
                if (pf)
                {                                 
                  pLine = fgets(buffer, sizeof(buffer), pf);
                  
                  if (pLine)
                  {
                    len = strlen(pLine);
                    if (len > 0)
                    {
                      if (pLine[len-1] == ',')
                      {
                        pLine[len-1] = '\0';
                      }
                    }
                    printf("%s", pLine);                    
                  }

                  status = SUCCESS;
                  pclose(pf);
                }  
              }
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


int ReadJSONFontList(char * pszOut, int bufsize)
{
  int status = ERROR;
  FILE * pf = NULL;
  char buffer[4096];
  char * pLine = NULL;
  char szCmd[512] = "";
  char szSrcDir[512] = "";
  if (ConfGetValue(g_pList, "ttf_dest_dir", &szSrcDir[0], sizeof(szSrcDir)) == SUCCESS)
  {
    int len = strlen(szSrcDir);  
    if (len > 0)
    {                            
      //ls /usr/share/fonts/X11/truetype/other | awk '/.[tT][tT][fF]/ {printf $NF ","}'
      
      sprintf(szCmd, "/bin/ls -1 %s | awk '/.[tT][tT][fF]/ {printf \"\\\"\" $0 \"\\\"\" \",\"}'", szSrcDir );

      //printf("cmd: %s \n", szCmd);
      pf = popen(szCmd, "r");
      if (pf)
      {                                 
        pLine = fgets(buffer, sizeof(buffer), pf);
        
        if (pLine)
        {
          len = strlen(pLine);
          if (len > 0)
          {
            if (pLine[len-1] == ',')
            {
              pLine[len-1] = '\0';
            }
          }
          strncpy(pszOut, pLine, bufsize);
          //printf("%s", pLine);
          status = SUCCESS;
        }              
        pclose(pf);
      }  
    }
  }
  
  return status;
}
