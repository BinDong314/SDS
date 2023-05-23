/**
 * *** Copyright Notice ***
 * SDS - Scientific Data Services framework, Copyright (c) 2015, The Regents of the University of California, through Lawrence Berkeley National Laboratory (subject to receipt of any required approvals from the U.S. Dept. of Energy).  All rights reserved.
 * If you have questions about your rights to use or distribute this software, 
 * please contact Berkeley Lab's Technology Transfer Department at TTD@lbl.gov.
 * 
 * NOTICE.  This software was developed under funding from the 
 * U.S. Department of Energy.  As such, the U.S. Government has been granted 
 * for itself and others acting on its behalf a paid-up, nonexclusive, 
 * irrevocable, worldwide license in the Software to reproduce, prepare 
 * derivative works, and perform publicly and display publicly.  
 * Beginning five (5) years after the date permission to assert copyright is 
 * obtained from the U.S. Department of Energy, and subject to any subsequent 
 * five (5) year renewals, the U.S. Government is granted for itself and others
 * acting on its behalf a paid-up, nonexclusive, irrevocable, worldwide license
 * in the Software to reproduce, prepare derivative works, distribute copies to
 * the public, perform publicly and display publicly, and to permit others to
 * do so.
 *
*/

/**
 *
 * Email questions to {dbin, sbyna, kwu}@lbl.gov
 * Scientific Data Management Research Group
 * Lawrence Berkeley National Laboratory
 *
*/
/*
 * This file defines the interface to analyze the configure files for sds server
 * Also, it defines the "extern global variables" used by server. 
 * Author: Bin Dong <dbin at lbl.gov >
 * Copyright 2015 the Regents of the University of California
 * 
 */

#include "sds-config-server.h"

extern int         query_threads_num;
extern char        sds_root_path[MAX_FILE_NAME_LENGTH];
extern int         monitor_interval;
extern SDS_Bool    cluster_version;
extern char        run_command[MAX_FILE_NAME_LENGTH];


int parse_server_file(char * ini_name)
{
    dictionary   *ini;
    char        *ts;
    
    /* Some temporary variables to hold query results */
    ini = iniparser_load(ini_name);
    if (ini==NULL) {
        fprintf(stderr, "cannot parse file: %s\n", ini_name);
        return -1 ;
    }
    
    query_threads_num = iniparser_getint(ini,   "server:query_threads_num", -1);
    ts                = iniparser_getstring(ini,    "common:sds_root_path", -1);
    if(ts != NULL){
      strcpy(sds_root_path, ts);
    }
    monitor_interval  = iniparser_getint(ini,    "server:monitor_interval", -1);
    cluster_version   = iniparser_getboolean(ini, "common:cluster_version", -1); 
    
    ts                = iniparser_getstring(ini,      "server:run_command", -1);
    if(ts != NULL){
      strcpy(run_command, ts);
    }
    
    int update_ip_flag = iniparser_getint(ini,    "server:update_server_ip_flag", -1);
    if (update_ip_flag == 1){
      char server_interface[244];
      ts = iniparser_getstring(ini,      "server:run_command", -1);
      if(ts != NULL){
        strcpy(server_interface, ts);
      }
      char ip_address[244];

      get_server_ip(ip_address, server_interface);

      if (iniparser_set(ini,      "common:server_ip", ip_address ) == 0){
        printf("Update server_ip in configure file. \n");
        
      }
  
    }
    

    iniparser_freedict(ini);
    return 0 ;
}



int get_server_ip(char *ifr, char *inet_address){
  int fd;
  struct ifreq ifr;
  
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  /* I want to get an IPv4 IP address */
  ifr.ifr_addr.sa_family = AF_INET;
  /* I want IP address attached to "eth0" */
  strncpy(ifr.ifr_name, ifr, IFNAMSIZ-1);
  ioctl(fd, SIOCGIFADDR, &ifr);
  close(fd);
  strcpy(inet_address, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
  /* display result */
  printf("Detected server IP for interface [%s] is %s\n", ifr, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}
