//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019 WAGO Kontakttechnik GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
///
/// \file    brightness_control.c
///
/// \version $Id: brightness_control.c 54500 2020-12-14 14:31:55Z wrueckl_elrest $
///
/// \brief   brightness control, display backlight settings
///
/// \author  Ralf Gliese, elrest Automationssysteme GmbH
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/input.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/file.h>
#include <sys/time.h>
#include <termios.h>
#include <signal.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <sys/inotify.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <linux/fb.h>
#include <ctype.h>


#include "config_tool_lib.h"
#include "msgtool.h"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

#define MAX_SENSOR_RANGES     20
#define MIN_SCREENSAVER_TIME  20 //seconds

#define MOTION_PATH           "/var/run/proximation"
#define BACKLIGHT_SYSFS_PATH  "/sys/class/backlight/backlight/"                               //CTP 

#define PID_FILE              "/var/run/brightness_control.pid"

//WTP
//#define AO_PWM_DRV_PATH       "/sys/kernel/ao_pwm_drv/"
//#define PWM_VALUE             "PWM_VALUE"

//CTP
#define AO_PWM_DRV_PATH       "/sys/class/leds/ao_pwm_led/"
#define PWM_VALUE             "brightness"

#define DEV_BRIGHTNESS        "/dev/brightness"

#define DEV_TOUCHSCREEN       "/dev/input/touchscreen"
#define DEV_CAPTOUCH          "/dev/input/captouch"

//modprobe ao_pwm_drv
//echo (0..255) > /sys/kernel/ao_pwm_drv/PWM_VALUE

//DEBUG
//#define DEBUG_MSG

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)

#define DISPLAY_ROTATION_ADDR   0x1F8

#define ADC_MIN_ADDR    0x4100
#define ADC_MAX_ADDR    0x4102

//#define MIN(a,b) (((a)<(b))?(a):(b))
//#define MAX(a,b) (((a)>(b))?(a):(b))

//WTP #define EEPROM_DEVICE_WTP            "/sys/class/i2c-adapter/i2c-1/1-0054/eeprom"; 
//WTP static const char EepromDevice[] = EEPROM_DEVICE_WTP;

//CTP
static const char EepromDevice[] = EEPROM_DEVICE;

/// data struct used by the screensaver thread function
typedef struct ThreadDataScreenSaver
{
  unsigned char thread_running;        ///< thread is running flag
  unsigned char running_screensaver;   ///< screensaver is active flag
  unsigned char event_received;        ///< event occured flag
  unsigned char mode;                  ///< screensaver mode
  unsigned char quit;                  ///< quit thread flag
  int theme;                            ///< screensaver theme
  char szImage[256];                    ///< screensaver image file name
  char szText[256];                     ///< screensaver text
} tThreadDataScreenSaver;

/// data struct used by the event listener thread function
typedef struct ThreadDataEventListener
{
  unsigned char thread_running;  ///< thread is running flag
  unsigned char event_received;  ///< event occured flag
  int event_type;                ///< event type
  unsigned char quit;            ///< quit thread flag
  unsigned char step;            ///< adjust backlight with +/- keys 
  int offset;                     ///< additive backlight value
} tThreadDataEventListener;

/// time struct required by daytime nighttime switch
typedef struct TimeOfDay
{
  int hour;    ///< hours
  int min;     ///< minutes
  int sec;     ///< seconds
}tTimeOfDay;

/// data struct used by brightness sensor 
typedef struct SensorRanges
{
  int from;    ///< range start point
  int to;      ///< range end point
  int val;     ///< backlight value for this range
} tSensorRanges;

/// data struct backlight configuration
typedef struct BrightnessData
{
  unsigned char backlighton;        ///< backlight on value 0..255
  unsigned char backlightonnight;   ///< on value if night-time
  unsigned char backlightoff;       ///< backlight off value 0..255
  unsigned char backlightoffnight;  ///< off value if night-time
  unsigned char mastermode;         ///< analog in/out master, slave, none
  unsigned char interface;          ///< voltage or ampere based master-slave communication
  unsigned char daynightswitch;     ///< day-night-switch activated
  unsigned char sensorbrightness;   ///< backlight depends on brightness sensor 
  unsigned char sensormotion;       ///< motion sensor wakes up screensaver  
  char sensordirection[10];         ///< Direction: left right front
  unsigned char step;
  int rangeCounter;                  ///< number of ranges using brightness sensor 
  struct TimeOfDay daytime;        ///< day-time configuration  
  struct TimeOfDay nighttime;      ///< night-time configuration
  struct SensorRanges aRanges[MAX_SENSOR_RANGES];   ///< array of ranges using brightness sensor
  int lastRangeIndex;               ///< last brightness range index (hysteresis)
  int sensorHysteresisDiff;         ///< configured hysteresis difference, brightness sensor
  int adcValue0V;                   ///< EEPROM calibration ADC value 0V
  int adcValue1V;                   ///< ADC value 1 V
  int adcValue10V;                  ///< EEPROM calibration ADC value 10V
  int adcLastValidValue;            ///< last 10 bit ADC measurement value
  int minBrightness4Slave;          ///< UL 0 is allowed, Benelux minvalue has to be visible
  int adcHysteresisDiff;            ///< configured hysteresis difference, ADC input value
  char useOffset;                   ///< use +/- keys 
} tBrightnessData;

/// data struct used by the cmd thread function
typedef struct ThreadDataCmd
{
  unsigned char thread_running;        ///< thread is running flag
  unsigned char quit;                  ///< quit thread flag
  unsigned char initialized;           ///< device is ready
  struct BrightnessData * pBrightnessData;
} tThreadDataCmd;


/// data struct screensaver state
typedef struct ScreenData
{
  unsigned char state;       ///< screensaver enabled
  int time;                   ///< screensaver interval [s]
}tScreenData;

/// data struct display care state
typedef struct DisplayData
{
  unsigned char enabled;        ///< display care enabled
  unsigned char called;         ///< display care enabled    
  struct TimeOfDay caretime;
  time_t tLastEventReceived;
}tDisplayData;


/// master-slave mode interface
enum eMS_IFACE
{
  MS_IFACE_VOLT = 0,   ///< use voltage interface
  MS_IFACE_AMPERE      ///< use ampere interface
};

/// analog in / out configuration
enum eAIO
{
  AIO_NONE = 0,   ///< do not use analog in / out
  AIO_MASTER,     ///< device is master
  AIO_SLAVE       ///< device is slave
};

/// screensaver mode
enum eSCREENSAVER
{
  SCREENSAVER_BACKLIGHT,  ///< screensaver mode backlight off
  SCREENSAVER_THEME       ///< screensaver mode theme img, txt, time
};

/// display type
enum eDISPLAYTYPE
{
  DISPLAY_TYPE_480_272,
  DISPLAY_TYPE_640_480,
  DISPLAY_TYPE_800_480,
  DISPLAY_TYPE_800_600,
  DISPLAY_TYPE_1280_800,
  
  DISPLAY_TYPE_END
};

/// axis
enum eAXIS
{
  AXIS_X,
  AXIS_Y,
  
