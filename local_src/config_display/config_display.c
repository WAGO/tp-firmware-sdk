//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019 WAGO Kontakttechnik GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
///
/// \file    config_display.c
///
/// \version $Id: config_display.c 43946 2019-10-23 11:10:18Z wrueckl_elrest $
///
/// \brief   change display settings / config-tools
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
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <errno.h>
#include <unistd.h>

#include "msgtool.h"
#include "config_tool_lib.h"


//------------------------------------------------------------------------------
// Local macros
//------------------------------------------------------------------------------

#define CNF_FILE  DISPLAY_CONF_FILE

#define MIN_TIMEOUT   5
#define MAX_TIMEOUT  99

//------------------------------------------------------------------------------
// External variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local typedefs
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local defines
//------------------------------------------------------------------------------
static const char EepromDevice[] = "/sys/bus/i2c/devices/1-0054/eeprom";
#define SYSFS_EEPROM_WP_PATH	"/sys/class/gpio/gpio23/value"
#define DISPLAY_ROTATION_ADDR	0x01F8

//------------------------------------------------------------------------------
// Local variables
//------------------------------------------------------------------------------

/// configuration list start pointer
tConfList * g_pList = NULL;

/// \brief Config data in form of name, default value, value list comma separated (no spaces)
static tConfValues aConfigValues[] =
{
  { "orientation", "landscape", "landscape,landscape180,portrait,portrait270" },
  { "care", "enabled", "enabled,disabled" },
  { "caretime", "03:00:00", "" },
  { "calibonboot", "yes", "yes,no" },
  { "calibtimeout", "20", "" }, 
  { "cleaningtimeout", "15", "" }, 

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
int IsTimeStringValid(char * pTimeStr);
int eeprom_open(void);
int WriteBlock(int fd, unsigned int iMemAddress, char *cData, unsigned short usDataCnt);

/// \brief main function
/// \param[in]  argc number of arguments
/// \param[in]  argv arguments
/// \retval 0  SUCCESS
int main(int argc, char **argv)
{
  int status = ERROR;
  int iRetVal;
  char szArgv[256] = "";
  char cTempBuf[6];
  int ret_systemcall = ERROR;
  
  int restart_brightness_control = 0;
 
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
      int i, k, ret, fd;
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

            if (stricmp(aConfigValues[k].nameStr, "orientation") == 0)
            {
              
              status = ERROR;
              fd = eeprom_open();
              if (fd < 0)
              {
                status = ERROR;
              }
              else
              {
                status = SUCCESS;
              }
              
              if ((stricmp(pStr, "landscape") == 0) && (status == SUCCESS))
              {
                //write orientation value 0..3 into EEPROM
                //int ret_systemcall = system("write_config_eeprom -set_rotation 0 > /dev/null 2>&1");
                cTempBuf[0] = 0;
                iRetVal = WriteBlock(fd, (int)DISPLAY_ROTATION_ADDR, cTempBuf, 1);
                if (iRetVal < 0) {
                  ret_systemcall = ERROR;
                  close(fd);
                }
                else
                  ret_systemcall = SUCCESS;
                close(fd);
                
                if (ret_systemcall == SUCCESS)
                { 
                  status = SUCCESS;
                }  
                else
                {                  
                  char szErrTxt[256];
                  sprintf(szErrTxt, "Error writing EEPROM (display rotation) %s", pStr);
                  setRgbLed(RGB_LED_STATE_RE_BLINK, szErrTxt);
                  SetLastError(szErrTxt);
                  status = ERROR;
                }
              }
              else if ((stricmp(pStr, "landscape180") == 0) && (status == SUCCESS))
              {
                //write orientation value 0..3 into EEPROM
                //int ret_systemcall = system("write_config_eeprom -set_rotation 2 > /dev/null 2>&1");
                cTempBuf[0] = 2;
                iRetVal = WriteBlock(fd, (int)DISPLAY_ROTATION_ADDR, cTempBuf, 1);
                if (iRetVal < 0) {
                  ret_systemcall = ERROR;
                  close(fd);
                }
                else
                  ret_systemcall = SUCCESS;
                close(fd);

                if (ret_systemcall == SUCCESS)
                { 
                  status = SUCCESS;
                }  
                else
                {                  
                  char szErrTxt[256];
                  sprintf(szErrTxt, "Error writing EEPROM (display rotation) %s", pStr);
                  setRgbLed(RGB_LED_STATE_RE_BLINK, szErrTxt);
                  SetLastError(szErrTxt);
                  status = ERROR;
                }
              }
              else if ((stricmp(pStr, "portrait") == 0) && (status == SUCCESS))
              {
                //write orientation value 0..3 into EEPROM
                //int ret_systemcall = system("write_config_eeprom -set_rotation 1 > /dev/null 2>&1");
                cTempBuf[0] = 1;
                iRetVal = WriteBlock(fd, (int)DISPLAY_ROTATION_ADDR, cTempBuf, 1);
                if (iRetVal < 0) {
                  ret_systemcall = ERROR;
                  close(fd);
                }
                else
                  ret_systemcall = SUCCESS;
                close(fd);

                if (ret_systemcall == SUCCESS)
                { 
                  status = SUCCESS;
                }  
                else
                {                  
                  char szErrTxt[256];
                  sprintf(szErrTxt, "Error writing EEPROM (display rotation) %s", pStr);
                  setRgbLed(RGB_LED_STATE_RE_BLINK, szErrTxt);
                  SetLastError(szErrTxt);
                  status = ERROR;
                }
              }
              else if ((stricmp(pStr, "portrait270") == 0) && (status == SUCCESS))
              {
                //write orientation value 0..3 into EEPROM
                //int ret_systemcall = system("write_config_eeprom -set_rotation 3 > /dev/null 2>&1");
                cTempBuf[0] = 3;
                iRetVal = WriteBlock(fd, (int)DISPLAY_ROTATION_ADDR, cTempBuf, 1);
                if (iRetVal < 0) {
                  ret_systemcall = ERROR;
                  close(fd);
                }
                else
                  ret_systemcall = SUCCESS;
                close(fd);

                if (ret_systemcall == SUCCESS)
                { 
                  status = SUCCESS;
                }  
                else
                {                  
                  char szErrTxt[256];
                  sprintf(szErrTxt, "Error writing EEPROM (display rotation) %s", pStr);
                  setRgbLed(RGB_LED_STATE_RE_BLINK, szErrTxt);
                  SetLastError(szErrTxt);
                  status = ERROR;
                }
                
              }
              else
              {
                AppendErrorText(INVALID_PARAMETER, "Orientation value is not valid");
                status = INVALID_PARAMETER;  
              }
              
            }
            else if (stricmp(aConfigValues[k].nameStr, "caretime") == 0)
            {
              if (IsTimeStringValid(pStr) == SUCCESS)
              {
                if (ConfSetValue(&aConfigValues[0], g_pList, aConfigValues[k].nameStr, pStr)==SUCCESS)
                {
                  status = SUCCESS;
                  restart_brightness_control = 1;
                }                
              }
              else
              {
                status = ERROR;
              }              
            }
            else if (stricmp(aConfigValues[k].nameStr, "calibtimeout") == 0)
            {
              status = ERROR;
              if (ConfIsNumber(pStr) == SUCCESS)
              {
                int n = atoi(pStr);
                if ((n >= MIN_TIMEOUT) && (n <= MAX_TIMEOUT))
                {
                  if (ConfSetValue(&aConfigValues[0], g_pList, aConfigValues[k].nameStr, pStr)==SUCCESS)
                  {
                    status = SUCCESS;
                  }
                }
              }
            }
            else if (stricmp(aConfigValues[k].nameStr, "cleaningtimeout") == 0)
            {
              status = ERROR;
              if (ConfIsNumber(pStr) == SUCCESS)
              {
                int n = atoi(pStr);
                if ((n >= MIN_TIMEOUT) && (n <= MAX_TIMEOUT))
                {
                  if (ConfSetValue(&aConfigValues[0], g_pList, aConfigValues[k].nameStr, pStr)==SUCCESS)
                  {
                    status = SUCCESS;
                  }
                }
              }
            }
            else if (ConfSetValue(&aConfigValues[0], g_pList, aConfigValues[k].nameStr, pStr)==SUCCESS)
            {              
              status = SUCCESS;
              if (stricmp(aConfigValues[k].nameStr, "care") == 0)
              {
                restart_brightness_control = 1;
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
          
          if (restart_brightness_control)
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
  printf("\n* Change display settings * \n\n");
  printf("Usage: config_display [orientation=orientation-value]\n");
  printf("                      [care=care-value]\n");
  printf("                      [caretime=caretime-value]\n");
  printf("                      [calibonboot=calibonboot-value]\n");
  printf("                      [calibtimeout=calibtimeout-value]\n");
  printf("                      [cleaningtimeout=cleaningtimeout-value]\n\n");
  
  printf("orientation-value:     landscape | landscape180 | portrait | portrait270\n");
  printf("care-value:            enabled | disabled\n");
  printf("caretime-value:        timestamp in format [hh:mm:ss] f.e. 03:00:00\n");
  printf("calibonboot-value:     yes | no\n");
  printf("calibtimeout-value:    timeout for each crosshair [s]\n");
  printf("cleaningtimeout-value: countdown [s] cleaning mode\n\n");    
  
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

/// \brief Check if time string format is correct 
///
/// \param[in]  pTimeStr        time string
/// \retval  0  string is correct
/// \retval -1  string is not correct
int IsTimeStringValid(char * pTimeStr)
{
  int status = ERROR;  
  char * pStr = pTimeStr;
   
  if (strlen(pStr) == 8)
  {
    if ((pStr[2]==':') && (pStr[5]==':'))
    {
      char szHour[10];
      char szMin[10];
      char szSec[10];
      strncpy(szHour, pStr, 2);    
      szHour[2] = 0;    
      pStr+=3;
      
      //printf("szHour %s\n", szHour);
      
      strncpy(szMin, pStr, 2);
      szMin[2] = 0;    
      pStr+=3;
      
      //printf("szMin %s\n", szMin);
      
      strncpy(szSec, pStr, 2);
      szSec[2] = 0;    
      
      //printf("szSec %s\n", szSec);
      
      if (ConfIsNumber(szHour) == SUCCESS)
      { 
        int n = atoi(szHour);
        if ((n >= 0) && (n < 24))
        {
          if (ConfIsNumber(szMin) == SUCCESS)
          {
            n = atoi(szMin);
            if ((n >= 0) && (n < 60))
            {
              if (ConfIsNumber(szSec) == SUCCESS)
              {
                 n = atoi(szSec);
                 if ((n >= 0) && (n < 60))
                 {
                   status = SUCCESS;  
                 }
              }            
            }            
          }          
        }        
      }    
    }
  }

  if (status == ERROR)
    AppendErrorText(INVALID_PARAMETER, "Wrong time format, use hh:mm:ss f.e. 23:59:59");
      
  return status;    
}

/// \brief open eeprom device
/// \param[in]  void
/// \retval fd SUCCESS, -1 ERROR
int eeprom_open(void)
{
  int fd;

  fd = open(EepromDevice, O_RDWR);
  if(fd == -1)
    printf("open device %s failed \n", EepromDevice);

  return fd;
}

/// Write data to EEPROM
///
/// \param fd           eeprom file descriptor
/// \param iMemAddress  eeprom memory address
/// \param cData        pointer to input data buffer
/// \param usDataCnt    write byte data count   
///
/// \return  0 SUCCESS
/// \return -1 ERROR
int WriteBlock(int fd, unsigned int iMemAddress, char *cData, unsigned short usDataCnt)
{
  int sysfs_fd;
  char eeprom_wp_off='0';
  char eeprom_wp_on='1';
  
  sysfs_fd = open("/sys/class/gpio/gpio23/value", O_WRONLY);
  if(sysfs_fd < 0) {
    printf("could not open '/sys/class/gpio/gpio23/value!\n");
		return -1;
  }
  write(sysfs_fd, &eeprom_wp_off, 1);
  
  if (lseek(fd, iMemAddress, SEEK_SET) < 0) {
    printf("lseek failed \n");
    write(sysfs_fd, &eeprom_wp_on, 1);
    close(sysfs_fd);
    return -1;
  }

  if( (write(fd, cData, usDataCnt)) < 0 ) {
    printf("eeprom write block failed \n");
    write(sysfs_fd, &eeprom_wp_on, 1);
    close(sysfs_fd);
    return -1;
  }

  write(sysfs_fd, &eeprom_wp_on, 1);
  close(sysfs_fd);  
  return 0;
}
