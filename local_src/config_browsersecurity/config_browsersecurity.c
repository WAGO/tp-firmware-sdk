//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019 WAGO Kontakttechnik GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
/// \file config_browsersecurity
///
/// \version $Id: config_browsersecurity.c 44064 2019-10-24 12:34:47Z wrueckl_elrest $
///
/// \brief set the browsers SSL security level
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
#include "msgtool.h"
#include "config_tool_lib.h"


//------------------------------------------------------------------------------
// Local macros
//------------------------------------------------------------------------------

#define CNF_FILE          BROWSER_SECURITY_CONF_FILE

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
  { "level", "high", "low,high" },
  { "low_enabled", "5,6,9,10,22", "" },
  { "check_datetime", "yes", "no,yes" },  
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
int main(int argc, char **argv)
{
  int status = ERROR;
  //int restart_service = 0;
  char szArgv[256] = "";
  
  char szSearchStr[128] = "";
  int len = 0;
 
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
          char szTxt[256] = "";
          sprintf(szTxt, "No items found in %s", CNF_FILE);
          SetLastError(szTxt); 
          status = ERROR; 
          setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt); 
        }
      }
      
      //if (restart_service)
      //{        
      //}
            
    }
  }

  //clean up list memory
  ConfDestroyList(g_pList);

  return status;
}

/// \brief Print help text / usage to stdout
void ShowHelpText()
{
  printf("\n* Set browsers security level * \n\n");
  printf("Usage: config_browsersecurity [level=level-value]\n");
  printf("                              [low_enabled=low_enabled-value]\n\n");
  printf("level: low | high\n");
  printf("low_enabled: comma separated list f.e. 5,6,9,10,22\n");
  printf("ssl numbers:\n");
  
  //NOT ALLOWED    
  printf("QSslError::UnableToGetIssuerCertificate         1\n");
  printf("QSslError::UnableToDecryptCertificateSignature  2\n");
  printf("QSslError::UnableToDecodeIssuerPublicKey        3\n");
  printf("QSslError::CertificateSignatureFailed           4\n");
  printf("QSslError::InvalidNotBeforeField                7\n");
  printf("QSslError::InvalidNotAfterField                 8\n");
  printf("QSslError::UnableToGetLocalIssuerCertificate   11\n");
  printf("QSslError::UnableToVerifyFirstCertificate      12\n");
  printf("QSslError::CertificateRevoked                  13\n");
  printf("QSslError::InvalidCaCertificate                14\n");
  printf("QSslError::PathLengthExceeded                  15\n");
  printf("QSslError::InvalidPurpose                      16\n");
  printf("QSslError::CertificateUntrusted                17\n");
  printf("QSslError::CertificateRejected                 18\n");
  printf("QSslError::SubjectIssuerMismatch               19\n");
  printf("QSslError::AuthorityIssuerSerialNumberMismatch 20\n");
  printf("QSslError::NoPeerCertificate                   21\n");
  printf("QSslError::HostNameMismatch                    22\n");
  //printf("QSslError::UnspecifiedError  -1\n");
  printf("QSslError::NoSslSupport                        23\n");
  printf("QSslError::CertificateBlacklisted              24\n");

  //ALLOWED
  printf("QSslError::CertificateNotYetValid               5\n");
  printf("QSslError::CertificateExpired                   6\n");
  printf("QSslError::SelfSignedCertificate                9\n");
  printf("QSslError::SelfSignedCertificateInChain        10\n\n");

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