  AXIS_END
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

void AppendErrorText(int iStatusCode, char * pText);
int InitBrightnessControl();
int SetBacklightValue(int iValue);
int GetBacklightValue();
int RefreshBacklightSettings();
int TurnScreensaverBacklightOn();
int IsNightTime();
int GetSensorBrightnessValue();
int GetScreensaverActivity();
int GetScreenCareActivity();
int ReadAIn(unsigned short * pAdcVal);
int GetClearScreenActivity();
int GetUsbKeyboardDeviceName(char * pInputDeviceName, int maxlen);
void CallScreenCare();
int GetOrientationValue();
int ReadScreenSize();
int GetADC_MinValue();
int GetADC_MaxValue();

//threads
int StartEventListenerThread();
int StartScreenSaverThread();
int StartCmdListenerThread();
void ThreadFctScreensaver(void * ptr);
void ThreadFctEventListener(void * ptr);
void ThreadFctCmd(void * ptr);
void CancelThreads();

//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables’ and constants’ definitions
//------------------------------------------------------------------------------

//char bEventTrigger, bBlPowerDown, bBlSleeping;
static pthread_t thread_screensaver;
static pthread_t thread_listener;
static pthread_t thread_cmd;
static tThreadDataScreenSaver threadDataScreenSaver;
static tThreadDataEventListener threadDataEventListener;
static tThreadDataCmd threadDataCmd;
static tBrightnessData g_brightnessData;
static tScreenData g_screensaverData;
static tDisplayData g_displayData;

//mutex backlight
//static pthread_mutex_t mutex_backlight = PTHREAD_MUTEX_INITIALIZER;
//static pthread_cond_t cond_backlight = PTHREAD_COND_INITIALIZER;

//critical section cs_
static pthread_mutex_t cs_sysfs = PTHREAD_MUTEX_INITIALIZER; // PTHREAD_RECURSIVE_MUTEX_INITIALIZER
static pthread_mutex_t mutex_offset = PTHREAD_MUTEX_INITIALIZER; // PTHREAD_RECURSIVE_MUTEX_INITIALIZER

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

int anumber(char *s) 
{
  if (*s == '\0')
    return 0;
    
  while (*s != '\0') 
  {
    if (!isdigit(*s++))
      return 0;
  }
  return 1; 
}

/// \brief cancel threads before exit program
///
/// \param[in]  sig  number
void handler (int sig)
{
  CancelThreads();  
#ifdef DEBUG_MSG
  printf ("\nexiting...(%d)\n", sig);
#endif  
  exit (0);
}

/// \brief show perror text and exit program
///
/// \param[in]  error  text
void perror_exit (char *error)
{
  perror (error);
  handler (9);
}

/// \brief signal handler SIGTERM
///
/// \param[in]  foo  number
static inline void sig_handler_term(int foo)
{
  //cancel threads before terminate
  CancelThreads();
#ifdef DEBUG_MSG  
  printf("exit: sig_handler kill \n");      
#endif  
  usleep(100*1000);
  exit(1);
}

/// \brief signal handler SIGINT
///
/// \param[in]  foo  number
static inline void sig_handler_int(int foo)
{
  //cancel threads before ctrl-c
  CancelThreads();
#ifdef DEBUG_MSG
  printf("exit: sig_handler ctrl-c \n");
#endif  
  usleep(100*1000);
  exit(1);
}

/// \brief Thread function backlight / theme screensaver
///
/// \param[in]  ptr  pointer to struct tThreadDataScreenSaver
void ThreadFctScreensaver(void * ptr)
{
  tThreadDataScreenSaver *data;
  
  struct timespec timespecLastEventReceived;
  struct timespec timespecNow;
  
  //time_t tLastEventReceived, tNow;
  time_t iSecondsToWait;
  char * pCmdClose = "close\n";
  
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  
  data = (tThreadDataScreenSaver *) ptr;   
  if (!data)
    pthread_exit(0); 
    
  data->thread_running = 1;
  iSecondsToWait = g_screensaverData.time;
  
  clock_gettime(CLOCK_MONOTONIC, &timespecLastEventReceived);
    
  while (!data->quit)
  {
    if (data->running_screensaver)
    {
      char szCmd[256] = "/usr/bin/screensaverqt";
      
      if (GetScreensaverActivity())
      {
        //already started
        //screensaverqt called from WBM
#ifdef DEBUG_MSG
        printf("screensaver already running\n");
#endif        
        system("killall -9 screensaverqt > /dev/null 2>&1");  
        usleep(100*1000);
      }           

      if (data->mode == SCREENSAVER_THEME)
      {
        switch (data->theme)
        {
          case 1:
            //THEME_IMAGE
            sprintf(szCmd, "/usr/bin/screensaverqt %d %s", data->theme, data->szImage);        
            break;
          case 2:
            //THEME_TIME
            sprintf(szCmd, "/usr/bin/screensaverqt %d", data->theme);        
            break;
          case 3:
            //THEME_TEXT
            sprintf(szCmd, "/usr/bin/screensaverqt %d \"%s\"", data->theme, data->szText);        
            break;
          default:  
            sprintf(szCmd, "/usr/bin/screensaverqt");            
        }
      }
      else
      {
        //SCREENSAVER_BACKLIGHT
        TurnScreensaverBacklightOn();
        sprintf(szCmd, "/usr/bin/screensaverqt");    
      }
      
      //close virtualkeyboard      
      Write2PipedFifo(DEV_VIRTUALKEYBOARD, pCmdClose); 
      
      //call screensaver app and wait      
      system(szCmd); 
      
      RefreshBacklightSettings();  

      data->running_screensaver = 0;
      clock_gettime(CLOCK_MONOTONIC, &timespecLastEventReceived);
    }
          
    sleep(1);
    
    if (data->event_received)
    {
      data->event_received = 0;
      clock_gettime(CLOCK_MONOTONIC, &timespecLastEventReceived);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &timespecNow);
    if ((timespecNow.tv_sec - timespecLastEventReceived.tv_sec) > iSecondsToWait)
    {
      //time expired
      data->running_screensaver = 1;
    }
  }
    
  data->thread_running = 0;  
  pthread_exit(0); 
}

void remap_qt1070_keys(int qt1070)
{
	int i, codes[][2] = {
		{ 3, KEY_BRIGHTNESSUP }, { 4, KEY_BRIGHTNESSDOWN },
	};
	for (i=0; i < sizeof codes / sizeof codes[0]; i++) {
		if (ioctl(qt1070, EVIOCSKEYCODE, codes[i]) < 0)
			printf("Error changing scan code(%d) %d->%d: %s\n",
				i, codes[i][0], codes[i][1], strerror(errno));
	}
}

/// \brief Thread function event listener
///
/// \param[in]  ptr  pointer to struct tThreadDataEventListener
void ThreadFctEventListener(void * ptr)
{
  tThreadDataEventListener *data;     
  struct input_event ev[64];
  struct pollfd pollfds[8];
  char szName[256] = "Unknown";
  char szDevice[256] = "";
  char szUsbDevice[128] = "";
  int iRetVal, rd, size = sizeof (struct input_event);
  int fd;
  int i = 0, k;
  int n, num, pollincount, inotify_fd = -1;
  char updateDevices = 1;
  
  memset(&pollfds, 0x00, sizeof(pollfds));  
  
#ifdef DEBUG_MSG
  printf("ThreadFctEventListener\n");
#endif  
                
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);     
  data = (tThreadDataEventListener *) ptr;   
  if (!data) 
  { 
#ifdef DEBUG_MSG      
    printf("ThreadFctEventListener: data pointer is null \n");
#endif        
    pthread_exit(0); 
  }
  
  data->thread_running = 1;

