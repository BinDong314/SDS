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

#ifndef _SDS_CONFIG_SERVER_H_
#define _SDS_CONFIG_SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h> /* for strncpy */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#include "iniparser.h"
#include "sds-common.h"

/* Interval to invoke thread for performing reorganization work */
extern int         query_threads_num;
/* Directory path for storing reorganized file data*/
extern char        sds_root_path[MAX_FILE_NAME_LENGTH];
/* The interval to start monitor threads */
extern int         monitor_interval;
/* Will the SDS Server run on a cluster, like Edison */
extern SDS_Bool    cluster_version;
/* The command to run the job */
extern char        run_command[MAX_FILE_NAME_LENGTH];
//The port of server to listen client requests
extern int         server_port;
/* In memory Berkeley DB for metadata (mostly for performance test)
 * Note: seting SDS_TRUE might cause data lost
 */
extern SDS_Bool    metadb_in_memory ;

int  parse_server_file(char * ini_name);

#endif
