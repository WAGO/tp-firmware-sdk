//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019 WAGO Kontakttechnik GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
///
/// \file    config_plcselect.c
///
/// \version $Id: config_plcselect.c 45958 2020-01-21 12:26:20Z wrueckl_elrest $
///
/// \brief   plc selection settings / config-tools
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------
#define _GNU_SOURCE 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "msgtool.h"
#include "config_tool_lib.h"


//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

#define  MAX_PLC_NUMBER      12          //WBM, PLC select list, 10 further PLCs
#define  CNF_FILE            PLCSELECT_CONF_FILE

#define  MIN_RECONNECT_INTERVAL        5
#define  MAX_RECONNECT_INTERVAL        99
#define  MIN_V2UPDATECHECK_INTERVAL    60

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

void ShowHelpText();
void AppendErrorText(int iStatusCode, char * pText);

//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables’ and constants’ definitions
//------------------------------------------------------------------------------

/// configuration list start pointer
tConfList * g_pList = NULL;

/// \brief Config data in form of name, default value, value list comma separated (no spaces)
static tConfValues aConfigValues[] =
{
  { "statemonitor", "enabled", "enabled,disabled" },
  { "reconnect_interval", "10", "" },
  { "v2updatecheck_interval", "120", "" },
  { "plc_selected", "0", "" },
  { "v2_ident", "webvisu.htm", "" },
  { "v3_ident", "WebVisuV3.bin", "" },  
  { "url00", "http://127.0.0.1/app", "" },
  { "txt00", "WBM", "" },
  { "vkb00", "enabled", "enabled,disabled" },  
  { "mon00", "0", "" },
  { "mic00", "0", "" },
  { "url01", "http://127.0.0.1/plclist/plclist.html", "" },
  { "txt01", "PLC select list", "" },
  { "vkb01", "enabled", "enabled,disabled" },
  { "mon01", "0", "" },
  { "mic01", "0", "" },
  { "url02", "", "" },
  { "txt02", "", "" },
  { "vkb02", "enabled", "enabled,disabled" },
  { "mon02", "1", "" },
  { "mic02", "0", "" },
  { "url03", "", "" },
  { "txt03", "", "" },
  { "vkb03", "enabled", "enabled,disabled" },
  { "mon03", "1", "" },
  { "mic03", "0", "" },
  { "url04", "", "" },
  { "txt04", "", "" },
  { "vkb04", "enabled", "enabled,disabled" },
  { "mon04", "1", "" },
  { "mic04", "0", "" },
  { "url05", "", "" },
  { "txt05", "", "" },
  { "vkb05", "enabled", "enabled,disabled" },
  { "mon05", "1", "" },
  { "mic05", "0", "" },
  { "url06", "", "" },
  { "txt06", "", "" },
  { "vkb06", "enabled", "enabled,disabled" },
  { "mon06", "1", "" },
  { "mic06", "0", "" },
  { "url07", "", "" },
  { "txt07", "", "" },
  { "vkb07", "enabled", "enabled,disabled" },
  { "mon07", "1", "" },
  { "mic07", "0", "" },
  { "url08", "", "" },
  { "txt08", "", "" },
  { "vkb08", "enabled", "enabled,disabled" },
  { "mon08", "1", "" },
  { "mic08", "0", "" },
  { "url09", "", "" },
  { "txt09", "", "" },
  { "vkb09", "enabled", "enabled,disabled" },
  { "mon09", "1", "" },
  { "mic09", "0", "" },
  { "url10", "", "" },
  { "txt10", "", "" },
  { "vkb10", "enabled", "enabled,disabled" },
  { "mon10", "1", "" },
  { "mic10", "0", "" },
  { "url11", "", "" },
  { "txt11", "", "" },
  { "vkb11", "enabled", "enabled,disabled" },
  { "mon11", "1", "" },
  { "mic11", "0", "" },

  // this line must always be the last one - don't remove it!
  { "", "", "" }
};

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