  WriteSysFs("/sys/class/leds/bright-plus/",  "trigger", "oneshot");
  WriteSysFs("/sys/class/leds/bright-minus/", "trigger", "oneshot");
  //thread loop
  while (!data->quit)
  {
    //update input devices listening to
    if (updateDevices)
    {
      updateDevices = 0;
#ifdef DEBUG_MSG      
      printf("UPDATE INPUT DEVICES\n");
#endif    
      for (n=0; n < i; n++)
      {
        //close fd
        if (pollfds[n].fd > 0)
        {
          //printf("close pollfds[%d].fd %d\n", n, pollfds[n].fd);
          close(pollfds[n].fd);
          pollfds[n].fd = -1;
        }
      }

      /* RG TODO open every device according device configuration
       * if open successful add it to pollfds[] */
      i = 0;
      //Open Device  
      if (FileExistsWithoutSizeCheck(DEV_CAPTOUCH) == 0)
      {
        strcpy(szDevice, DEV_CAPTOUCH);
      }
      else
      {
        strcpy(szDevice, DEV_TOUCHSCREEN);
      }
      if ((fd = open (szDevice, O_RDONLY)) > 0)
      {
        //printf ("device %s open successful \n", device);
        pollfds[i].fd = fd;
        pollfds[i].events=POLL_IN;
        //Print Device Name
        ioctl (pollfds[i].fd, EVIOCGNAME (sizeof (szName)), szName);
        //printf ("Reading From : %s (%s)\n", device, name);
        i++;
      }
      else
      {
        char szErrorText[256];
        sprintf(szErrorText, "brightness: %s is not a vaild device", szDevice);
        setRgbLed(RGB_LED_STATE_RE_BLINK, szErrorText);
        SetLastError(szErrorText);

      }
      
      //only available if keypad present
      strcpy(szDevice, "/dev/input/keypad");
      if ((fd = open (szDevice, O_RDONLY)) > 0)
      {
        //printf ("device %s open successful \n", device);
        pollfds[i].fd = fd;
        pollfds[i].events=POLL_IN;
        //Print Device Name
        ioctl (pollfds[i].fd, EVIOCGNAME (sizeof (szName)), szName);
        //printf ("Reading From : %s (%s)\n", device, name);
        i++;
      }
         
      
      if (g_brightnessData.sensormotion)
      {
        if (inotify_fd != -1)
          close(inotify_fd);
        inotify_fd = inotify_init();
        if (inotify_fd >= 0 &&
            inotify_add_watch(inotify_fd, MOTION_PATH, IN_CLOSE_WRITE) != -1)
        {
          //printf ("device %s open successful \n", device);
          pollfds[i].fd = inotify_fd;
          pollfds[i].events=POLL_IN;
          i++;
        }
        else
        {
          char szErrorText[256];
          sprintf(szErrorText, "brightness: %s is not a vaild device", MOTION_PATH);
          setRgbLed(RGB_LED_STATE_RE_BLINK, szErrorText);
          SetLastError(szErrorText);
          if (inotify_fd != -1)
            close(inotify_fd);
          inotify_fd = -1;
        }
      }
      
      //qt1070
      strcpy(szDevice, "/dev/input/qt1070");
      if ((fd = open (szDevice, O_RDONLY)) > 0)
      {
        //printf ("device %s open successful \n", device);
        pollfds[i].fd = fd;
        pollfds[i].events=POLL_IN;
        //Print Device Name
        ioctl (pollfds[i].fd, EVIOCGNAME (sizeof (szName)), szName);
        //printf ("Reading From : %s (%s)\n", device, name);
        i++;
        remap_qt1070_keys(fd);
      }
      else
      {
        char szErrorText[256];
        sprintf(szErrorText, "brightness: %s is not a vaild device", szDevice);
        setRgbLed(RGB_LED_STATE_RE_BLINK, szErrorText);
        SetLastError(szErrorText);
      }    
      
      
      //only available if usb mouse is present
      strcpy(szDevice, "/dev/input/mouse1");
      if ((fd = open (szDevice, O_RDONLY)) > 0)
      {
        //printf ("device %s open successful \n", device);
        pollfds[i].fd = fd;
        pollfds[i].events=POLL_IN;
        //Print Device Name
        ioctl (pollfds[i].fd, EVIOCGNAME (sizeof (szName)), szName);
        //printf ("Reading From : %s (%s)\n", szDevice, szName);
        i++;
      }


      //grep -E 'Handlers|EV=' /proc/bus/input/devices | grep -B1 'EV=120013' | grep -Eo 'event[0-9]+'
      if (GetUsbKeyboardDeviceName(szUsbDevice, sizeof(szUsbDevice)) == 0)
      {
        sprintf(szDevice, "/dev/input/%s", szUsbDevice);
        if ((fd = open (szDevice, O_RDONLY)) > 0)
        {
          //printf ("device %s open successful \n", device);
          pollfds[i].fd = fd;
          pollfds[i].events=POLL_IN;
          //Print Device Name
          ioctl (pollfds[i].fd, EVIOCGNAME (sizeof (szName)), szName);
          //printf ("Reading From : %s (%s)\n", szDevice, szName);
          i++;
        }
      }
      
      if (i == 0)
      {
        char szErrorText[256];
        sprintf(szErrorText, "brightness: no valid input device found");
        setRgbLed(RGB_LED_STATE_RE_BLINK, szErrorText);
        SetLastError(szErrorText);

        data->thread_running = 0;
        pthread_exit(0); 
      }
    
    }

    // wait for input
    //iRetVal = poll(&pollfds, 1, -1);
    //iRetVal = poll(&pollfds, i, -1);
    iRetVal = poll(&pollfds[0], i, 500); //timeout 500 [ms]
    if(iRetVal ==  -1)
    {                          
      //error                            
#ifdef DEBUG_MSG      
      printf ("listener thread poll error: errno %d \n", errno);
#endif      
      usleep(500 * 1000);  
      continue;
    }
    else if (iRetVal == 0)
    {
      //timeout
      continue;
    }
    
    //printf("poll returns %d \n", iRetVal);
    //printf("->touch_thread_fkt wait \n");
    
    pollincount = 0;
    
    for (k=0; k < i; k++)
    {
      if(pollfds[k].revents & POLL_IN)
      {
        pollincount++;
        if (pollfds[k].fd == inotify_fd) {
          char buf[16];
          int len;
          struct inotify_event ev;

          /* Read and ignore the inotify data. We only watch one file. */
          read(inotify_fd, &ev, sizeof ev);
          /* ReadSysFs() is buggy and does not \0 terminate the buffer. */
          memset(buf, 0, sizeof buf);
          /* MOTION_PATH is actually not sysfs, but ReadSysFs does not care. */
          ReadSysFs(MOTION_PATH, "", buf, sizeof buf -1);
          len = strlen(buf);
          if (len > 0 && buf[len -1] == '\n')
            buf[len -1] = 0;
          //printf("MOTION: '%s' '%s'\n", buf, g_brightnessData.sensordirection);
          /* Either the current proximation direction matches the
           * configured direction or the configured direction
           * is "all" and the current proximation direction is not "none" */
          if (!strcmp(g_brightnessData.sensordirection, buf) ||
              (!strcmp(g_brightnessData.sensordirection, "all") &&
                strcmp(buf, "none"))
          ) {
            //printf("WAKEUP!\n");
            data->event_received = 1;
            data->event_type = EV_MSC;
          }
          continue;
        }
        //printf("read (pollfds[%d]\n", k);
        if ((rd = read (pollfds[k].fd, ev, size * 64)) < size)
        {
          //data->thread_running = 0;
          //perror_exit ("read()");
          //printf ("listener thread poll read: rd %d errno %d \n", rd, errno);
          usleep(10);
        }
        else
        {
          num = rd / sizeof(struct input_event);
          if (num > 64)
            num = 64;
         
          //STARTPOINT
          //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
          switch (ev[0].type)
          {
            case 1:
              data->event_received = 1;
              data->event_type = ev[0].type;
              if (ev[0].value == 1) //pressed
              {
                if (ev[0].code == KEY_BRIGHTNESSUP)
                {
                  WriteSysFs("/sys/class/leds/bright-plus/", "shot", "1");
                  if (GetClearScreenActivity() != SUCCESS)
                  {
                    //plus key 
                    pthread_mutex_lock(&mutex_offset);
                    data->offset += data->step;
                    pthread_mutex_unlock(&mutex_offset);
                    //printf("%d==== PLUS ====\n", k);
                  }
                }
                else if (ev[0].code == KEY_BRIGHTNESSDOWN)
                {
                  WriteSysFs("/sys/class/leds/bright-minus/", "shot", "1");
                  if (GetClearScreenActivity() != SUCCESS)
                  {
                    //minus key
                    pthread_mutex_lock(&mutex_offset);
                    data->offset -= data->step;
                    pthread_mutex_unlock(&mutex_offset);
                    //printf("%d==== MINUS ====\n", k);
                  }
                }
                else if (ev[0].code == BTN_TOUCH) {
                  /* Ignore Error */
                  WriteSysFs("/sys/class/leds/beeper/", "shot", "1");
                }
                //else if (ev[0].code == KEY_WAKEUP)
                //{
                //   motion sensor si1142 event received                  
                //   screensaverqt receives this event too and close itself
                //}
              }
              
            break;

            case 3:
              /* captouch beeper*/
              data->event_received = 1;
              data->event_type = ev[0].type;
              if (ev[0].code == ABS_MT_TRACKING_ID)
              {
                if( ev[0].value > 0 )
                  WriteSysFs("/sys/class/leds/beeper/", "shot", "1");
              }
            break;
            
            case 4:
              data->event_received = 1;
              data->event_type = ev[0].type;
            break;
            
            default:
              
            break;
          }
        }
      }
    }
    
    if (pollincount == 0)
    {
      updateDevices = 1;
#ifdef DEBUG_MSG      
      printf("poll input - update devices !\n");
#endif      
    }
    
  }  //while
  
  //cleanup
  for (n=0; n < i; n++)
  {
    //close fd
    if (pollfds[n].fd > 0)
    {
      //printf("close pollfds[%d].fd %d\n", n, pollfds[n].fd);
      close(pollfds[n].fd);
      pollfds[n].fd = -1;
    }
  }  
    
  data->thread_running = 0;    
  pthread_exit(0); 
}


/// \brief Thread function cmd
///
/// \param[in]  ptr  pointer to struct tThreadDataCmd
void ThreadFctCmd(void * ptr)
{
  tThreadDataCmd *data;   
  char *lineptr = NULL;
  size_t linelen;
  FILE *fp;     
  int n;      
  char * pStr = NULL;
  const char * pszBacklightOn = "backlighton=";
  
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  
  data = (tThreadDataCmd *) ptr;   
  if (!data)
    pthread_exit(0); 
    
  data->thread_running = 1;

  //create device for incoming commands  
  unlink(DEV_BRIGHTNESS);
  if (mkfifo(DEV_BRIGHTNESS, 0666) >= 0)
  {
    if (chmod(DEV_BRIGHTNESS, 0666) >= 0)
    {
      data->initialized = 1;
    }
  }

  if (data->initialized == 1)
  {
    fp = fopen(DEV_BRIGHTNESS, "r+");
    if (fp)
    {
      while (!data->quit)
      {
        if ((n = getline(&lineptr, &linelen, fp)) < 0)
          continue;
        
        //command received
        pStr = strstr(lineptr, pszBacklightOn);
        if (pStr)
        {
          pStr += strlen(pszBacklightOn);
          pStr = TrimRight(pStr);
          if (anumber(pStr))
          {
            int iVal = MIN(atoi(pStr), 0xFF);
            if (iVal >= 0)
              data->pBrightnessData->backlighton = iVal;
          }
        }
      }
      fclose(fp);
    }
  }
  
  unlink(DEV_BRIGHTNESS);
    
  data->thread_running = 0;  
  pthread_exit(0); 
}




/// \brief Start screensaver thread
/// handles backlight / theme screensaver
/// if an error occurs the function returns -1
///
/// \retval  0 SUCCESS
/// \retval -1 ERROR
int StartScreenSaverThread()
{
  int iRet;
  int state = ERROR;
  iRet = pthread_create (&thread_screensaver, NULL, (void *) &ThreadFctScreensaver, (void *) &threadDataScreenSaver);   
  if (iRet == 0)
  {
    //printf("backlight thread started\n");
    usleep(100);
    state = SUCCESS; 
  }
  return state;
}

/// \brief Start Command listener thread
/// handles backlight 
/// if an error occurs the function returns -1
///
/// \retval  0 SUCCESS
/// \retval -1 ERROR
int StartCmdListenerThread()
{
  int iRet;
  int state = ERROR;
  iRet = pthread_create (&thread_cmd, NULL, (void *) &ThreadFctCmd, (void *) &threadDataCmd);   
  if (iRet == 0)
  {
    //printf("cmd thread started\n");
    usleep(100);
    state = SUCCESS; 
  }
  return state;
}



