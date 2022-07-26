//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019-2022 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
/// \file tempd.c
///
/// \version $Id: tempd.c 65689 2022-03-11 14:37:43Z falk.werner@wago.com $
///
/// \brief operating hours counter
///
/// \author Ralf Gliese, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
#define RUNNING_DIR	"/tmp"
//#define LOCK_FILE	"exampled.lock"
//#define _PRINT_TEMPVAL 1
#define BS_CNT_ADDR   0x2000

typedef struct
{
  unsigned long     dwRange_10m;
  unsigned long     dwRange_10_0;
  unsigned long     dwRange0_10;
  unsigned long     dwRange10_20;
  unsigned long     dwRange20_30;
  unsigned long     dwRange30_40;
  unsigned long     dwRange40_50;
  unsigned long     dwRange50_60;
  unsigned long     dwRange60_70;
  unsigned long     dwRange70p;
} BS_COUNTER ;

#define LOG_FILE	"/tmp/tempd.log"
#define log_message(file, msg) system("echo msg >> file")

#define LM75_FILE       "/sys/bus/i2c/devices/1-0049/hwmon/hwmon0/temp1_input"
#define LM75_FILE_1     "/sys/bus/i2c/devices/1-0049/hwmon/hwmon1/temp1_input"

static const char EepromDevice[] = "/sys/bus/i2c/devices/1-0054/eeprom";

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
void signal_handler(sig)
int sig;
{
	switch(sig) {
	case SIGHUP:
		//log_message(LOG_FILE,"hangup signal catched");
		break;
	case SIGTERM:
		//log_message(LOG_FILE,"terminate signal catched");
		exit(0);
		break;
	}
}

//------------------------------------------------------------------------------
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
    printf("open device %s failed \n", EepromDevice);

  return fd;
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
  if (lseek(fd, iMemAddress, SEEK_SET) < 0)
  {
    printf("lseek failed \n");
    return -1;
  }

  if( (read(fd, cData, usDataCnt)) < 0 )
  {
    printf("eeprom read block failed \n");
    return -1;
  }
  
  return 0;
}

//------------------------------------------------------------------------------
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
#ifdef USE_EEPROM_WP  
  int sysfs_fd;
  char eeprom_wp_off='0';
  char eeprom_wp_on='1';
  
  sysfs_fd = open("/sys/class/gpio/gpio170/value", O_WRONLY);
  if(sysfs_fd < 0) {
    printf("could not open '/sys/class/gpio/gpio170/value!\n");
		return -1;
  }
  write(sysfs_fd, &eeprom_wp_off, 1);
#endif


  if (lseek(fd, iMemAddress, SEEK_SET) < 0) {
    printf("lseek failed \n");
#ifdef USE_EEPROM_WP    
    write(sysfs_fd, &eeprom_wp_on, 1);
    close(sysfs_fd);
#endif    
    return -1;
  }


  if( (write(fd, cData, usDataCnt)) < 0 ) {
    printf("eeprom write block failed \n");
#ifdef USE_EEPROM_WP    
    write(sysfs_fd, &eeprom_wp_on, 1);
    close(sysfs_fd);
#endif    
    return -1;
  }

#ifdef USE_EEPROM_WP
  write(sysfs_fd, &eeprom_wp_on, 1);
  close(sysfs_fd);  
#endif  
  return 0;
}

void resetBsCounter(BS_COUNTER * pCounter)
{
  int iRetVal;
  unsigned int memAddress = (unsigned int) BS_CNT_ADDR;
  
  int fd = eeprom_open();
  if (fd < 0)
    exit(EXIT_FAILURE);

  memset(pCounter, 0x00, sizeof(BS_COUNTER));
  
  iRetVal = WriteBlock(fd, memAddress, (char *)pCounter, sizeof(BS_COUNTER));
  if (iRetVal < 0)
  {
    printf("bs_cnt_reset failed %s \n", EepromDevice);
    close(fd);
    exit(EXIT_FAILURE);
  }
  
  close(fd);
}

