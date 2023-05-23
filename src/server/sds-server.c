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
 * This file is the main for SDS-Server
 *  Start the threads, one for listen and NUM_QUREY_HANDLER_THREADS threads for the query handling.
 * 
 *  Author: Bin Dong <dbin at lbl.gov >
 *  Copyright 2015 the Regents of the University of California
 * 
 */

#include "sds-common.h"
#include "sds-server.h"
#include "metadata-db-access.h"
//#include "message.protoc.pb-c.h"
#include "workqueue.h"
#include "reorganization-manager-thread.h"
#include "query-handler-thread.h"
#include "sds-error.h"
#include "listen-thread.h"


/* System library */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>
#include <unistd.h>
#include <syslog.h>
#include <getopt.h>

/* Interval to invoke thread for performing reorganization work */
int         query_threads_num=2;
/* Directory path for storing reorganized file data*/
char        sds_root_path[MAX_FILE_NAME_LENGTH]="./SDS-ROOT_DIR";
/* The interval to start monitor threads */
int         monitor_interval=20;
/* Will the SDS Server run on a cluster, like Edison */
SDS_Bool    cluster_version=SDS_TRUE;
/* The command to run the job */
char        run_command[MAX_FILE_NAME_LENGTH]="aprun";
//The port of server to listen client requests
int         server_port=90001;
/* The ini configureation files for SDS Server */
char        ini_file[MAX_FILE_NAME_LENGTH] = "./sds.conf";
/* In memory Berkeley DB for metadata (mostly for performance test)
 * Note: seting SDS_TRUE might cause data lost
 */
SDS_Bool    metadb_in_memory = SDS_FALSE;

//Global variable for the hanlde of metadata and attribute database
sds_dbs_t                  g_sds_dbs;                             
//Workqueue, used  to pass request from listen thread to metadata query hanlde thread
workqueue_t                metadata_query_workqueue;             
//Workqueue, used to  pass reqornization request from listen thread to reorganization thread
workqueue_t                reorganization_request_workqueue;   
//1: print error info on stderr; 0: store error info to file
int                        log_to_stderr = 1;


int usage(FILE *std,char *appname){
 
  char *msg="Usage: %s [OPTION] \n\
      	  -h help (--help)\n\
	  -d run as daemon (--daemon)\n\
          -c create metadata and attribute database (--createdb)\n\
          -p listen port number (--port)\n\
          -s directory for stored SDS (reorganized) file\n\
          -i interval (seconds) to invoke SDS reorganization manger thread \n\
          -o configure file \n";
    fprintf(std, msg, appname);
  return 1;
}

static struct option long_options[] =
{
  {"help",     no_argument,       0, 'h'},
  {"daemon",   no_argument,       0, 'd'},
  {"createdb", no_argument,       0, 'c'},
  {"port",     required_argument, 0, 'p'},
  {"sds directory",      required_argument, 0, 's'},
  {"interva",      required_argument, 0, 'i'},
  {"configure file", required_argument, 0, 'o'},
  {0, 0, 0, 0}
};

 
int main(int argc, char **argv){
  char         *appname="sds-server";
  //  static int    syslog_flag=0; 
  int           daemon=0;
  int           create_db_flag=0;
  int           port=0;


  static int c;
  while (1) {
    /* getopt_long stores the option index here. */
    int option_index = 0;
    c = getopt_long(argc, argv, "hdcp:s:i:o:",long_options, &option_index);
 
    /* Detect the end of the options. */
    if (c == -1)
      break;
 
    switch (c) {
      case 'h':
        usage(stdout,appname);
        return EXIT_SUCCESS;
      case 'd':
        daemon=1;
        break;
      case 'c':
        create_db_flag = 1;
        break;
      case 'p':
        port=atoi(optarg);
        break;
      case 's':
        strcpy(sds_root_path, optarg);
        break;
      case 'i':
        monitor_interval=atoi(optarg);
        break;
      case 'o':
        strcpy(ini_file, optarg);
        break;
      default:
        log_quit("Unknow options !");
    }
  }

  /*
   *Parser the configures file to find user provided parameters
   */
  parse_server_file(ini_file);
  
  /*
   * Before starts sds-server first time, one needs to create two
   * database(metadata and attribute) with -c option
   *
   * Otherwise, opens the existing databases for reading & writing  
   */
  if(create_db_flag){
    //Do remever to close after "create_db_done:"
    create_sds_metadata_db();
    //create_sds_query_db();
    /* Following two functions is used to test purpose: write some fate file record */
    //write_fake_data();
    //read_fake_data();
    goto create_db_done;
  }else{
    open_sds_metadata_db();
    //open_sds_query_db();
    //write_fake_data();
    //read_fake_data();
    //write_metadata_test_data();
    //write_fake_data_5T();
  }


  /* Use default port (defined in common/sds-common.h) if it is not specified */
  if(port == 0){
    port = server_port;
  }
 
  /* Store error information to syslog when server runs as daemon */
  if(daemon == 1){
    log_to_stderr = 0;
    log_open(appname, LOG_PID, LOG_USER);
    log_msg("started by User %d at port %d \n", getuid(), port);
    /* Run sds-server as a demon */
    daemon_it(appname);
  }else{
    printf("%s started by User %d at port %d \n",appname, getuid(), port);
  }


  /* Initialize metadata query handler thread and its work queue*/
  if (query_handler_thread_init(&metadata_query_workqueue, query_threads_num)) {
    workqueue_shutdown(&metadata_query_workqueue);
    log_quit("Failed to create thread for metadata query handler");
  }
  log_msg("Query Handler thread at SDS Server is running.");

  
  /* Start reorganization handler thread and its work queue. */
  if (reorganization_manager_thread_init(&reorganization_request_workqueue, 1)) {
    workqueue_shutdown(&reorganization_request_workqueue);
    log_quit("Failed to create thread for reorganization work manager");
    return 1;
  }
  log_msg("Reorganization thread at SDS Server is running.");
  
 
  /*The run_listen_server will run until it is terminated, it is based on libevent.  */
  run_listen_server(port);
  
create_db_done:
  if(create_db_flag){
    close_sds_metadata_db();
    //close_sds_db();
  }

  exit(EXIT_SUCCESS);
}


int daemon_it(char *application_name){
  char        f_pid_name[FILENAME_MAX]="/var/run/";
  pid_t       pid, sid;
  FILE       *f_pid;
  strcat(f_pid_name, application_name);
  strcat(f_pid_name,".pid");
  
  /* Fork off the parent process */
  pid = fork();
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }
    
  /* If we got a good PID, then
   * we can exit the parent process. */
  if (pid > 0) {
    /*Write PID in /var/run/xxx.pid*/
    f_pid=fopen(f_pid_name,"w");
    if(f_pid != NULL){
      fprintf(f_pid,"%i\n",pid);
      fclose(f_pid);
    }else{
      log_sys(" opening %s file failed!", f_pid_name);
    }
    exit(EXIT_SUCCESS);
  }
  /* Change the file mode mask */
  umask(0);
  
  /* Create a new SID for the child process */
  sid = setsid();
  if (sid < 0) {
    /* Log the failure */
    exit(EXIT_FAILURE);
  }
  
  /* Change the current working directory */
  if ((chdir("/")) < 0) {
    /* Log the failure */
    exit(EXIT_FAILURE);
  }
 
  /* Close out the standard file descriptors */
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  return 0;
}