/// \brief Start event listener thread
/// listen to touchscreen, keyboard, motionsensor events
/// if an error occurs the function returns -1
///
/// \retval  0 SUCCESS
/// \retval -1 ERROR
int StartEventListenerThread()
{
  int iRet;
  int state = ERROR;
  struct sched_param p;
  iRet = pthread_create (&thread_listener, NULL, (void *) &ThreadFctEventListener, (void *) &threadDataEventListener);   
  if (iRet == 0)
  {
    
    p.sched_priority = sched_get_priority_min(SCHED_FIFO);
    //printf("p.sched_priority %d\n", p.sched_priority);
    if (p.sched_priority < 0)
    {
      //failed
      p.sched_priority = 1;
    }
    
    pthread_setschedparam (thread_listener, SCHED_FIFO, &p);
    //printf("listener thread started\n"); 
    usleep(100);
    state = SUCCESS; 
  }
  else
  {
    printf("StartEventListenerThread FAILED\n");  
  }
  return state;
}

/// \brief main function
/// \param[in]  argc number of arguments
/// \param[in]  argv arguments
/// \retval 0  SUCCESS
int main(int argc, char *argv[])
{  
  int iRetValue;  
      
  //Running as daemon 
  // Our process ID and Session ID 
  pid_t pid, sid;
 
  int pid_file = open(PID_FILE, O_CREAT | O_RDWR, 0666);
  int rc = flock(pid_file, LOCK_EX | LOCK_NB);
  if(rc) 
  {
    if(EWOULDBLOCK == errno)
    {
      printf("program is already running\n");
      exit(EXIT_FAILURE);
    }
  }
  else 
  {
    //printf("this is the first instance\n");
  }  
  
  // Fork off the parent process 
  pid = fork();
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }
  // If we got a good PID, then we can exit the parent process. 
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  // Change the file mode mask 
  umask(0);
          
  // Create a new SID for the child process 
  sid = setsid();
  if (sid < 0) {
    exit(EXIT_FAILURE);
  }
   
  // Change the current working directory 
  if ((chdir("/")) < 0) {
    exit(EXIT_FAILURE);
  }
  
  // Close out the standard file descriptors 
  //#ifndef DEBUG_MSG  
  //  close(STDIN_FILENO);
  //  close(STDOUT_FILENO);
  //  close(STDERR_FILENO);
  //#endif  
    
 
  //int iServerId = -1;
    
  //hook in sighandler
  signal(SIGTERM, sig_handler_term); //killall
  signal(SIGINT, sig_handler_int);   //ctrl-c
  
  if ((getuid ()) != 0)
  {
    setRgbLed(RGB_LED_STATE_RE_BLINK, "Brightness control: You are not root! This may not work.");
    SetLastError("brightness_control: you are not root! This may not work.");
  }

  memset(&threadDataEventListener, 0x00, sizeof(threadDataEventListener));
  memset(&threadDataScreenSaver, 0x00, sizeof(threadDataScreenSaver));
  memset(&threadDataCmd, 0x00, sizeof(threadDataCmd));
  
  threadDataCmd.pBrightnessData = &g_brightnessData;
  
  if (InitBrightnessControl() != SUCCESS)
  {    
    char szTxt[128] = "Initialize brightness control failed";
    setRgbLed(RGB_LED_STATE_RE_BLINK, szTxt);
    SetLastError(szTxt);  
    //set defaults
    g_brightnessData.mastermode = AIO_NONE;
    g_brightnessData.daynightswitch = 0;
    g_brightnessData.sensorbrightness = 0;
    g_brightnessData.rangeCounter = 0;
    g_screensaverData.state = 0;
    g_screensaverData.time = 600;
    threadDataScreenSaver.mode = SCREENSAVER_BACKLIGHT; 
    g_brightnessData.backlighton = 255;    
    g_brightnessData.backlightonnight = 255;    
    g_brightnessData.sensormotion = 0;
    g_brightnessData.step = 10;
    g_brightnessData.adcValue0V = 0xFFFF;
    g_brightnessData.adcValue1V = 0xFFFF;
    g_brightnessData.adcValue10V = 0xFFFF;
  }

  threadDataEventListener.step = g_brightnessData.step;
  
  //init screensaver

  iRetValue = StartEventListenerThread();  
  if (iRetValue == SUCCESS)
  {
    if (g_screensaverData.state)
    {     
      iRetValue = StartScreenSaverThread();
      if (iRetValue != SUCCESS)
      {
        setRgbLed(RGB_LED_STATE_RE_BLINK, "Brightness control: error starting screensaver thread."); 
        SetLastError("Brightness control: error starting screensaver thread.");
      }
    }
  }            
  else
  {
    setRgbLed(RGB_LED_STATE_RE_BLINK, "Brightness control: error starting event listener thread.");
    SetLastError("Brightness control: error starting event listener thread.");
  }

  iRetValue = StartCmdListenerThread();
  if (iRetValue != SUCCESS)
  {
    setRgbLed(RGB_LED_STATE_RE_BLINK, "Brightness control: error starting cmd thread."); 
    SetLastError("Brightness control: error starting cmd thread.");
  }
      
  while(1)
  {
    //if screensaver backlight is used do not alter backlight value
    if (!threadDataScreenSaver.running_screensaver)
    {      
      RefreshBacklightSettings();  
    }    
           
    if (threadDataEventListener.event_received)
    {
      if ((threadDataScreenSaver.running_screensaver) && (threadDataEventListener.event_type == 4))
      {
        system("killall -9 screensaverqt > /dev/null 2>&1"); 
        threadDataScreenSaver.running_screensaver = 0; 
      }  
      threadDataEventListener.event_received = 0;            
      time(&g_displayData.tLastEventReceived);
      
      if (g_screensaverData.state)
      {                        
        //reset timeout
        threadDataScreenSaver.event_received = 1;         
      }
    }
    
    CallScreenCare();          
    sleep(1);             
  } 

  CancelThreads();
  usleep(100*1000);
  exit(EXIT_SUCCESS);
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