/// \brief main function
/// \param[in]  argc number of arguments
/// \param[in]  argv arguments
/// \retval 0  SUCCESS
int main(int argc, char **argv)
{
  int status = ERROR;
  int index = -1;
  int errorcounter = 0;
  char szArgv[256] = "";
  char szOut[256] = "";
 
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
      int nFileElements = 0;
      int nStructElements = 0;
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
        char szErrorTxt[256];
        sprintf(szErrorTxt, "File open error %s", CNF_FILE);
        setRgbLed(RGB_LED_STATE_RE_BLINK, szErrorTxt);
        
        AppendErrorText(FILE_OPEN_ERROR, CNF_FILE);
        //terminate
        ConfDestroyList(g_pList);
        return ERROR;
      }
      
      nFileElements = ConfGetCount(g_pList);
      if (nFileElements == 0)
      {
        char szErrorTxt[256];
        sprintf(szErrorTxt, "File inconsistent error %s", CNF_FILE);
        setRgbLed(RGB_LED_STATE_RE_BLINK, szErrorTxt);
        
        AppendErrorText(CONFIG_FILE_INCONSISTENT, CNF_FILE);
        //terminate
        ConfDestroyList(g_pList);
        return ERROR;
      }    
      
      // count struct elements
      nStructElements = 0;
      while (aConfigValues[nStructElements].nameStr[0])
      {
        nStructElements++;
      }
      
      //sanity check
      if (nStructElements > nFileElements)
      {
        int iElements = 0;
        //printf("nFileElements: %d StructElements: %d\n", nFileElements, nStructElements);
        //add missing elements
        while (aConfigValues[iElements].nameStr[0])
        {
          if (ConfGetValue(g_pList, aConfigValues[iElements].nameStr, &szOut[0], sizeof(szOut)) != SUCCESS)
          {
            //add missing element
            //printf("missing: %s\n", aConfigValues[iElements].nameStr);
            ConfAddValue(g_pList, aConfigValues[iElements].nameStr, aConfigValues[iElements].defaultStr);
          }
          iElements++;
        }
      }
      
      // URL decoding       
      strncpy(szArgv, g_uri_unescape_string(argv[1], ""), sizeof(szArgv));     
      
      if (ConfIsNumber(szArgv) == SUCCESS)
      {
        index = atoi(szArgv);           
        if ((index < 0) || (index >= MAX_PLC_NUMBER))
        {
          //LED
          char szErrTxt[256];
          sprintf(szErrTxt, "Invalid parameter: %s", argv[0]);
          setRgbLed(RGB_LED_STATE_RE_BLINK, szErrTxt);
          
          AppendErrorText(INVALID_PARAMETER, argv[0]);
          //wrong index terminate
          ConfDestroyList(g_pList);
          return ERROR;  
        }
      }
      
      for (i = 1; i < argc; i++)
      {
        // URL decoding       
        strncpy(szArgv, g_uri_unescape_string(argv[i], ""), sizeof(szArgv));     
        
        pStr =  &szArgv[0];
        if (strstr(szArgv, "reconnect_interval=") && (strlen(pStr) > 19))
        {
          pStr += 19;          
          if (ConfIsNumber(pStr) == SUCCESS)
          {
            int iValue = atoi(pStr);
            char szNew[64] = "";
            
            if (iValue < MIN_RECONNECT_INTERVAL)
            {
              char szErrTxt[256];
              sprintf(szErrTxt, "Minimal reconnect interval is %d [s]", MIN_RECONNECT_INTERVAL);
              AppendErrorText(INVALID_PARAMETER, szErrTxt);
              ConfDestroyList(g_pList);
              return ERROR;  
            }
            else if (iValue > MAX_RECONNECT_INTERVAL)
            {
              char szErrTxt[256];
              sprintf(szErrTxt, "Maximum reconnect interval is %d [s]", MAX_RECONNECT_INTERVAL);
              AppendErrorText(INVALID_PARAMETER, szErrTxt);
              ConfDestroyList(g_pList);
              return ERROR;
            }

            sprintf(szNew, "%d", iValue);
            if (ConfSetValue(NULL, g_pList, "reconnect_interval", szNew)==SUCCESS)
            {            
              status = SUCCESS;
            }
          }
        }        
        else if (strstr(szArgv, "v2updatecheck_interval=") && (strlen(pStr) > 23))
        {
          pStr += 23;          
          if (ConfIsNumber(pStr) == SUCCESS)
          {
            int iValue = atoi(pStr);
            char szNew[64] = "";
            if (iValue < MIN_V2UPDATECHECK_INTERVAL)
            {
              char szErrTxt[256];
              sprintf(szErrTxt, "Minimal V2UpdateCheck interval is %d [s]", MIN_V2UPDATECHECK_INTERVAL);
              AppendErrorText(INVALID_PARAMETER, szErrTxt);
              ConfDestroyList(g_pList);
              return ERROR;  
            }
            
            sprintf(szNew, "%d", iValue);
            if (ConfSetValue(NULL, g_pList, "v2updatecheck_interval", szNew)==SUCCESS)
            {            
              status = SUCCESS;
            }
          }
        }
        //else if (strstr(szArgv, "plc_count=") && (strlen(pStr) > 10))
        //{
        //  pStr += 10;
        //  if ((ConfIsNumber(pStr) == SUCCESS) && 
        //      (ConfSetValue(NULL, g_pList, "plc_count", pStr)==SUCCESS))
        //  {
        //    
        //    status = SUCCESS;
        //  }
        //}
        else if (strstr(szArgv, "plc_selected=") && (strlen(pStr) > 13))
        {
          pStr += 13;
          if ((ConfIsNumber(pStr) == SUCCESS) && 
              (ConfSetValue(NULL, g_pList, "plc_selected", pStr)==SUCCESS))
          {
            
            status = SUCCESS;
          }
        }
        else if (strstr(szArgv, "statemonitor=") && (strlen(pStr) > 13))
        {
          pStr += 13;
          if (ConfSetValue(&aConfigValues[0], g_pList, "statemonitor", pStr)==SUCCESS)
          {
            
            status = SUCCESS;
          }
        }
        else
        {
          char szUrl[64] = ""; //Browser URL
          char szTxt[64] = ""; //description
          char szVkb[64] = ""; //virtualkeyboard
          char szMon[64] = ""; //monitoring, reconnect handling
          char szMic[64] = ""; //microbrowser used for codesys V2 visu
          char tmp[64] = "";                              
                    
          if (index >= 0)
          {            
            int iPlcSelected = 0;
            
            if (ConfGetValue(g_pList, "plc_selected", &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              if (ConfIsNumber(szOut) == SUCCESS)
              {
                iPlcSelected = atoi(szOut);
                //printf("iPlcSelected %d\n", iPlcSelected);
              }
            }
            
            pStr = szArgv;                        
            sprintf(szUrl, "url=");  
            sprintf(szTxt, "txt=");  
            sprintf(szVkb, "vkb=");  
            sprintf(szMon, "mon=");  
            sprintf(szMic, "mic=");  
            
            //Webbrowser URL used by Webenginebrowser or Microbrowser
            if (strstr(szArgv, szUrl) && (strlen(pStr) >= strlen(szUrl)))
            {
              char * s;
              pStr += strlen(szUrl);
              szUrl[strlen(szUrl)-1] = '\0';
              sprintf(tmp, "%s%02d", szUrl, index);
              while(*pStr == ' ')
              {
                pStr++;
              }
              
              if ((strlen(pStr) > 0) &&
                  (strcasestr(pStr, "http://") != pStr) && 
                  (strcasestr(pStr, "https://") != pStr) &&
                  (strcasestr(pStr, "ws://") != pStr) &&
                  (strcasestr(pStr, "wss://") != pStr))
              {
                char szError[256] = "";
                sprintf(szError, "invalid url: %s", pStr);
                SetLastError(szError);
                status = ERROR;
                errorcounter++;
              }
              else
              {
              //debug printf("....%s....\n", pStr);
                if (ConfSetValue(NULL, g_pList, tmp, pStr)==SUCCESS)
                {                
                  status = SUCCESS;                                
                  
                  if (strlen(pStr) == 0)
                  {
                    //empty string means delete this entry
                    if ((iPlcSelected > 1) && (index == iPlcSelected))
                    {
                      //set plc_selected to plc selection list
                      if (ConfSetValue(NULL, g_pList, "plc_selected", "1") == SUCCESS)
                      {
                        //printf("activate plc selection list: iPlcSelected %d\n", iPlcSelected);
                      }
                    }
                  }
                }
                else
                {
                  char szErrorTxt[256];
                  sprintf(szErrorTxt, "SetValue failed: %s", pStr);
                  SetLastError(szErrorTxt); 
                  status = ERROR;
                  errorcounter++;
                }
              }
            }            
            
            //Description of Visu or URL
            if (strstr(szArgv, szTxt) && (strlen(pStr) >= strlen(szTxt)))
            {
              pStr += strlen(szTxt);
              szTxt[strlen(szTxt)-1] = '\0';
              sprintf(tmp, "%s%02d", szTxt, index);
              if (ConfSetValue(NULL, g_pList, tmp, pStr)==SUCCESS)
              {                
                status = SUCCESS;
              }
              else
              {
                char szErrorTxt[256];
                sprintf(szErrorTxt, "SetValue failed: %s", pStr);
                SetLastError(szErrorTxt); 
                status = ERROR;
                errorcounter++;
              }
            }
            
            //Virtualkeyboard enabled / disabled
            if (strstr(szArgv, szVkb) && (strlen(pStr) >= strlen(szVkb)))
            {
              pStr += strlen(szVkb);
              szVkb[strlen(szVkb)-1] = '\0';
              sprintf(tmp, "%s%02d", szVkb, index);
              if (ConfSetValue(&aConfigValues[0], g_pList, tmp, pStr)==SUCCESS)
              {                
                status = SUCCESS;
              }
              else
              {
                char szErrorTxt[256];
                sprintf(szErrorTxt, "SetValue failed: %s", pStr);
                SetLastError(szErrorTxt); 
                status = ERROR;
                errorcounter++;
              }
            }
            
            //Monitoring means enable webvisu automatic reconnect 
            if (strstr(szArgv, szMon) && (strlen(pStr) >= strlen(szMon)))
            {
              pStr += strlen(szMon);
              szMon[strlen(szMon)-1] = '\0';
              sprintf(tmp, "%s%02d", szMon, index);
              if (ConfIsNumber(pStr) == SUCCESS)
              {
                if (ConfSetValue(&aConfigValues[0], g_pList, tmp, pStr)==SUCCESS)
                {                
                  status = SUCCESS;
                }
                else
                {
                  char szErrorTxt[256];
                  sprintf(szErrorTxt, "SetValue failed: %s", pStr);
                  SetLastError(szErrorTxt); 
                  status = ERROR;
                  errorcounter++;
                }
              }
            }

            //Microbrowser (codesys v2 webvisu)
            if (strstr(szArgv, szMic) && (strlen(pStr) >= strlen(szMic)))
            {
              pStr += strlen(szMic);
              szMic[strlen(szMic)-1] = '\0';
              sprintf(tmp, "%s%02d", szMic, index);
              if (ConfIsNumber(pStr) == SUCCESS)
              {
                if (ConfSetValue(&aConfigValues[0], g_pList, tmp, pStr)==SUCCESS)
                {                
                  status = SUCCESS;
                }
                else
                {
                  char szErrorTxt[256];
                  sprintf(szErrorTxt, "SetValue failed: %s", pStr);
                  SetLastError(szErrorTxt); 
                  status = ERROR;
                  errorcounter++;
                }
              }
            }
            
          }
          
        }               
      
      }

      if (errorcounter > 0)
      {
        status = ERROR;
      }

      //save values to file
      if (status == SUCCESS)
      {
        if (ConfGetCount(g_pList) > 0)
        {
          ConfSaveValues(g_pList, CNF_FILE);
          system("/usr/bin/php /etc/script/generate_plclist.php > /dev/null 2>&1");
          system("/bin/sync");          
        }
        else
        {
          char szErrorTxt[256];
          sprintf(szErrorTxt, "No items found in %s", CNF_FILE);
          SetLastError(szErrorTxt); 
          status = ERROR;          
          setRgbLed(RGB_LED_STATE_RE_BLINK, szErrorTxt); 
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
  printf("\n* Change PLC selection settings * \n\n");
  printf("Usage1: config_plcselect [reconnect_interval=reconnect_interval-value]\n");
  printf("                         [statemonitor=statemonitor-value]\n");
  //printf("                         [plc_count=plc_count-value]\n");
  printf("                         [plc_selected=plc_selected-value]\n");
  printf("\n");
  printf("Usage2: config_plcselect <index> [url=url-value]\n");
  printf("                                 [txt=txt-value]\n");
  printf("                                 [vkb=vkb-value]\n");
  printf("                                 [mon=mon-value]\n");
  printf("                                 [mic=mic-value]\n");
  printf("\n");
  printf("statemonitor-value:       monitoring system enabled or disabled\n");
  printf("reconnect_interval-value: time in [s] to next reconnect\n");
  //printf("plc_count-value:          number of configured PLCs\n");
  printf("\n");
  printf("index:                    0..11 (0=WBM, 1=PLC select list)\n");
  printf("url-value:                URL of the PLC f.e. http://192.168.1.1\n");
  printf("txt-value:                Name or Description of the PLC\n");
  printf("vkb-value:                virtual keyboard enabled or disabled\n");
  printf("mon-value:                0=no monitoring, 1=Codesys Webvisu\n");
  printf("mic-value:                0=non codesys v2 visu, 1=use microbrowser for codesys v2 visu\n");
  printf("\n");
  //printf("example: config_plcselect plc_count=2\n");
  printf("example: config_plcselect statemonitor=enabled\n");
  printf("example: config_plcselect 2 url=http://192.168.1.10/plc/webvisu.htm\n");
  printf("example: config_plcselect 4 vkb=disabled\n");
  printf("example: config_plcselect 4 mon=1\n");
  printf("example: config_plcselect 5 mic=1\n");
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

