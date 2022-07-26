//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019-2022 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
///
/// \file    tcp_monitor.c
///
/// \version $Id: tcp_monitor.c 65689 2022-03-11 14:37:43Z falk.werner@wago.com $
///
/// \brief   tcp packet listener to monitor plclist URLs
///
/// \author  Wolfgang Rückl, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pcap.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/ip_icmp.h>   //Provides declarations for icmp header
#include <netinet/udp.h>       //Provides declarations for udp header
#include <netinet/tcp.h>       //Provides declarations for tcp header
#include <netinet/ip.h>        //Provides declarations for ip header

#include <netdb.h> //hostent
#include <ctype.h>
#include <time.h>  
#include <signal.h>

#include "msgtool.h"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

#define MAX_ITEMS              10                         ///< max count of saved IPs
#define FILE_TMP_BROWSER_URL   "/tmp/webenginebrowser_url.txt"
#define STATE_TIMER_INTERVAL    2                         ///< timer interval [s]

typedef struct stMonitorData
{
  struct timespec timeRx;           ///< timestamp rx packet received
  struct in_addr srcAddr;           ///< source address tcp packet coming from
} tMonitorData;

typedef struct stTrcDevice
{
  char szDev[64];
  int found;
  int index;
} tTrcDevices;

static tTrcDevices aTrcDevices[] =
{
  { "cmdlinedevice", 0, 0 },
  { "br0", 0, 0 },
  { "br1", 0, 0 },
  { "eth0", 0, 0 },

  // this line must always be the last one - don't remove it!
  { "", 0, 0 }
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
 
void process_packet(u_char *, const struct pcap_pkthdr *, const u_char *);
void process_ip_packet(const u_char * , int);
void print_ip_packet(const u_char * , int);
void print_tcp_packet(const u_char *  , int );

void Insert2MonitorData(struct in_addr);
int SendData2Webbrowser(char * pszIp);
int Hostname2Ip(char * hostname , char* ip);
void sig_handler_timer(int sig);

//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variable definitions
//------------------------------------------------------------------------------

struct sockaddr_in source,dest;
int tcp=0,udp=0,icmp=0,others=0,igmp=0,total=0,i,j;
uint32_t GetLocalIpAddr();
char szLocalAddr[64];             ///< ip address eth0, br0 
struct in_addr localAddr;  
char *devname = NULL; 

tMonitorData aMonitorData[MAX_ITEMS];

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
 
/// \brief get the current local ip address
///
/// \retval internet ip address 
uint32_t GetLocalIpAddr()
{
 int fd;
 struct ifreq ifr;

 fd = socket(AF_INET, SOCK_DGRAM, 0);
 ifr.ifr_addr.sa_family = AF_INET;
 strncpy(ifr.ifr_name, devname, IFNAMSIZ-1);
 ioctl(fd, SIOCGIFADDR, &ifr);
 close(fd);

 localAddr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr; 
 strncpy(szLocalAddr, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), sizeof(szLocalAddr)); 

 return ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
}

 
/// \brief main function
/// \param[in]  argc number of arguments
/// \param[in]  argv arguments
/// \retval 0  SUCCESS
int main(int argc, char *argv[])
{
    pcap_if_t *alldevsp , *device;
    pcap_t *handle;
    
    //filter
    bpf_u_int32 mask;           // netmask
    bpf_u_int32 net;            // IP 
    struct bpf_program fp;      // compiled filter
    char filter_exp[] = "tcp";  // filter expression
 
    char errbuf[PCAP_ERRBUF_SIZE], devs[100][100];
    int count = 1;
    int n = -1;
    int k = 0;
    
    if (argc > 1)
    {
      strncpy(aTrcDevices[0].szDev, argv[1], sizeof(aTrcDevices[0].szDev));      
    }
    
    //init monitor data
    memset (&aMonitorData[0], 0x00, sizeof(aMonitorData));
    
    //alarm, timer
    signal(SIGALRM, sig_handler_timer); 
     
    //First get the list of available devices
    if( pcap_findalldevs( &alldevsp , errbuf) )
    {
        printf("Error finding devices : %s" , errbuf);
        exit(1);
    }
     
    for(device = alldevsp ; device != NULL ; device = device->next)
    {      
      if(device->name != NULL)
      {
        strcpy(devs[count] , device->name);
        k = 0;
        while (aTrcDevices[k].szDev[0])
        {
          if (strcmp(device->name, aTrcDevices[k].szDev) == 0)
          {
            //device found in list
            aTrcDevices[k].found = 1;
            aTrcDevices[k].index = count;
            //printf("device found %s - %d\n", device->name, count);
            break;
          }
          k++;
        }
      }
      count++;
    }
    
    k = 0;
    while (aTrcDevices[k].szDev[0])
    {
      if (aTrcDevices[k].found > 0)
      {
        n = aTrcDevices[k].index;
        break;
      }      
      k++;
    }
    
    if (n < 0)
    {
      printf("Error no device found\n");
      exit(1);
    }
            
    devname = devs[n];
    printf("devname: %s\n", devname);
    
    if (pcap_lookupnet(devname, &net, &mask, errbuf) == -1) {
      fprintf(stderr, "Couldn't get netmask for device %s: %s\n", devname, errbuf);
      net = 0;
      mask = 0;
    }
     
    GetLocalIpAddr();
    //DHCP but no IP set
    while (strcmp(szLocalAddr, "0.0.0.0")==0)
    {
      //wait until ip address is available
      sleep(5);
      GetLocalIpAddr();
    } 
     
    //Open the device for sniffing
    //printf("Opening device %s for sniffing ... " , devname);
    //handle = pcap_open_live(devname , 65536 , 1 , 0 , errbuf);
    handle = pcap_open_live(devname , 2048 , 1 , 0 , errbuf);
     
    if (handle == NULL)
    {
        fprintf(stderr, "Couldn't open device %s : %s\n" , devname , errbuf);
        exit(1);
    }
    
    // Compile and apply the filter
    if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
      fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
      return 2;
    }
    if (pcap_setfilter(handle, &fp) == -1) {
      fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
      return 2;
    }
    
    //call timer first time
    alarm(STATE_TIMER_INTERVAL); 
     
    //Put the device in sniff loop
    pcap_loop(handle , -1 , process_packet , NULL);
     
    return 0;  
}
 