/// \brief Read Configuration Data, initialize data
/// backlight, screensaver, motion sensor
/// if an error occurs the function returns -1
///
/// \retval  0 SUCCESS
/// \retval -1 ERROR
int InitBrightnessControl()
{
  int ret = 0;
  char szOut[256] = "";
  char * pStr1 = NULL;
  char * pStr2 = NULL;
  int k = 0;
  
  tConfList * pListBacklight = NULL;
  tConfList * pListScreen = NULL;
  tConfList * pListMotion = NULL;
  tConfList * pListDisplay = NULL;
  
  memset (&g_brightnessData, 0x00, sizeof(tBrightnessData));  
  memset (&g_screensaverData, 0x00, sizeof(tScreenData)); 
  memset (&g_displayData, 0x00, sizeof(tDisplayData)); 
  
  time(&g_displayData.tLastEventReceived);
  g_brightnessData.step = 10;
  g_brightnessData.adcLastValidValue = 1000; //10bit
  g_brightnessData.useOffset = 1;
  
  //create list
  pListBacklight = ConfCreateList();
      
  ret = ConfReadValues(pListBacklight, BACKLIGHT_CONF_FILE);
  if (ret != SUCCESS)
  {
    AppendErrorText(FILE_OPEN_ERROR, BACKLIGHT_CONF_FILE);
    ConfDestroyList(pListBacklight);
    return ERROR;
  }
  
  //create list
  pListScreen = ConfCreateList();
  
  ret = ConfReadValues(pListScreen, SCREEN_CONF_FILE);
  if (ret != SUCCESS)
  {
    AppendErrorText(FILE_OPEN_ERROR, SCREEN_CONF_FILE);
    ConfDestroyList(pListScreen);
    ConfDestroyList(pListBacklight);
    return ERROR;
  }
  
  //create list
  pListMotion = ConfCreateList();
  
  ret = ConfReadValues(pListMotion, MOTION_CONF_FILE);
  if (ret != SUCCESS)
  {
    AppendErrorText(FILE_OPEN_ERROR, MOTION_CONF_FILE);
    ConfDestroyList(pListMotion);
    ConfDestroyList(pListScreen);
    ConfDestroyList(pListBacklight);
    return ERROR;
  }
  
  //create list
  pListDisplay = ConfCreateList();
  
  ret = ConfReadValues(pListDisplay, DISPLAY_CONF_FILE);
  if (ret != SUCCESS)
  {
    AppendErrorText(FILE_OPEN_ERROR, DISPLAY_CONF_FILE);
    ConfDestroyList(pListDisplay);
    ConfDestroyList(pListMotion);
    ConfDestroyList(pListScreen);
    ConfDestroyList(pListBacklight);
    return ERROR;
  }
      
  //backlight data
  if (ConfGetValue(pListBacklight, "backlighton", szOut, sizeof(szOut)) == SUCCESS)
  {
    g_brightnessData.backlighton = atoi(szOut);
  }
   
  if (ConfGetValue(pListBacklight, "backlightonnight", szOut, sizeof(szOut)) == SUCCESS)
  {
    g_brightnessData.backlightonnight = atoi(szOut);
  }
    
  if (ConfGetValue(pListBacklight, "backlightoff", szOut, sizeof(szOut)) == SUCCESS)
  {
    g_brightnessData.backlightoff = atoi(szOut);
  }
  
  if (ConfGetValue(pListBacklight, "backlightoffnight", szOut, sizeof(szOut)) == SUCCESS)
  {
    g_brightnessData.backlightoffnight = atoi(szOut);
  }
  
  if (ConfGetValue(pListBacklight, "plusminusstep", szOut, sizeof(szOut)) == SUCCESS)
  {
    g_brightnessData.step = atoi(szOut);
  }
  
  if (ConfGetValue(pListBacklight, "hysteresisdiff", szOut, sizeof(szOut)) == SUCCESS)
  {
    g_brightnessData.sensorHysteresisDiff = atoi(szOut);
  }
  else
  {  
    g_brightnessData.sensorHysteresisDiff = 15;
  }
  
  g_brightnessData.adcHysteresisDiff = 5;
  
  if (ConfGetValue(pListBacklight, "mastermode", szOut, sizeof(szOut)) == SUCCESS)
  {
    if ((stricmp(szOut, "master") == 0))
    {     
      g_brightnessData.mastermode = AIO_MASTER;
    }
    else if ((stricmp(szOut, "slave") == 0))
    {     
      g_brightnessData.mastermode = AIO_SLAVE;
    }
    else 
    {     
      g_brightnessData.mastermode = AIO_NONE;
    }
  }
  
  if (ConfGetValue(pListBacklight, "interface", szOut, sizeof(szOut)) == SUCCESS)
  {
    if ((stricmp(szOut, "ampere") == 0))
    {     
      g_brightnessData.interface = MS_IFACE_AMPERE;
    }
    else 
    {     
      g_brightnessData.interface = MS_IFACE_VOLT;
    }
  }  
  
  if (ConfGetValue(pListBacklight, "daynightswitch", szOut, sizeof(szOut)) == SUCCESS)
  {
    if ((stricmp(szOut, "enabled") == 0))
    {     
      g_brightnessData.daynightswitch = 1;
    }
    else 
    {     
      g_brightnessData.daynightswitch = 0;
    }
  }
  
  if (ConfGetValue(pListBacklight, "usesensor", szOut, sizeof(szOut)) == SUCCESS)
  {
    if ((stricmp(szOut, "yes") == 0))
    {     
      g_brightnessData.sensorbrightness = 1;
    }
    else 
    {     
      g_brightnessData.sensorbrightness = 0;
    }
  }
  
  if (ConfGetValue(pListBacklight, "daytime", szOut, sizeof(szOut)) == SUCCESS)
  {
    pStr1 = &szOut[0];
    pStr2 = strchr(pStr1, ':');
    if (pStr2)
    {
      *pStr2 = '\0';
      //hours
      g_brightnessData.daytime.hour = atoi(pStr1);  
      
      pStr1 = pStr2 + 1;
      if (strlen(pStr1) > 0)
      {
        pStr2 = strchr(pStr1, ':');
        if (pStr2)
        {
          *pStr2 = '\0';
          //minutes
          g_brightnessData.daytime.min = atoi(pStr1);  
          
          pStr1 = pStr2 + 1;
          if (strlen(pStr1) > 0)
          {
            pStr2 = strchr(pStr1, ':');
            if (pStr2)
            {
              *pStr2 = '\0';                              
            }        
            //seconds 
            g_brightnessData.daytime.sec = atoi(pStr1); 
          }
        }
        else
        {
          //minutes
          g_brightnessData.daytime.min = atoi(pStr1);    
        }      
      }
    }            
  }
     
  if (ConfGetValue(pListBacklight, "nighttime", szOut, sizeof(szOut)) == SUCCESS)
  {
    pStr1 = &szOut[0];
    pStr2 = strchr(pStr1, ':');
    if (pStr2)
    {
      *pStr2 = '\0';
      //hours
      g_brightnessData.nighttime.hour = atoi(pStr1);  
      
      pStr1 = pStr2 + 1;
      if (strlen(pStr1) > 0)
      {
        pStr2 = strchr(pStr1, ':');
        if (pStr2)
        {
          *pStr2 = '\0';
          //minutes
          g_brightnessData.nighttime.min = atoi(pStr1);  
          
          pStr1 = pStr2 + 1;
          if (strlen(pStr1) > 0)
          {
            pStr2 = strchr(pStr1, ':');
            if (pStr2)
            {
              *pStr2 = '\0';                              
            }
            //seconds
            g_brightnessData.nighttime.sec = atoi(pStr1);         
          }
        } 
        else
        {
          //minutes
          g_brightnessData.nighttime.min = atoi(pStr1);    
        }     
      }
    }            
  }

  if (ConfGetValue(pListBacklight, "sensorranges", szOut, sizeof(szOut)) == SUCCESS)
  {
    pStr1 = &szOut[0];
    pStr2 = strchr(pStr1, ',');
    k=0;
    while ((pStr2) && (k < MAX_SENSOR_RANGES))
    {
      *pStr2 = '\0';   
      //FROM   
      g_brightnessData.aRanges[k].from = atoi(pStr1); 
      
      pStr1 = pStr2 + 1;
      if (strlen(pStr1) <= 0)
        break;
      pStr2 = strchr(pStr1, ',');
      if (pStr2)   
      {
        *pStr2 = '\0';      
        //TO
        g_brightnessData.aRanges[k].to = atoi(pStr1); 
        
        pStr1 = pStr2 + 1;
        if (strlen(pStr1) <= 0)
          break;
        pStr2 = strchr(pStr1, ',');
        if (pStr2)   
        {
          *pStr2 = '\0';                     
          //VALUE
          g_brightnessData.aRanges[k].val = atoi(pStr1);           
          
          pStr1 = pStr2 + 1;
          if (strlen(pStr1) <= 0)
            break;
          pStr2 = strchr(pStr1, ',');                                                   
          k++;  
        }        
        else
        {
          //VALUE
          g_brightnessData.aRanges[k].val = atoi(pStr1); 
          
          k++;  
          break;
        }
      }
      else
      {
        //triple not complete
        break;
      }                                            
    }        
  }
  
  g_brightnessData.rangeCounter = k;
  
  if (g_brightnessData.rangeCounter > 0)
  {
    g_brightnessData.lastRangeIndex = g_brightnessData.rangeCounter -1;
  }

  //screensaver data
  if (FileExistsWithoutSizeCheck("/var/run/frambuffer.mode") != 0)
  {
    //no qt5 screensaver in framebuffer mode
    if (ConfGetValue(pListScreen, "state", szOut, sizeof(szOut)) == SUCCESS)
    {
      if ((stricmp(szOut, "enabled") == 0))
      {
        g_screensaverData.state = 1;
      }
      else 
      {
        g_screensaverData.state = 0;
      }
    }
  }

  //Xorg
  if (ConfGetValue(pListScreen, "mode", szOut, sizeof(szOut)) == SUCCESS)
  {
    if ((stricmp(szOut, "theme") == 0))
    {          
      threadDataScreenSaver.mode = SCREENSAVER_THEME;
    }
    else 
    { 
      threadDataScreenSaver.mode = SCREENSAVER_BACKLIGHT;          
    }
  }
  
  if (ConfGetValue(pListScreen, "theme", szOut, sizeof(szOut)) == SUCCESS)
  {
    threadDataScreenSaver.theme = atoi(szOut);

  }
  
  if (ConfGetValue(pListScreen, "image", szOut, sizeof(szOut)) == SUCCESS)
  {
    strncpy(threadDataScreenSaver.szImage, szOut, sizeof(threadDataScreenSaver.szImage));

  }
  
  if (ConfGetValue(pListScreen, "text", szOut, sizeof(szOut)) == SUCCESS)
  {
    strncpy(threadDataScreenSaver.szText, szOut, sizeof(threadDataScreenSaver.szText));

  }
  
  if (ConfGetValue(pListScreen, "time", szOut, sizeof(szOut)) == SUCCESS)
  {
    g_screensaverData.time = atoi(szOut);
    if (g_screensaverData.time < MIN_SCREENSAVER_TIME)
      g_screensaverData.time = MIN_SCREENSAVER_TIME;

  }
  
  if (ConfGetValue(pListMotion, "state", szOut, sizeof(szOut)) == SUCCESS)
  {
    if ((stricmp(szOut, "enabled") == 0))
    {     
      g_brightnessData.sensormotion = 1;
      if (ConfGetValue(pListMotion, "direction", szOut, sizeof(szOut)) == SUCCESS)
      {
        if (strlen(szOut) < sizeof(g_brightnessData.sensordirection))
          strcpy(g_brightnessData.sensordirection, szOut);
      }
    }
    else 
    { 
      g_brightnessData.sensormotion = 0;
    }
  }
  
  g_brightnessData.adcValue0V = GetADC_MinValue();
  g_brightnessData.adcValue10V = GetADC_MaxValue();
  
//NEU+++++++++++++++++++++++++            
  //calculate adcValue1V
  g_brightnessData.adcValue1V = g_brightnessData.adcValue0V + lround((g_brightnessData.adcValue10V - g_brightnessData.adcValue0V) / 10.0);
//NEU+++++++++++++++++++++++++            
  
   
#ifdef DEBUG_MSG        
  printf("\nEEPROM ADC 0V: 0x%x EEPROM ADC 10V: 0x%x \n", g_brightnessData.adcValue0V, g_brightnessData.adcValue10V);
  printf("calculated ADC 1V: 0x%x\n", g_brightnessData.adcValue1V);
#endif  
  
 
  // display care mode and time
  if (ConfGetValue(pListDisplay, "care", szOut, sizeof(szOut)) == SUCCESS)
  {
    if ((stricmp(szOut, "enabled") == 0))
    {     
      g_displayData.enabled = 1;
    }
    else 
    {     
      g_displayData.enabled = 0;
    }
  }
  
  if (ConfGetValue(pListDisplay, "caretime", szOut, sizeof(szOut)) == SUCCESS)
  {
    pStr1 = &szOut[0];
    pStr2 = strchr(pStr1, ':');
    if (pStr2)
    {
      *pStr2 = '\0';
      //hours
      g_displayData.caretime.hour = atoi(pStr1);  
      
      pStr1 = pStr2 + 1;
      if (strlen(pStr1) > 0)
      {
        pStr2 = strchr(pStr1, ':');
        if (pStr2)
        {
          *pStr2 = '\0';
          //minutes
          g_displayData.caretime.min = atoi(pStr1);  
          
          pStr1 = pStr2 + 1;
          if (strlen(pStr1) > 0)
          {
            pStr2 = strchr(pStr1, ':');
            if (pStr2)
            {
              *pStr2 = '\0';                              
            }        
            //seconds 
            g_displayData.caretime.sec = atoi(pStr1); 
          }
        }
        else
        {
          //minutes
          g_displayData.caretime.min = atoi(pStr1);    
        }      
      }
    }            
  }
 
  //destroy list
  ConfDestroyList(pListBacklight);
  ConfDestroyList(pListScreen);  
  ConfDestroyList(pListMotion);
  ConfDestroyList(pListDisplay);
  
  return SUCCESS;  
}

