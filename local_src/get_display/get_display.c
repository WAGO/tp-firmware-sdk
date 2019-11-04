//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019 WAGO Kontakttechnik GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
///
/// \file    get_display.c
///
/// \version $Id: get_display.c 44064 2019-10-24 12:34:47Z wrueckl_elrest $
///
/// \brief   get display settings / config-tools
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "msgtool.h"
#include "config_tool_lib.h"

//------------------------------------------------------------------------------
// Local macros
//------------------------------------------------------------------------------

#define CNF_FILE            DISPLAY_CONF_FILE
#define DISPLAY_ROTATION_ADDR   0x1F8
static const char EepromDevice[] = EEPROM_DEVICE;

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
int GetOrientationValue();

/// \brief main function
/// \param[in]  argc number of arguments
/// \param[in]  argv arguments
/// \retval 0  SUCCESS
int main(int argc, char **argv)
{
  int status = ERROR;  
  int iErrorCounter = 0;
  char szOut[256] = "";
  char szErrorTxt[256] = "";
  
  if (argc >= 2)
  {
    if ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0))
    {
      ShowHelpText();
      status = SUCCESS;
    }
    else
    {
      int i, ret, iOrientation;
      char * pStr;
      
      //create list
      g_pList = ConfCreateList();
      
      if (FileExists(CNF_FILE) != SUCCESS)
      {
        //resettofactory, create file
        system("/etc/config-tools/config_display resettofactory > /dev/null 2>&1");  
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
      
      iOrientation = GetOrientationValue();
      if (iOrientation > 3)
      {                
        sprintf(szErrorTxt, "Invalid display rotation value %d)", iOrientation);
        setRgbLed(RGB_LED_STATE_RE_BLINK, szErrorTxt); 
        SetLastError(szErrorTxt);  
        iErrorCounter++;
      }
      else if (iOrientation < 0)  
      {
        iErrorCounter++;  
      }
             
      if ((strcmp(argv[1], "-a") == 0) || (strcmp(argv[1], "all") == 0) || (strcmp(argv[1], "all-json") == 0))
      {
        char szJson[4096] = "";
        char szLine[512] = "";
        char *psz = szJson;
        //print all values in JSON format
        status = SUCCESS;
        strcat(szJson, "{");
        
        //read display rotation from EEPROM
        switch (iOrientation)
        {
          case 0:
            //printf("landscape");
            status = SUCCESS;
            sprintf(szLine, "\"orientation\": \"%s\",", "landscape");          
            strcat(szJson, szLine);
          break;
          case 1:                  
            //printf("portrait");
            status = SUCCESS;
            sprintf(szLine, "\"orientation\": \"%s\",", "portrait");          
            strcat(szJson, szLine);
          break;
          case 2:
            //printf("landscape180");
            status = SUCCESS;
            sprintf(szLine, "\"orientation\": \"%s\",", "landscape180");          
            strcat(szJson, szLine);
          break;
          case 3:                  
            //printf("portrait270");
            status = SUCCESS;
            sprintf(szLine, "\"orientation\": \"%s\",", "portrait270");          
            strcat(szJson, szLine);
          break;
          
          default:
            status = ERROR; 
          break;
        }                
        
        psz += strlen(szJson);
        ConfPrintAllValuesJson(g_pList, psz);
        ConfRemoveJsonSeparatorFromEnd(&szJson[0]);
        strcat(szJson, "}");
        printf(szJson);
        
      }
      else
      {
              
        for (i = 1; i < argc; i++)
        {
          pStr = argv[i];
          
          if (stricmp(pStr, "orientation") == 0) 
          {
            if ((iOrientation >= 0) && (iOrientation < 4))
            {
              switch (iOrientation)
              {
                case 0:
                  printf("landscape");
                  status = SUCCESS;
                break;
                case 1:                  
                  printf("portrait");
                  status = SUCCESS;
                break;
                case 2:
                  printf("landscape180");
                  status = SUCCESS;
                break;
                case 3:                  
                  printf("portrait270");
                  status = SUCCESS;
                break;
              }
            }
            else
            {
              status = ERROR;
            }
          }
          else if (stricmp(pStr, "care") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "caretime") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "calibonboot") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "calibtimeout") == 0) 
          {
            if (ConfGetValue(g_pList, pStr, &szOut[0], sizeof(szOut)) == SUCCESS)
            {
              //printf("%s=%s\n", pStr, szOut);
              printf("%s", szOut);
              status = SUCCESS;
            }
          }
          else if (stricmp(pStr, "cleaningtimeout") == 0) 
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
  
  if (iErrorCounter > 0)
  {
    status = ERROR;
  }

  //clean up list memory
  ConfDestroyList(g_pList);

  return status;
}

