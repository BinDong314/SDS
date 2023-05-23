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
#ifndef __SDS_SERVER_H__
#define __SDS_SERVER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>
#include <err.h>


#include "sds-common.h"
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <message.protoc.pb-c.h>

/* 
 * Size of metadata query handler thread pool. 
 * It is equal to the number of CPU cores minus two 
 * -- one for listen thread, and another one for reorganization
 * The number of CPU cores is reported in /proc/cpuinfo.
 */


/*
 * Struct to carry around connection (client)-specific data.
 */
typedef struct client {
  /* The client's socket. */
  int fd;
  /* The event_base for this client. */
  struct event_base  *evbase;
  /* The bufferedevent for this client. */
  struct bufferevent *buf_ev;
  /* The output buffer for this client. */
  struct evbuffer    *output_buffer;
  /* Here you can add your own application-specific attributes which
   * are connection-specific. */
  ClientRequest      *request;

  struct bufferevent *bev;
}client_t;



int  daemon_it(char *application_name);




#endif