/// \brief Set current touchscreen backlight value 
///
/// The function sets the current backlight value,
/// if an error occurs the function returns -1
///
/// \param[in] iValue  new backlight value [0..255]
/// \retval  0 SUCCESS
/// \retval -1 ERROR
int SetBacklightValue(int iValue)
{
  static unsigned char count = 3;
  int state = SUCCESS;
  char szValue[64] = "";           
  int iNewValue;
  
  if (g_brightnessData.useOffset > 0)
  { 
    //use offset +/- keys
    //LOCK
    pthread_mutex_lock(&mutex_offset);
    iNewValue = iValue + threadDataEventListener.offset;  
    if (iNewValue > 255)
    { 
      iNewValue = 255;       
      threadDataEventListener.offset = 255 - iValue;   
    }
    else if (iNewValue < 0)
    {
      iNewValue = 0;
      threadDataEventListener.offset = (iValue * -1) ;
    }
    //UNLOCK
    pthread_mutex_unlock(&mutex_offset);
  }
  else
  {
    //do not user +/- keys offset
    iNewValue = MIN(iValue, 255);
    iNewValue = MAX(iValue, 0);  
  }

  //Resthelligkeit nach der Norm gilt für den Master und den Slave
  if ((g_brightnessData.mastermode == AIO_SLAVE)||(g_brightnessData.mastermode == AIO_MASTER))
  {
     //check LIMIT Benelux Resthelligkeit (minBrightness4Slave)
     iNewValue = MAX(g_brightnessData.minBrightness4Slave, iNewValue);
  }
        
  sprintf(szValue, "%d", iNewValue);   
  
  if (GetBacklightValue() != iNewValue)
  {                  
    /* Enter the critical section */
    pthread_mutex_lock( &cs_sysfs );
         
    if (WriteSysFs(BACKLIGHT_SYSFS_PATH, "brightness", szValue) == SUCCESS)
    {
      time_t tme;
      time(&tme);
      
#ifdef DEBUG_MSG      
      printf("%d set backlight %s \n", tme, szValue);
#endif      
      state = SUCCESS;  
    }
    else
    {
      setRgbLed(RGB_LED_STATE_RE_BLINK, "Brightness control: error writing backlight value");
      state = ERROR;
    }
    
    if (g_brightnessData.mastermode == AIO_MASTER)
    {
      // 1.0 V --> Backlight 0
      //10.0 V --> Backlight 255
      double dy = 25.5 + (iNewValue * 0.9);
      sprintf(szValue, "%ld", lround(dy));   
      
      
      if (g_brightnessData.interface == MS_IFACE_VOLT)
      {
        //set pwm output value / voltage
        if (WriteSysFs(AO_PWM_DRV_PATH, PWM_VALUE, szValue) == SUCCESS)
        {
          //printf("WriteSysFs success value %s \n", szValue);
          state = SUCCESS;          
        }
        else
        {
          setRgbLed(RGB_LED_STATE_RE_BLINK, "Brightness control: error sending backlight value to slave"); 
          state = ERROR;
        }
      }
      else if (g_brightnessData.interface == MS_IFACE_AMPERE)
      {
        //set ampere interface 
        //TODO
      }
    }        
    
    /*Leave the critical section */
    pthread_mutex_unlock( &cs_sysfs );        
  }    
  else
  {    
    if (g_brightnessData.mastermode == AIO_MASTER)
    {
      // 1.0 V --> Backlight 0
      //10.0 V --> Backlight 255
      double dy = 25.5 + (iNewValue * 0.9);
      sprintf(szValue, "%ld", lround(dy));         
      
      //Sicherstellen dass nach dem Gerätehochlauf eine Spannung am AOUT angelegt wird
      if (count)
      {
        count --;
        pthread_mutex_lock( &cs_sysfs );
        
        if (g_brightnessData.interface == MS_IFACE_VOLT)
        {
          //set pwm output value / voltage
          if (WriteSysFs(AO_PWM_DRV_PATH, PWM_VALUE, szValue) == SUCCESS)
          {
            //printf("WriteSysFs success value %s \n", szValue);
            state = SUCCESS;          
          }
          else
          {
            setRgbLed(RGB_LED_STATE_RE_BLINK, "Brightness control: error sending backlight value to slave"); 
            state = ERROR;
          }
        }
        else if (g_brightnessData.interface == MS_IFACE_AMPERE)
        {
          //set ampere
          //TODO
        }
        pthread_mutex_unlock( &cs_sysfs );
      }
    }
  }
  
  return state;
}

/// \brief Read current touchscreen backlight value 
///
/// The function returns the current backlight value,
/// if an error occurs the function returns -1
///
/// \retval    backlight value [0..255]
/// \retval -1 ERROR
int GetBacklightValue()
{
  int state = ERROR;
  char szOut[256] = "";
  char c;
  
  /* Enter the critical section */
  pthread_mutex_lock( &cs_sysfs );
      
  if (ReadSysFs(BACKLIGHT_SYSFS_PATH, "brightness", &szOut[0], sizeof(szOut)) == SUCCESS)
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
    
    if (ConfIsNumber(szOut)==SUCCESS)
    {
      state = atoi(szOut);  
    }  
  }
  else
  {
    setRgbLed(RGB_LED_STATE_RE_BLINK, "Brightness control: error reading backlight value"); 
  }

  /*Leave the critical section */
  pthread_mutex_unlock( &cs_sysfs );

  return state;
}


int cmpfunc (const void * a, const void * b)
{
   return ( *(int*)a - *(int*)b );
}