/// \brief Print help text / usage to stdout
void ShowHelpText()
{  
  printf("\n* Read display settings * \n\n");
  printf("Usage:        get_display [options] <parameter>\n");
  printf("\n");
  printf("options:\n");
  printf("all-json print all values in JSON format \n"); 
  printf("  -h print this help\n");
  printf("\n");
  printf("parameter: orientation | care | caretime | calibonboot | cleaningtimeout\n");
  printf("\n");
  printf("orientation:     landscape | landscape180 | portrait | portrait270\n");
  printf("care:            enabled | disabled\n");
  printf("caretime:        timestamp in format [hh:mm:ss] f.e. 03:00:00\n");
  printf("calibtimeout:    timeout for each crosshair [s]\n");
  printf("calibonboot:     yes | no\n");
  printf("cleaningtimeout: countdown [s] cleaning mode\n\n");  

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


//------------------------------------------------------------------------------
/// Read data from EEPROM
///
/// \param fd           eeprom file descriptor
/// \param iMemAddress  eeprom memory address
/// \param cData        pointer to output data buffer
/// \param usDataCnt    count of data bytes to be read 
///
/// \return  0 SUCCESS
/// \return -1 ERROR
//------------------------------------------------------------------------------
int ReadBlock(int fd, unsigned int iMemAddress, char *cData, unsigned short usDataCnt)
{
  if (lseek(fd, iMemAddress, SEEK_SET) < 0) {
    printf("lseek failed \n");
    return -1;
  }

  if( (read(fd, cData, usDataCnt)) < 0 ) {
    printf("eeprom read block failed \n");
    return -1;
  }
  
  return 0;
}

/// open eeprom device
///
/// \param void
///
/// \return fd eeprom file descriptor
/// \return -1 ERROR
//------------------------------------------------------------------------------
int eeprom_open(void)
{
  int fd;

  fd = open(EepromDevice, O_RDWR);
  if(fd == -1)
  {
    printf("open device %s failed \n", EepromDevice);
  }

  return fd;
}

/// \brief read display orientation from EEPROM
///
/// \retval -1 ERROR
/// \retval 0  landscape
/// \retval 1  portrait
/// \retval 2  landscape 180°
/// \retval 3  portrait 270°
int GetOrientationValue()
{
  int ret = ERROR;
  int i2c_fd;
  char cTempBuf[6];
  
  char szErrorTxt[256] = "";  
  
  i2c_fd = eeprom_open();
  if (i2c_fd >= 0)
  {              
    if (ReadBlock(i2c_fd, (unsigned int) DISPLAY_ROTATION_ADDR, cTempBuf, 1) >= 0) 
    {
      ret = cTempBuf[0];  
    }
    else
    {
      sprintf(szErrorTxt, "Could not read EEPROM (display rotation)");
      setRgbLed(RGB_LED_STATE_RE_BLINK, szErrorTxt);
      SetLastError(szErrorTxt);
    }
    close(i2c_fd);    
  }
  else
  {
    sprintf(szErrorTxt, "Could not open EEPROM (display rotation)");
    setRgbLed(RGB_LED_STATE_RE_BLINK, szErrorTxt);  
    SetLastError(szErrorTxt);    
  }
    
  return ret;
}