void process_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *buffer)
{
    int size = header->len;
     
    //Get the IP Header part of this packet , excluding the ethernet header
    struct iphdr *iph = (struct iphdr*)(buffer + sizeof(struct ethhdr));
    ++total;
    switch (iph->protocol) //Check the Protocol and do accordingly...
    {
        //case 1:  //ICMP Protocol
        //    ++icmp;
        //    print_icmp_packet( buffer , size);
        //    break;
         
        //case 2:  //IGMP Protocol
        //    ++igmp;
        //    break;
         
        case 6:  //TCP Protocol
            ++tcp;
            print_tcp_packet(buffer , size);
            break;
         
        //case 17: //UDP Protocol
        //    ++udp;
        //    print_udp_packet(buffer , size);
        //    break;
         
        //default: //Some Other Protocol like ARP etc.
        //    ++others;
        //    break;
    }
}
 

void print_ip_header(const u_char * Buffer, int Size)
{
    unsigned short iphdrlen;
         
    struct iphdr *iph = (struct iphdr *)(Buffer  + sizeof(struct ethhdr) );
    iphdrlen =iph->ihl*4;
     
    memset(&source, 0, sizeof(source));
    source.sin_addr.s_addr = iph->saddr;
     
    memset(&dest, 0, sizeof(dest));
    dest.sin_addr.s_addr = iph->daddr;

    if (localAddr.s_addr == dest.sin_addr.s_addr)
    {
      Insert2MonitorData(source.sin_addr);
    }
    
}