/// \brief Update the backlight value 
///
/// The function sets the current backlight value,
/// if an error occurs the function returns -1
///
/// \retval  0 SUCCESS
/// \retval -1 ERROR
int RefreshBacklightSettings()
{
  int state = SUCCESS;
  int iSensorValue, k;  
         
  //BACKLIGHT 
  if (g_brightnessData.mastermode == AIO_SLAVE)
  {
    if (g_brightnessData.interface == MS_IFACE_VOLT)
    {
      //SLAVE MODUS
      unsigned short usAdcValue = 0;
      int iValueFromMaster;
    
      //Ist das Gerät kalibriert
      if ((g_brightnessData.adcValue0V >= 0) && (g_brightnessData.adcValue0V < 0xFFFF) && 
          (g_brightnessData.adcValue10V > 100) && (g_brightnessData.adcValue10V < 0xFFFF))
      { 
        int i;
        int aValues[5];
        //average value
        for (i=0; i < 5; i++)
        {
          if (ReadAIn(&usAdcValue) != SUCCESS)
            break;
            
          aValues[i] = usAdcValue;
        }
               
        //Vom Master empfangenen Wert setzen
        if ( i > 4 )
        {
          char szValue[64] = "";
          int iDiff;
          qsort(aValues, 5, sizeof(int), cmpfunc);        
          usAdcValue = (aValues[2] + aValues[3] + aValues[4]) / 3;    
          //printf("ADC Mittelwert %d\n", usAdcValue);
          
          //NEU+++++++++++++++++++++++++          
          //Empfangener Wert 1:1 wieder an den Ausgang legen (ohne Interpretation)
          //dazu gilt ein Bereich von 0 - 10 V
          iValueFromMaster = (usAdcValue - g_brightnessData.adcValue0V) * 255 / (g_brightnessData.adcValue10V - g_brightnessData.adcValue0V);
          sprintf(szValue, "%d", iValueFromMaster);
          //set pwm output value / voltage
          if (WriteSysFs(AO_PWM_DRV_PATH, PWM_VALUE, szValue) != SUCCESS)
          {
            setRgbLed(RGB_LED_STATE_RE_BLINK, "Brightness control: error sending backlight value to slave"); 
            state = ERROR;
          }
          //NEU+++++++++++++++++++++++++          
          
          //Calibration of adcValueMin is  0,0 V
          //Calibration of adcValueMax is 10,0 V
          //Voltage < 0,5 V means not connected or cable break
          if ((usAdcValue < (g_brightnessData.adcValue1V / 2)) || (usAdcValue > 1024))
          {
             // master is not connected, usAdcValue not valid use last valid value
             usAdcValue = g_brightnessData.adcLastValidValue;
             iValueFromMaster = (usAdcValue - g_brightnessData.adcValue1V) * 255 / (g_brightnessData.adcValue10V - g_brightnessData.adcValue1V);
                        
             g_brightnessData.useOffset = 0; //do not use +/- keys
             state = SetBacklightValue(iValueFromMaster);        
          }
          else if (usAdcValue < (g_brightnessData.adcValue1V))
          {
            // master is connected but ADC reads a value lower than 1 V
            // set brightness to lowest value that is allowed (UL or Benelux)
            g_brightnessData.useOffset = 0; //do not use +/- keys
            state = SetBacklightValue(g_brightnessData.minBrightness4Slave);
          }
          else
          {
            char voltageGettingLower = 0;
            //OLD state = SetBacklightValue(usAdcValue >> 2);  // 10 bit value shr 2
            
            //ADC LIMITS                    
            usAdcValue = MIN(usAdcValue, g_brightnessData.adcValue10V);
            usAdcValue = MAX(usAdcValue, g_brightnessData.adcValue1V);          
            
            //Hysterese only if voltage is getting lower
            if (usAdcValue < g_brightnessData.adcLastValidValue)
            {
              //voltage is getting lower  
              voltageGettingLower = 1;
            }                    
            
            iDiff = abs(usAdcValue - g_brightnessData.adcLastValidValue);
            if ((voltageGettingLower == 0) || (iDiff > g_brightnessData.adcHysteresisDiff))
            {
              //value from master using range 1 - 10 V
              g_brightnessData.adcLastValidValue = usAdcValue;
              iValueFromMaster = (usAdcValue - g_brightnessData.adcValue1V) * 255 / (g_brightnessData.adcValue10V - g_brightnessData.adcValue1V);
              //printf("iValueFromMaster %d\n", iValueFromMaster);        
              
              //check LIMITS
              iValueFromMaster = MIN(255, iValueFromMaster);
              iValueFromMaster = MAX(g_brightnessData.minBrightness4Slave, iValueFromMaster);
              state = SetBacklightValue(iValueFromMaster);
            }
          }
        }
        else
        {
          setRgbLed(RGB_LED_STATE_RE_BLINK, "Brightness control: read ADC value failed"); 
          state = ERROR;  
        }    
      }
      else
      {
        //Gerät ist nicht kalibriert
        state = ERROR;  
      }
    }
    else if (g_brightnessData.interface == MS_IFACE_AMPERE)
    {
      //TODO
      //use ampere interface to set remote backlight
    }
    
  }
  else if (g_brightnessData.sensorbrightness)
  {
    //Helligkeitssensor
    state = ERROR;
    
    iSensorValue = GetSensorBrightnessValue();
    printf("GetSensorBrightnessValue(): %d\n", iSensorValue);
    if (iSensorValue >= 0)
    {
      for (k=0; k < g_brightnessData.rangeCounter; k++)
      {
        if ((iSensorValue >= g_brightnessData.aRanges[k].from) && (iSensorValue <= g_brightnessData.aRanges[k].to))
        {                    
          if  (k < g_brightnessData.lastRangeIndex)
          {
            //getting darker immediately
            state = SetBacklightValue(g_brightnessData.aRanges[k].val);
            g_brightnessData.lastRangeIndex = k; 
          }
          else
          {
            //getting brighter using hysteresis
            if (iSensorValue >= (g_brightnessData.aRanges[k].from + g_brightnessData.sensorHysteresisDiff))
            {
              state = SetBacklightValue(g_brightnessData.aRanges[k].val);
              g_brightnessData.lastRangeIndex = k;          
            }            
          }
          
          break;
        }
      }
    }
  }
  else if (g_brightnessData.daynightswitch)
  {
    if (IsNightTime())
    {
      //night-time
      state = SetBacklightValue(g_brightnessData.backlightonnight);  
    }
    else
    {
      //day-time
      state = SetBacklightValue(g_brightnessData.backlighton);  
    }
  }
  else
  {
    //refresh
    state = SetBacklightValue(g_brightnessData.backlighton);  
  }

  return state;  
}

/// \brief Turn backlight off (screensaver backlight)
///
/// Set backlight off value consider day/night settings
///
/// \retval -1 ERROR
/// \retval  0 SUCCESS
int TurnScreensaverBacklightOn()
{
#ifdef DEBUG_MSG  
  printf("TurnScreensaverBacklightOn\n");
#endif  
  
  if (g_brightnessData.daynightswitch)
  {
    if (IsNightTime())
    {
      //night-time
      SetBacklightValue(g_brightnessData.backlightoffnight);  
    }
    else
    {
      //day-time
      SetBacklightValue(g_brightnessData.backlightoff);  
    }
  }
  else
  {
    SetBacklightValue(g_brightnessData.backlightoff);
  }    
  
  return SUCCESS;  
}

/// \brief Check if current time is night or day 
///
/// The function returns TRUE (Night) or FALSE (Day)
///
/// \retval  1 night time
/// \retval  0 day time
int IsNightTime()
{
  time_t tnow;
  struct tm *tmnow;
  int state = TRUE; //night
  
  int iNow, iDay, iNight;

  time(&tnow);
  tmnow = localtime(&tnow);
  
  //printf("Heute ist der ");
  //printf("%d.%d.%d\n",tmnow->tm_mday, tmnow->tm_mon + 1, tmnow->tm_year + 1900);
  
  iNow = (tmnow->tm_hour * 3600) + (tmnow->tm_min * 60) + tmnow->tm_sec;
  iDay = (g_brightnessData.daytime.hour * 3600) + (g_brightnessData.daytime.min * 60) + g_brightnessData.daytime.sec;
  iNight = (g_brightnessData.nighttime.hour * 3600) + (g_brightnessData.nighttime.min * 60) + g_brightnessData.nighttime.sec;
  
  if (iDay < iNight)
  {
    if ((iNow >= iDay) && (iNow < iNight))
    {
      state = FALSE; //day
    }
  }  
  else
  {
    if (iNow >= iDay)
      state = FALSE; //day
  }
    
  return state;
}

/// \brief Read current sensor brightness value 
///
/// The function returns the current sensor value,
/// if an error occurs the function returns -1
///
/// \retval    sensor value [0..255]
/// \retval -1 ERROR
int GetSensorBrightnessValue()
{
	char b[1024], *p;
	ssize_t s;
	struct sockaddr_un addr = {
		.sun_family = AF_UNIX,
		.sun_path = "/var/run/si1142"
	};
	int fd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (fd == -1) {
		perror("socket(AF_UNIX, SOCK_STREAM, 0)");
		return ERROR;
	}
	if (connect(fd, (void*)&addr, sizeof addr) == -1) {
		perror("connect()");
		close(fd);
		return ERROR;
	}
	s = read(fd, b, sizeof b);
	if (s == -1) {
		perror("read(socket)");
		close(fd);
		return ERROR;
	}
	close(fd);

	b[s] = 0;
	p = strchr(b, '=');
	if (!p)
		return ERROR;
	*p++ = 0;
	if (strcmp(b, "intensity"))
		return ERROR;
	return atoi(p);
}

/// \brief CancelThreads
///
/// Cancel threads in case of SIGTERM, SIGINT
/// 
///
void CancelThreads()
{

  if (GetScreensaverActivity())
  {
    system("killall -9 screensaverqt > /dev/null 2>&1");  
    usleep(10*1000);
  }      
  
  if (threadDataEventListener.thread_running)
  {
    pthread_cancel(thread_listener);  
    usleep(10*1000);
  }

  if (threadDataScreenSaver.thread_running)
  {
    pthread_cancel(thread_screensaver);      
    usleep(10*1000);  
  }
  
  if (threadDataCmd.thread_running)
  {
    pthread_cancel(thread_cmd);      
    usleep(10*1000);  
  }
  
  unlink(DEV_BRIGHTNESS);
  remove(PID_FILE);
  RefreshBacklightSettings();
  //printf("CancelThreads done ...\n");
}