int main(int argc, char **argv)
{
  pid_t pid, sid;
  int iRetVal;
  char cTempBuf[6], cMinuteCnt;
  float fTempVal,fTempValSum;
  BS_COUNTER bs_counter;
  int i2c_temp_sensor;
  FILE *fTmpData;
  unsigned int iMemAddress;
  char *szInVal;
  int fd;

  umask(022); // Change the file mode to 0644
  memset(&bs_counter, 0x00, sizeof(bs_counter));

  if (argc > 1)
  {
    szInVal = argv[1];
    if(*szInVal == 'h')
    {
      printf("h: print help \n");
      printf("d: reset bs_counter \n");
      printf("s: show actual bs_counter \n");
      printf("t: read temperature sensor and print value \n");
      printf("r: read byte from eeprom and print value, input parameter iMemAddress \n");
      printf("w: write byte to eeprom address, input parameter iMemAddress an byte value \n");
      printf("without parameter start bs_counter \n");
      exit(EXIT_SUCCESS);
    }
    else if(*szInVal == 'd')
    {
      printf("d: reset bs_counter \n");
      resetBsCounter(&bs_counter);
      exit(EXIT_SUCCESS);
    }
    else if(*szInVal == 's')
    {
      printf("s: show actual bs_counter \n");
      fd = eeprom_open();
      if (fd < 0)
        exit(EXIT_FAILURE);

      iMemAddress = (int)BS_CNT_ADDR;
      iRetVal = ReadBlock(fd, iMemAddress, (char *)&bs_counter, sizeof(bs_counter));
      if (iRetVal != 0)
      {
        printf("eeprom read failed /dev/i2c-1 address 0x54 \n");
        close(fd);
        exit(EXIT_FAILURE);
      }
      close(fd);
      
      printf("bs_counter.dwRange_10m = %ld \n", bs_counter.dwRange_10m);
      printf("bs_counter.dwRange_10_0 = %ld \n", bs_counter.dwRange_10_0);
      printf("bs_counter.dwRange_0_10 = %ld \n", bs_counter.dwRange0_10);
      printf("bs_counter.dwRange_10_20 = %ld \n", bs_counter.dwRange10_20);
      printf("bs_counter.dwRange_20_30 = %ld \n", bs_counter.dwRange20_30);
      printf("bs_counter.dwRange_30_40 = %ld \n", bs_counter.dwRange30_40);
      printf("bs_counter.dwRange_40_50 = %ld \n", bs_counter.dwRange40_50);
      printf("bs_counter.dwRange_50_60 = %ld \n", bs_counter.dwRange50_60);
      printf("bs_counter.dwRange_60_70 = %ld \n", bs_counter.dwRange60_70);
      printf("bs_counter.dwRange_70p = %ld \n", bs_counter.dwRange70p);

      exit(EXIT_SUCCESS);
    }
    
    else if(*szInVal == 't')
    {
      //printf("t: read temperature sensor and print value \n");
      i2c_temp_sensor = open(LM75_FILE, O_RDONLY);
      if(i2c_temp_sensor < 0)
        i2c_temp_sensor = open(LM75_FILE_1, O_RDONLY);
      if(i2c_temp_sensor > 0)
      {
        read (i2c_temp_sensor, cTempBuf, 6);
        close(i2c_temp_sensor);
        fTempValSum += (atof(cTempBuf)) / 1000;
        //printf("fTempVal = %f \n", ((atof(cTempBuf)) / 1000));
        printf("%f\n", ((atof(cTempBuf)) / 1000));
      }
      else
      {
        printf("could not read temperature sensor \n");
        exit(EXIT_FAILURE);
      }
      close(i2c_temp_sensor);
      exit(EXIT_SUCCESS);
    }
    else if(*szInVal == 'r')
    {
      iMemAddress = 0;   
      if (argc > 2)
      {
        iMemAddress = atoi(argv[2]);
      }
      else
      {
        printf("r: read byte too few input parameters \n");
        exit(EXIT_FAILURE);
      }
 
      fd = eeprom_open();
      if (fd < 0)
        exit(EXIT_FAILURE);

      printf("r: read byte from eeprom address \n");        
 
      iRetVal = ReadBlock(fd, iMemAddress, cTempBuf, 1);
      if (iRetVal < 0)
      {
        printf("eeprom read failed %s iMemAddress = %d \n", EepromDevice, iMemAddress);
        close(fd);
        exit(EXIT_FAILURE);
      }
      printf("eeprom read data = %d \n", cTempBuf[0]);
  
      close(fd);
      exit(EXIT_SUCCESS);
    }
    else if(*szInVal == 'w')
    {
      iMemAddress = 0;   
      if (argc > 3)
      {
        iMemAddress = atoi(argv[2]);
        cTempBuf[0] = (char)atoi(argv[3]);
      }
      else
      {
        printf("w: read byte too few input parameters \n");
        exit(EXIT_FAILURE);
      }
      
      fd = eeprom_open();
      if (fd < 0)
        exit(EXIT_FAILURE);
      
      printf("w: write byte to eeprom address \n");

      iRetVal = WriteBlock(fd, iMemAddress, cTempBuf, 1);
      if (iRetVal < 0)
      {
        printf("eeprom write failed /dev/i2c-1 address 0x54 iMemAddress = %d \n", iMemAddress);
        close(fd);
        exit(EXIT_FAILURE);
      }
  
      close(fd);
      exit(EXIT_SUCCESS);
    }
  }
  
	if(getppid() == 1) return 0; /* already a daemon */
	pid = fork();
	if (pid < 0)
    exit(EXIT_FAILURE); /* fork error */
	if (pid > 0)
    exit(EXIT_SUCCESS); /* got a valid pid, parent exits */
	/* now child (daemon) continues */
  
  
  
	sid = setsid(); /* obtain a new process group */
  if (sid<0)
    exit(EXIT_FAILURE);
  
  /* Change the current working directory */
  if ((chdir(RUNNING_DIR)) < 0) {
    /* Log any failure here */
    exit(EXIT_FAILURE);
  }
        
  /* Close out the standard file descriptors */
  //close(STDIN_FILENO);
  //close(STDOUT_FILENO);
  //close(STDERR_FILENO);

  // read eeprom
  fd = eeprom_open();
  if (fd < 0)
    exit(EXIT_FAILURE);

  iMemAddress = (int)BS_CNT_ADDR;
  
  iRetVal = ReadBlock(fd, iMemAddress, (char *)&bs_counter, sizeof(bs_counter));
  if (iRetVal < 0)
  {
    printf("eeprom read failed /dev/i2c-1 address 0x54 \n");
    close(fd);
    exit(EXIT_FAILURE);
  }
  close(fd);

  // init EEPROM
  if (bs_counter.dwRange_10m == 0xFFFFFFFF)
  {
    printf("Init EEPROM (reset)\n");
    resetBsCounter(&bs_counter);
  }

  fTmpData = fopen("/tmp/tempdata.dat", "w+");
  if(fTmpData)
  {
    fprintf(fTmpData,"bs_counter.dwRange_10m = %ld \n", bs_counter.dwRange_10m);
    fprintf(fTmpData,"bs_counter.dwRange_10_0 = %ld \n", bs_counter.dwRange_10_0);
    fprintf(fTmpData,"bs_counter.dwRange_0_10 = %ld \n", bs_counter.dwRange0_10);
    fprintf(fTmpData,"bs_counter.dwRange_10_20 = %ld \n", bs_counter.dwRange10_20);
    fprintf(fTmpData,"bs_counter.dwRange_20_30 = %ld \n", bs_counter.dwRange20_30);
    fprintf(fTmpData,"bs_counter.dwRange_30_40 = %ld \n", bs_counter.dwRange30_40);
    fprintf(fTmpData,"bs_counter.dwRange_40_50 = %ld \n", bs_counter.dwRange40_50);
    fprintf(fTmpData,"bs_counter.dwRange_50_60 = %ld \n", bs_counter.dwRange50_60);
    fprintf(fTmpData,"bs_counter.dwRange_60_70 = %ld \n", bs_counter.dwRange60_70);
    fprintf(fTmpData,"bs_counter.dwRange_70p = %ld \n", bs_counter.dwRange70p);
  }
  else
  {
    printf("open data file failed \n");
    exit(EXIT_FAILURE);
  }
  fclose(fTmpData);

  /* reset minute count */
  cMinuteCnt = 0;
  fTempValSum = 0;
  
  //log_message(LOG_FILE, test);

	while(1)
  {
    sleep(60);
    cMinuteCnt++;
    i2c_temp_sensor = open(LM75_FILE, O_RDONLY); // PTE4XX kernel 3.6.11
    if(i2c_temp_sensor < 0)
      i2c_temp_sensor = open(LM75_FILE_1, O_RDONLY); 
    if(i2c_temp_sensor > 0)
    {
      read (i2c_temp_sensor, cTempBuf, 6);
      close(i2c_temp_sensor);
      fTempValSum += (atof(cTempBuf)) / 1000;
#ifdef _PRINT_TEMPVAL  
      printf("fTempVal = %f \n", ((atof(cTempBuf)) / 1000));
      printf("cMinuteCnt = %d \n", cMinuteCnt);
#endif
    }
    else
    {
      printf("could not read temperature sensor \n");
      exit(EXIT_FAILURE);
    }

    if (cMinuteCnt == 10)
    {
      fd = eeprom_open();
      if (fd < 0)
        exit(EXIT_FAILURE);
      iMemAddress = (int)BS_CNT_ADDR; /* RG TODO eeprom organized in 512pages a 128bytes */

      /* get actual bs_counter */
      iRetVal = ReadBlock(fd, iMemAddress, (char *)&bs_counter, sizeof(bs_counter));
      if (iRetVal < 0)
      {
        printf("eeprom read failed /dev/i2c-1 address 0x54 \n");
        close(fd);
        exit(EXIT_FAILURE);
      }
      fTempVal = fTempValSum/10;
#ifdef _PRINT_TEMPVAL
      printf("fTempVal mean value = %f \n", fTempVal);
#endif
      if (fTempVal < -10.0f)
        bs_counter.dwRange_10m += 10;
      else if ((fTempVal >= -10.0f) && (fTempVal < 0.0f))
        bs_counter.dwRange_10_0 += 10;
      else if ((fTempVal >= 0.0f) && (fTempVal < 10.0f))
        bs_counter.dwRange0_10 += 10;
      else if ((fTempVal >= 10.0f) && (fTempVal < 20.0f))
        bs_counter.dwRange10_20 += 10;
      else if ((fTempVal >= 20.0f) && (fTempVal < 30.0f))
        bs_counter.dwRange20_30 += 10;
      else if ((fTempVal >= 30.0f) && (fTempVal < 40.0f))
        bs_counter.dwRange30_40 += 10;
      else if ((fTempVal >= 40.0f) && (fTempVal < 50.0f))
        bs_counter.dwRange40_50 += 10;
      else if ((fTempVal >= 50.0f) && (fTempVal < 60.0f))
        bs_counter.dwRange50_60 += 10;
      else if ((fTempVal >= 60.0f) && (fTempVal < 70.0f))
        bs_counter.dwRange60_70 += 10;
      else
        bs_counter.dwRange70p += 10;
        
#ifdef _PRINT_TEMPVAL
      printf("bs_counter.dwRange_10m = %ld \n", bs_counter.dwRange_10m);
      printf("bs_counter.dwRange_10_0 = %ld \n", bs_counter.dwRange_10_0);
      printf("bs_counter.dwRange_0_10 = %ld \n", bs_counter.dwRange0_10);
      printf("bs_counter.dwRange_10_20 = %ld \n", bs_counter.dwRange10_20);
      printf("bs_counter.dwRange_20_30 = %ld \n", bs_counter.dwRange20_30);
      printf("bs_counter.dwRange_30_40 = %ld \n", bs_counter.dwRange30_40);
      printf("bs_counter.dwRange_40_50 = %ld \n", bs_counter.dwRange40_50);
      printf("bs_counter.dwRange_50_60 = %ld \n", bs_counter.dwRange50_60);
      printf("bs_counter.dwRange_60_70 = %ld \n", bs_counter.dwRange60_70);
      printf("bs_counter.dwRange_70p = %ld \n", bs_counter.dwRange70p);
#endif
      fTmpData = fopen("/tmp/tempdata.dat", "w+");
      if(fTmpData)
      {
        fprintf(fTmpData,"bs_counter.dwRange_10m = %ld \n", bs_counter.dwRange_10m);
        fprintf(fTmpData,"bs_counter.dwRange_10_0 = %ld \n", bs_counter.dwRange_10_0);
        fprintf(fTmpData,"bs_counter.dwRange_0_10 = %ld \n", bs_counter.dwRange0_10);
        fprintf(fTmpData,"bs_counter.dwRange_10_20 = %ld \n", bs_counter.dwRange10_20);
        fprintf(fTmpData,"bs_counter.dwRange_20_30 = %ld \n", bs_counter.dwRange20_30);
        fprintf(fTmpData,"bs_counter.dwRange_30_40 = %ld \n", bs_counter.dwRange30_40);
        fprintf(fTmpData,"bs_counter.dwRange_40_50 = %ld \n", bs_counter.dwRange40_50);
        fprintf(fTmpData,"bs_counter.dwRange_50_60 = %ld \n", bs_counter.dwRange50_60);
        fprintf(fTmpData,"bs_counter.dwRange_60_70 = %ld \n", bs_counter.dwRange60_70);
        fprintf(fTmpData,"bs_counter.dwRange_70p = %ld \n", bs_counter.dwRange70p);
      }
      else
      {
        printf("open data file failed \n");
        exit(EXIT_FAILURE);
      }
      fclose(fTmpData);
      
      iMemAddress = (int)BS_CNT_ADDR; /* RG TODO eeprom organized in 512pages a 128bytes */
      
      iRetVal = WriteBlock(fd, iMemAddress, (char *)&bs_counter, sizeof(bs_counter));
      if (iRetVal < 0)
      {
        printf("eeprom write failed /dev/i2c-1 address 0x54 \n");
        close(fd);
        exit(EXIT_FAILURE);
      }
      close(fd);
      
      cMinuteCnt = 0;
      fTempValSum = 0;
    }
  }

  exit(EXIT_SUCCESS);
}
