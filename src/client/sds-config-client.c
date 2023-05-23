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
 * This file defines the interface to analyze the configure files for SDS Client 
 * Also, it defines the "extern global variables" used by server. 
 * Author: Bin Dong <dbin at lbl.gov >
 * Copyright 2015 the Regents of the University of California
 * 
 */

#include "sds-config-client.h"



int parse_client_file(char * ini_name)
{
    dictionary   *ini;
    char        *ts;
    int          ti;
    
    /* Some temporary variables to hold query results */
    ini = iniparser_load(ini_name);
    if (ini==NULL) {
      log_quit( "cannot parse config file: %s for SDS Client\n", ini_name);
    }
    
    ts = iniparser_getstring(ini,    "common:sds_root_path", NULL);
    if(ts != NULL){
      strcpy(client_sds_root_path, ts);
    }else{
      log_quit("common:sds_root_path should be specified in %s ! ", ini_name);
    }
    printf("   Derectory to store SDS Augmented files: [%s] \n", client_sds_root_path);

    ti = iniparser_getboolean(ini, "common:cluster_version", -1); 
    if(ti != -1){
      client_cluster_version  = ti;
    }else{
      log_quit("common:cluster_version should be specified in %s ! ", ini_name);
    }

    if(client_cluster_version){
      printf("   This is cluster version: [TRUE] \n");
    }else{
      printf("   This is cluster version: [FALSE] \n");
    }

    int update_ip_flag = iniparser_getboolean(ini,    "common:update_server_ip_flag", -1);
    char server_ip_temp[MAX_FILE_NAME_LENGTH];
    memset(&server_ip_temp[0], 0, MAX_FILE_NAME_LENGTH);
    if (update_ip_flag == 1){
      ts = iniparser_getstring(ini,  "common:server_ip_file", NULL);
      if(ts != NULL){
        char name_str[512];
        strcpy(name_str, ts);
        FILE * fp = fopen(name_str, "r");
        printf("fopen() for %s !!!\n", name_str);
        if(NULL == fp){
          printf("\n fopen() Error for %s !!!\n", name_str);
          return 1;
        }
        fseek(fp, SEEK_SET, 0);
        //int str_len = fread(&server_ip_temp[0], sizeof(char), MAX_FILE_NAME_LENGTH, fp);
        fgets(&server_ip_temp[0], MAX_FILE_NAME_LENGTH, fp);
        int len = strlen(server_ip_temp);
        if (len > 0 && server_ip_temp[len-1] == '\n') server_ip_temp[len-1] = '\0';
        //if (str_len > 0){
        //  printf("Str-len = %d \n", str_len);
        //  server_ip_temp[str_len] = 0;
        strncpy(server_ip, server_ip_temp, len-1);
        //}
        fclose(fp);
      }
    }else{
      ts = iniparser_getstring(ini, "common:server_ip", NULL);
      if(ts != NULL){
        strcpy(server_ip, ts);
      }else{
        log_quit("client:server_ip should be specified in %s ! ", ini_name);
      }
    }
    printf("   SDS Server IP: [%s] \n", server_ip);
    
    ti = iniparser_getint(ini,      "common:server_port", -1);
    if(ti != -1){
      server_port = ti;
    }else{
      log_quit("common:server_port should be specified in %s ! ", ini_name);
    }

    printf("   SDS Server port : [%d] \n", server_port);
    
    iniparser_freedict(ini);
    return 0 ;
}