int ReplaceLine(tConfList * ptr, char * pNew)
{
  int state = ERROR;
  if (ptr->pStrLeft)
  {
    free(ptr->pStrLeft);
  }

  ptr->pStrLeft = (char*) malloc(strlen(pNew) + 1);
  if (ptr->pStrLeft)
  {
    strcpy(ptr->pStrLeft, pNew);
    state = SUCCESS;
  }
        
  return state;
}

/// \brief Read if screensaver is active at this moment 
///
/// The function returns the current screensaver activity
///
/// \retval  1 screensaver is just active on screen
/// \retval  0 screensaver is not active on screen
///
int GetScreensaverActivity()
{
  int status = 0;  
  if (system("pidof screensaverqt > /dev/null 2>&1") == 0)
  {
    status = 1;  
  }
  return status;
}

/// \brief Read if screen care is active at this moment 
///
/// The function returns the current screen care activity
///
/// \retval  1 screen care is just active on screen
/// \retval  0 screen care is not active on screen
///
int GetScreenCareActivity()
{
  int status = 0;  
  if (system("pidof screen_care > /dev/null 2>&1") == 0)
  {
    status = 1;  
  }
  return status;
}

/// \brief Read analog input value 
///
/// The function reads the AIN value (10 bit value)
/// Shift result 8 bit to the right to get the backlight [0..255]
/// If an error occurs the function returns -1
///
/// \param[out] pAdcVal  pointer to a unsigned short variable
/// \retval  0 SUCCESS
/// \retval -1 ERROR
int ReadAIn(unsigned short * pAdcVal)
{
  int state = ERROR;
  int file;
  int adapter_nr = 2;
  char filename[256] = "";
  
  unsigned char reg_addr = 0x00; /* Device register to access */
  //int res;
  unsigned char buf[10];
  memset (&buf[0], 0x00, sizeof(buf));

  snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);
  //printf("open device %s \n", filename);
  file = open(filename, O_RDWR);
  if (file < 0)
  {
    /* ERROR HANDLING; you can check errno to see what went wrong */
#ifdef DEBUG_MSG    
    printf("open device failed \n");
#endif    
    return ERROR;
  }
  //printf("open device successfull \n");

  int addr = 0x51; /* The I2C address -> A0 on ground */

  if (ioctl(file, I2C_SLAVE, addr) < 0)
  {
    /* ERROR HANDLING; you can check errno to see what went wrong */
#ifdef DEBUG_MSG    
    printf("ioctl failed \n");
#endif    
    close(file);
    return ERROR;
  }
  //printf("ioctl success \n");

  reg_addr = 0x00;
  buf[0] = reg_addr;
  buf[1] = 0x00;
  buf[2] = 0x00;  
  /* Using I2C Read, equivalent of i2c_smbus_read_byte(file) */
  if (read(file, buf, 2) != 2)
  {
      /* ERROR HANDLING: i2c transaction failed */
#ifdef DEBUG_MSG      
      printf("i2c bus read failed \n");
#endif      
      close(file);
      return ERROR;
  }
  else
  {
    //10 bit value
    *pAdcVal = (((unsigned short)buf[0]) << 6) | ((((unsigned short)buf[1]) & 0xFC)  >> 2);
    //printf("ADC: %0.2x %0.2x %0.2x \n", buf[0], buf[1], buf[2]);
#ifdef DEBUG_MSG          
    //printf("10 bit ADC value= %d \n", *pAdcVal);
#endif    

  }
  
  close(file);
  state = SUCCESS;
  return state;
}


/// \brief call screen care program if configured and time is reached 
///
void CallScreenCare()
{
  if (g_displayData.enabled)
  {
    time_t tnow;
    struct tm *tmnow;
    
    int iNow, iCare, diff;

    time(&tnow);
    tmnow = localtime(&tnow);
        
    iNow  = (tmnow->tm_hour * 3600) + (tmnow->tm_min * 60) + tmnow->tm_sec;
    iCare = (g_displayData.caretime.hour * 3600) + (g_displayData.caretime.min * 60) + g_displayData.caretime.sec;
    
    diff = iNow - iCare;
    
    if ((diff >= 0) && (diff < 5))
    {      
      time(&tnow);
           
      if (!g_displayData.called)
      {  
        //do not start display care if someone is using the panel
        //if ((tnow - g_displayData.tLastEventReceived) > 60)
        //{
          //start if not already running and no screensaver is active
          if ((GetScreensaverActivity() == 0) && (GetScreenCareActivity() == 0))
          {
            g_displayData.called = 1;
            system("/usr/bin/screen_care");
          }
        //}        
      }
    }
    else
    {
      g_displayData.called = 0;
    }  
    
  }
}


/// \brief Read if clear_screen is active at this moment 
///
/// The function returns the current clear_screen activity
///
/// \retval  0 clear_screen is running 
/// \retval -1 clear_screen is not running 
int GetClearScreenActivity()
{
  int status = ERROR;  
  if (system("/bin/pidof clear_screen > /dev/null 2>&1") == 0)
  {
    status = SUCCESS;  
  }
  return status;
}

/// \brief Read usb keyboard input device name  
///
/// f.e. event5 (/dev/input/event5)
///
/// \retval  0 SUCCESS 
/// \retval -1 ERROR
int GetUsbKeyboardDeviceName(char * pInputDeviceName, int maxlen)
{
  //grep -E 'Handlers|EV=' /proc/bus/input/devices | grep -B1 'EV=120013' | grep -Eo 'event[0-9]+'
  
  char buffer[512];
  char * pLine = NULL;
  int iRet = -1;
  char szCmd[128] = "grep -E 'Handlers|EV=' /proc/bus/input/devices | grep -B1 'EV=120013' | grep -Eo 'event[0-9]+'";
  
  FILE * pFile = popen(szCmd, "r");

  if (!pFile)
  {
    return -1;
  }  
  pLine = fgets(buffer, sizeof(buffer), pFile);
  
  if (pLine)
  {
    TrimRight(pLine);
    //printf("pLineX1: %s\n", pLine);
    
    if (strlen(pLine) > 0)
    {
      iRet = 0;
      strncpy(pInputDeviceName, pLine, maxlen);
    }
    else
    {
      iRet = -1;
    }
  }
  
  pclose(pFile);
  
  return iRet;
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


/// \brief read AIN 0V ADC value from EEPROM
///
/// \retval -1 ERROR
/// \retval Min ADC value 
int GetADC_MinValue()
{
  int ret = ERROR;
  int i2c_fd;
  char cTempBuf[6];
  
  char szErrorTxt[256] = "";  
  
  i2c_fd = eeprom_open();
  if (i2c_fd >= 0)
  {              
    if (ReadBlock(i2c_fd, (unsigned int) ADC_MIN_ADDR, cTempBuf, 2) >= 0) 
    {
      ret = ((cTempBuf[1] << 8) | (cTempBuf[0]));  
    }
    else
    {
      sprintf(szErrorTxt, "Could not read EEPROM (ADC value)");
      setRgbLed(RGB_LED_STATE_RE_BLINK, szErrorTxt);
      SetLastError(szErrorTxt);
    }
    close(i2c_fd);    
  }
  else
  {
    sprintf(szErrorTxt, "Could not open EEPROM (ADC value)");
    setRgbLed(RGB_LED_STATE_RE_BLINK, szErrorTxt);  
    SetLastError(szErrorTxt);    
  }
    
  return ret;
}

/// \brief read AIN 10V ADC value from EEPROM
///
/// \retval -1 ERROR
/// \retval Max ADC value 10V
int GetADC_MaxValue()
{
  int ret = ERROR;
  int i2c_fd;
  char cTempBuf[6];
  
  char szErrorTxt[256] = "";  
  
  i2c_fd = eeprom_open();
  if (i2c_fd >= 0)
  {              
    if (ReadBlock(i2c_fd, (unsigned int) ADC_MAX_ADDR, cTempBuf, 2) >= 0) 
    {
      ret = ((cTempBuf[1] << 8) | (cTempBuf[0]));  
    }
    else
    {
      sprintf(szErrorTxt, "Could not read EEPROM (ADC value)");
      setRgbLed(RGB_LED_STATE_RE_BLINK, szErrorTxt);
      SetLastError(szErrorTxt);
    }
    close(i2c_fd);    
  }
  else
  {
    sprintf(szErrorTxt, "Could not open EEPROM (ADC value)");
    setRgbLed(RGB_LED_STATE_RE_BLINK, szErrorTxt);  
    SetLastError(szErrorTxt);    
  }
    
  return ret;
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

int ReadScreenSize()
{
  struct fb_var_screeninfo var;
  int fb_fd=0;
  
  char *defaultfbdevice = "/dev/fb0";
  char *fbdevice = NULL;

  if ((fbdevice = getenv ("TSLIB_FBDEVICE")) == NULL)
    fbdevice = defaultfbdevice;
  
  fb_fd = open(fbdevice, O_RDWR);
  if (fb_fd == -1) {
    perror("open fbdevice");
    return -1;
  }
  
  if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &var) < 0) {
    perror("ioctl FBIOGET_VSCREENINFO");
    close(fb_fd);
    return -1;
  }

  close(fb_fd);
  
  return SUCCESS;
}

