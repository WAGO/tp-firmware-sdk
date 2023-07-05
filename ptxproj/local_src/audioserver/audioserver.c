//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019-2022 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
///
/// \file    audioserver.c
///
/// \version $Id$
///
/// \brief   description
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <ctype.h>
#include <dirent.h>
#include <spawn.h>
#include <wait.h>


//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
//#define _GNU_SOURCE

//#define DEBUG_MSG

#define PID_FILE              "/var/run/audioserver.pid"
#define CMD_DEVFILE           "/dev/audioserver"

//------------------------------------------------------------------------------
// Local macros
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External variables
//------------------------------------------------------------------------------
extern char **environ; //spawn

//------------------------------------------------------------------------------
// Local typedefs
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local variables
//------------------------------------------------------------------------------
static char g_loop = 1;

//------------------------------------------------------------------------------
// external functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local functions
//------------------------------------------------------------------------------


/// \brief signal handler SIGTERM
///
/// \param[in]  foo  number
static inline void sig_handler_term(int foo)
{
  //cancel threads before terminate
  g_loop = 0;

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
  g_loop = 0;

#ifdef DEBUG_MSG
  printf("exit: sig_handler ctrl-c \n");
#endif  
  usleep(100*1000);
  exit(1);
}

char *ltrim(char *s)
{
    while(isspace(*s)) s++;
    return s;
}

char *rtrim(char *s)
{
  char* back = s + strlen(s);
  while(isspace(*--back));
  *(back+1) = '\0';
  return s;
}

char *trim(char *s)
{
  return rtrim(ltrim(s)); 
}

void msdelay(int ms)
{
  // ms
  usleep(ms * 1000);
}

static int SystemCall(char * pCmd, char * argv[])
{
  int iRet = 0;
  pid_t pid;
  int status;
  
#ifdef DEBUG_MSG
    printf("SystemCall BEGIN: %s\n", pCmd);
#endif
  
  status = posix_spawn(&pid, pCmd, NULL, NULL, argv, environ);
  if (status == 0)
  {
    //waiting is neccessary using spawn
    if (waitpid(pid, &status, 0) != -1)
    {
      iRet = WEXITSTATUS(status);
      //iRet = 0;
    }
    else
    {
      iRet = -1;
    }
  }
  else
  {
    iRet = -1;
  }
  
#ifdef DEBUG_MSG
    printf("SystemCall END: %s\n", pCmd);
#endif  

  return iRet;
}


static int FindProcess(const char * pszName)
{
   int iRet = 0;
   const char* directory = "/proc";
   size_t      taskNameSize = 1024;
   char*       taskName = (char*) calloc(1, taskNameSize);

   DIR* dir = opendir(directory);

   if (dir)
   {
      struct dirent* de = 0;

      while ((de = readdir(dir)) != 0)
      {
         if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;

         int pid = -1;
         int res = sscanf(de->d_name, "%d", &pid);

         if (res == 1)
         {
            // we have a valid pid

            // open the cmdline file to determine what's the name of the process running
            char cmdline_file[1024] = {0};
            sprintf(cmdline_file, "%s/%d/cmdline", directory, pid);

            FILE* cmdline = fopen(cmdline_file, "r");
            if (cmdline)
            {
              if (getline(&taskName, &taskNameSize, cmdline) > 0)
              {
                // is it the process we care about?
                if (strstr(taskName, pszName) != 0)
                {
                  //fprintf(stdout, "A %s process, with PID %d, has been detected.\n", pszName, pid);
                  //found
                  iRet = 1;
                }
              }
              
              fclose(cmdline);
            }
         }
         if (iRet)
           break;
      }

      closedir(dir);
   }

   free(taskName);

   return iRet;
}

void killAplay()
{
  //char *argv[] = { "killall", "aplay", (char *) 0 };
  //char *pCmd = "/usr/bin/killall";
  //SystemCall(pCmd, argv);

  system("/usr/bin/killall aplay 2>&1 > /dev/null");

}

