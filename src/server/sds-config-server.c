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



int parse_server_file(char * ini_name)
{
    dictionary   *ini;
    char        *ts;
    int          ti;
    printf("Configurations of SDS Server: \n ");
    /* Some temporary variables to hold query results */
    ini = iniparser_load(ini_name);
    if (ini==NULL) {
      fprintf(stderr, "cannot parse file: %s (Use default configurations)\n", ini_name);
      return -1 ;
    }
    
    ti = iniparser_getint(ini,   "server:query_threads_num", -1);
    if(ti != -1){
      query_threads_num = ti;
    }
    
    printf("  # of query answering threads [%d] \n", query_threads_num);
    
    ts                = iniparser_getstring(ini,    "common:sds_root_path", NULL);
    if(ts != NULL){
      strcpy(sds_root_path, ts);
    }
    printf("   Derectory to store SDS Augmented files: [%s] \n", sds_root_path);
    
    ti = iniparser_getint(ini,    "server:monitor_interval", -1);
    if(ti != -1)
      monitor_interval = ti;
    printf("   Server monitor interval: [%d] seconds \n", monitor_interval);

    ti = iniparser_getint(ini,    "common:server_port", -1);
    if(ti != -1)
      server_port = ti;
    printf("   Server listen port : [%d] \n", server_port);
    
    ti = iniparser_getboolean(ini, "common:cluster_version", -1); 
    if(ti != -1)
      cluster_version = ti;
    if(cluster_version){
      printf("   This is cluster version: [TRUE] \n");
    }else{
      printf("   This is cluster version: [FALSE] \n");
    }
    
    ts                = iniparser_getstring(ini,      "server:run_command", NULL);
    if(ts != NULL){
      strcpy(run_command, ts);
    }
    
    printf("   Command to run SDS Services [%s] \n", run_command);

    char ip_address[100];
    int update_ip_flag = iniparser_getboolean(ini,    "common:update_server_ip_flag", -1);
    if (update_ip_flag == 1){
      char server_interface[32];
      ts = iniparser_getstring(ini,      "server:server_ip_interface", -1);
      if(ts != NULL){
        strcpy(server_interface, ts);
      }
      get_server_ip(server_interface, ip_address);

      //if (iniparser_set(ini,   "common:server_ip", ip_address ) == 0){
      //  printf("Updated the  server_ip in configure file. \n");
      //}
      
      ts = iniparser_getstring(ini,  "common:server_ip_file", NULL);
      if(ts != NULL){
        char name_str[512];
        strcpy(name_str, ts);
        printf("Write IP address [%s] to file [%s]. \n", ip_address, name_str);

        FILE * fp = fopen(name_str, "w");
        if(NULL == fp){
          printf("\n fopen() Error for %s !!!\n", name_str);
          return 1;
        }
        fprintf(fp, "%s\n", ip_address); 
        fflush(fp); 
        fclose(fp);
      }
    }else{
      ts  = iniparser_getstring(ini, "common:server_ip", NULL);
      if(ts != NULL){
        strcpy(ip_address, ts);
        printf("Use the default IP address [%s] \n", ip_address);
      }else{
        printf("Please specify the IP address for server \n");
      }
    }

    iniparser_freedict(ini);
    return 0 ;
}


int get_server_ip(char *ifrn, char *inet_address){
  int fd;
  struct ifreq ifr;
  
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  /* I want to get an IPv4 IP address */
  ifr.ifr_addr.sa_family = AF_INET;
  /* I want IP address attached to "eth0" */
  strncpy(ifr.ifr_name, ifrn, IFNAMSIZ-1);
  ioctl(fd, SIOCGIFADDR, &ifr);
  close(fd);
  strcpy(inet_address, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
  /* display result */
  printf("Detected server IP for interface [%s] is %s\n", ifrn, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}