void Insert2MonitorData(struct in_addr in)
{
  static int index = 0;
  int i = 0;
  while (i < MAX_ITEMS)
  {
    if (aMonitorData[i].srcAddr.s_addr == 0)
    {
      //new entry
      aMonitorData[i].srcAddr.s_addr = in.s_addr;
      clock_gettime(CLOCK_MONOTONIC, &aMonitorData[i].timeRx);
      index = i;
      break;
    }
    else if (in.s_addr == aMonitorData[i].srcAddr.s_addr)
    {
      //found, update timestamp
      clock_gettime(CLOCK_MONOTONIC, &aMonitorData[i].timeRx);
      break;
    }
    i++;
  }
  
  if (i >= MAX_ITEMS)
  {
    //list is full
    index++;
    if (index >= MAX_ITEMS)
      index = 0;
      
    //overwrite current position  
    aMonitorData[index].srcAddr.s_addr = in.s_addr;
    clock_gettime(CLOCK_MONOTONIC, &aMonitorData[index].timeRx);
  }

}
 
void print_tcp_packet(const u_char * Buffer, int Size)
{
    unsigned short iphdrlen;
     
    struct iphdr *iph = (struct iphdr *)( Buffer  + sizeof(struct ethhdr) );
    iphdrlen = iph->ihl*4;
     
    struct tcphdr *tcph=(struct tcphdr*)(Buffer + iphdrlen + sizeof(struct ethhdr));
             
    int header_size =  sizeof(struct ethhdr) + iphdrlen + tcph->doff*4;
     
    print_ip_header(Buffer,Size);

}
 
int SendData2Webbrowser(char * pszIp)
{
  int i;
  int ret = -1;
  int iSeconds;
  struct in_addr search;
  char szCmd[128] = "";  
  char * pIp = pszIp;
  char szNumericIp[100] = "";
  if (isdigit(*pszIp) == 0)
  {
    //not a number
    if (Hostname2Ip(pszIp, szNumericIp) == 0)
    {
      //success
      pIp = szNumericIp;
    }
    else
    {
      //printf("Hostname2Ip %s failed %s \n", pszIp, szNumericIp);
      return ret;
    }
  }
  
  if (inet_aton(pIp, &search) == 1)
  {  
    for (i=0; i < MAX_ITEMS; i++)
    {
      if (aMonitorData[i].srcAddr.s_addr == search.s_addr)
      {
        //clock_gettime(CLOCK_MONOTONIC,&timeNow);   
        //iSeconds = (int)timeNow.tv_sec - (int)aMonitorData[i].timeRx.tv_sec;
        iSeconds = (int)aMonitorData[i].timeRx.tv_sec;    
        sprintf(szCmd, "tcpmonitor%s=%d\n", pszIp, iSeconds);
        Write2PipedFifo(DEV_WEBENGINEBROWSER, szCmd);
        
        break;
      }
    }
    ret = 0;
  }
  
  return ret;
}

int Hostname2Ip(char * hostname , char* ip)
{
  struct hostent *he;
  struct in_addr **addr_list;
  int i;
       
  if ( (he = gethostbyname( hostname ) ) == NULL) 
  {
    // get the host info
    herror("gethostbyname");
    return 1;
  }

  addr_list = (struct in_addr **) he->h_addr_list;
   
  for(i = 0; addr_list[i] != NULL; i++) 
  {
    //Return the first one;
    strcpy(ip , inet_ntoa(*addr_list[i]) );    
    return 0;
  }
   
  return 1;
}
 
/// \brief signal handler SIGALRM
///
/// use SIGALRM as a timer
/// \param[in]  sig  number
void sig_handler_timer(int sig)
{
  //printf("timer called \n");
  
  if (FileExistsWithoutSizeCheck(FILE_TMP_BROWSER_URL) == 0)
  {
    //success
    char szLine[256] = "";
    FILE * fp;
    fp = fopen(FILE_TMP_BROWSER_URL, "r+");
    if (fp)
    {
      if (fgets(szLine, sizeof(szLine), fp) != NULL)
      {
        TrimRight(szLine);
        SendData2Webbrowser(szLine);
      }
      fclose(fp);
    }
  }
  
  //call next
  alarm(STATE_TIMER_INTERVAL); 
}