int stop()
{
  int iSuccess = 1;
  if (FindProcess("/usr/bin/aplay") == 1)
  {
    //stop playing
    killAplay();
    msdelay(2);
    if (FindProcess("/usr/bin/aplay") == 1)
      iSuccess = 0;
  }
  
  return iSuccess;
}

int play(char * pszFile)
{
  int iSuccess = 0;
  int iCnt = 0;
  
  struct stat sts;
  if (stat(pszFile, &sts) == -1 && errno == ENOENT)
  {
    //file does NOT exists
#ifdef DEBUG_MSG
    printf("file not found: %s\n", pszFile);
#endif
    return iSuccess;
  }
  
  while (FindProcess("/usr/bin/aplay") == 1)
  {
    //try stopping several times
#ifdef DEBUG_MSG
    printf("aplay busy\n");
#endif            
    killAplay();
    msdelay(2);
    iCnt++;
    if (iCnt > 10)
      break;
  }
  
  if (FindProcess("/usr/bin/aplay") == 0)
  {
    //play in background - only one aplay instance allowed
    char szCmd[312] = "";
    sprintf(szCmd, "/usr/bin/aplay %s 2>&1 > /dev/null &", pszFile);
    system(szCmd);
    iSuccess = 1;

    //char *argv[] = { "aplay", pszFile, "2>&1 > /dev/null &", (char *) 0 };
    //char *pCmd = "/usr/bin/aplay";
    //if (SystemCall(pCmd, argv) == 0)
    //  iSuccess = 1;

    msdelay(1);
  }
  
  return iSuccess;
}

static void readFromPipedFifoLoop()
{
  char *lineptr = NULL;
  size_t linelen;
  FILE *fp;
  int n;
  char szCmd[312] = "";
  char szFile[256] = "";
  //char * homeDir = "/home/codesys_root/PlcLogic/audio/";

  fp = fopen(CMD_DEVFILE, "r+");
  if (fp)
  {
    //int fd = fileno(fp);
    
    while (g_loop > 0)
    {
      if ((n = getline(&lineptr, &linelen, fp)) < 0)
        continue;
#ifdef DEBUG_MSG
      printf("CMD: %s\n", lineptr);
      //printf("rw_hint: %ld \n", (long)fcntl( fd, F_GET_FILE_RW_HINT  ));
#endif      
      if (strncasecmp(lineptr, "close", 5) == 0) {
        stop();
        g_loop = 0;
      }
      else if (strncasecmp(lineptr, "play=", 5) == 0)
      {
        char * pFile = lineptr;
        pFile += 5;
        strncpy(szFile, trim(pFile), sizeof(szFile));
#ifdef DEBUG_MSG        
        printf("set filename to %s\n", szFile);
#endif        
        play(&szFile[0]);
      }
      else if (strncasecmp(lineptr, "volume=", 7) == 0)
      {
        char * pstr = lineptr;
        pstr += 7;
#ifdef DEBUG_MSG        
        printf("set volume to %s\n", trim(pstr));
#endif        
        if (FindProcess("/usr/bin/amixer") != 1)
        {
          sprintf(szCmd, "/usr/bin/amixer -c 0 sset Headphone,0 %s 2>&1 > /dev/null", trim(pstr));
          system(szCmd);
        }
        else
        {
          printf("amixer busy\n");
        }
      }
      else if (strncasecmp(lineptr, "stop", 4) == 0)
      {
        stop();
      }

    }
    if (lineptr)
      free(lineptr);
    fclose(fp);
  }

}


int main(int argc, char *argv[])
{
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
  
   //hook in sighandler
  signal(SIGTERM, sig_handler_term); //killall
  signal(SIGINT, sig_handler_int);   //ctrl-c
  
  //unlink former fifo
  unlink(CMD_DEVFILE);
  
  //create pipe fifo
  if (mkfifo(CMD_DEVFILE, 0664) >= 0)
  {
    if (chmod(CMD_DEVFILE, 0664) >= 0)
    {
      //bInitialized = true;
    }
  }
  
  //loop
  readFromPipedFifoLoop();
  
  unlink(CMD_DEVFILE);
  exit(EXIT_SUCCESS);
}
