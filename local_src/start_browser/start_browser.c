//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019-2022 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
/// \file start_browser.c
///
/// \version $Id: start_browser.c 65689 2022-03-11 14:37:43Z falk.werner@wago.com $
///
/// \brief send commands to webengine_browser 
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "msgtool.h"

#include "config_tool_lib.h"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variable definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
int SendCmd2Webengine(char * pCmdStr, char * pStr);


/// \brief Print help text / usage to stdout
void ShowHelpText()
{
  printf("\n* send command to browser * \n\n");
  printf("Usage: start_browser <cmd> [param]\n\n");
  printf("cmd:\n");
  printf("url       show the given URL [param]\n"); 
  printf("          [param] url string\n"); 
  printf("stop      stop the current website\n"); 
  printf("reload    reload the current website\n"); 
  printf("vkeyb     use virtual keyboard [param]\n"); 
  printf("          [param] enabled | disabled\n"); 
  printf("close     browser will be closed (exit)\n"); 
  printf("-h        print this help \n\n");
  printf("f.e. start_browser url=\"http://192.168.1.1/plc/webvisu.htm\" vkeyb=disabled \n");
  printf("f.e. start_browser stop\n");
  printf("f.e. start_browser reload \n\n");
}


/// \brief send command to webengine_browser
/// \param[in]  pCmdStr command string
/// \param[in]  pStr string sending to webengine
/// \retval 0  SUCCESS
int SendCmd2Webengine(char * pCmdStr, char * pStr)
{
  int ret = -1;
  char szCmd[512] = "";
  
  if (strcmp(pCmdStr, "start")==0)
  {
    if (pStr)
    {
      sprintf(szCmd, "load=%s\n", pStr);
      ret = Write2PipedFifo(DEV_WEBENGINEBROWSER, szCmd);
    }  
  }
  else if (strcmp(pCmdStr, "vkeyb")==0)
  {
    if (pStr)
    {
      if (strcmp(pStr, "disabled")==0)
      {
        sprintf(szCmd, "disable\n");
        ret = Write2PipedFifo(DEV_VIRTUALKEYBOARD, szCmd);
      }
      else
      {
        sprintf(szCmd, "enable\n");
        ret = Write2PipedFifo(DEV_VIRTUALKEYBOARD, szCmd);
      }
    }  
  }
  else if (strcmp(pCmdStr, "stop")==0)
  {
    sprintf(szCmd, "stop\n");
    ret = Write2PipedFifo(DEV_WEBENGINEBROWSER, szCmd);
  }
  else if (strcmp(pCmdStr, "reload")==0)
  {
    sprintf(szCmd, "reload\n");
    ret = Write2PipedFifo(DEV_WEBENGINEBROWSER, szCmd);
  }
  else if (strcmp(pCmdStr, "close")==0)
  {
    sprintf(szCmd, "close\n");
    ret = Write2PipedFifo(DEV_WEBENGINEBROWSER, szCmd);
  }
  else if (strcmp(pCmdStr, "back")==0)
  {
    sprintf(szCmd, "back\n");
    ret = Write2PipedFifo(DEV_WEBENGINEBROWSER, szCmd);
  }

  return ret;
}


/// \brief main function
/// \param[in]  argc number of arguments
/// \param[in]  argv arguments
/// \retval 0  SUCCESS
int main(int argc, char *argv[])
{
  int state = ERROR;
  
  char szCmd[128] = "";
  char szUrl[512] = "";
  char szKbd[128] = "";
  
  char * pPos = NULL;
  
  if (argc >= 2)
  {
    if ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0))
    {
      ShowHelpText();
      state = SUCCESS;
    }
    else
    {
      int i;
      char * pStr;            
              
      for (i = 1; i < argc; i++)
      {
        pStr = argv[i];
        
        if (stricmp(pStr, "url") == 0) 
        {
          strcpy(szCmd, "start");          
          if (i+1 < argc)
          {
            strncpy(szUrl, argv[i+1], sizeof(szUrl));
            i++;
          }
        }
        else if ((pPos = strstr(pStr, "url="))  && (pPos == pStr))
        {
          if (strlen(pStr) > 4)
          {          
            pStr += 4;
            strcpy(szCmd, "start");                      
            strncpy(szUrl, pStr, sizeof(szUrl));            
          }
        }
        else if (stricmp(pStr, "stop") == 0) 
        {
          strcpy(szCmd, "stop");          
        }
        else if (stricmp(pStr, "close") == 0) 
        {
          strcpy(szCmd, "close");          
        }
        else if (stricmp(pStr, "reload") == 0) 
        {
          strcpy(szCmd, "reload");          
        }
        else if (stricmp(pStr, "vkeyb") == 0) 
        {          
          if (i+1 < argc)
          {
            strncpy(szKbd, argv[i+1], sizeof(szKbd));
            i++;
          }
        }
        else if ((pPos = strstr(pStr, "vkeyb="))  && (pPos == pStr)) 
        {
          if (strlen(pStr) > 6)
          {          
            pStr += 6;
            strncpy(szKbd, pStr, sizeof(szKbd));  
          }
        }

      } 

      if (strlen(szCmd) > 0)
      {
        
        //server_id = MsgToolGetServerId(MSG_TOOL_IDENT_WEBKITBROWSER);
        
        if ((strcmp(szKbd, "disabled") == 0) || (strcmp(szKbd, "enabled") == 0))
        {
          if (SendCmd2Webengine("vkeyb", szKbd) == SUCCESS)
            state = SUCCESS;   
        }
        
        if (strcmp(szCmd, "start") == 0)
        {
          if (strlen(szUrl) > 0)
          {
            //start browser via msg
            if (SendCmd2Webengine(szCmd, szUrl) == SUCCESS)
              state = SUCCESS;
          }          
        }
        else if (strcmp(szCmd, "stop") == 0)
        {
          if (SendCmd2Webengine(szCmd, NULL) == SUCCESS)
            state = SUCCESS;                        
        }
        else if (strcmp(szCmd, "back") == 0)
        {
          if (SendCmd2Webengine(szCmd, NULL) == SUCCESS)
            state = SUCCESS;                        
        }
        else if (strcmp(szCmd, "reload") == 0)
        {
          if (SendCmd2Webengine(szCmd, NULL) == SUCCESS)
            state = SUCCESS;                        
        }        
        else if (strcmp(szCmd, "close") == 0)
        {
          if (SendCmd2Webengine(szCmd, NULL) == SUCCESS)
            state = SUCCESS;                        
        }        

      }
    }
  }
    

  return state;
}
