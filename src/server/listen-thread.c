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
 * This file is listen thread of SDS Server
 *  Receive request from client and decode the type to schedule it to proper thread(s)  
 *  Author: Bin Dong <dbin at lbl.gov >
 *  Copyright 2015 the Regents of the University of California
 * 
 */

#include "listen-thread.h"
#include "sds-error.h"
#include "sds-common.h"
#include "message.protoc.pb-c.h"
#include "workqueue.h"
#include "sds-server.h"



/* System library */
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <err.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

//Base event used in listen thread (required by libevent)
static struct event_base  *evbase_accept;                       

/* Folloing two queue is declared at begining of server/sds-server.c */
extern  workqueue_t         metadata_query_workqueue;             
extern  workqueue_t         reorganization_request_workqueue;   


/**
 * Run the listen server.  This function blocks, only returning when the server has 
 * terminated.
 */
int run_listen_server(int port){
  evutil_socket_t    listenfd;
  struct sockaddr_in listen_addr;
  struct event      *ev_accept;
  int                reuseaddr_on;

  /* Create listening socket. */
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) {
    log_sys("Can't create listen socket.");
  }


  memset(&listen_addr, 0, sizeof(listen_addr));
  listen_addr.sin_family       = AF_INET;
  listen_addr.sin_addr.s_addr  = INADDR_ANY;
  listen_addr.sin_port         = htons(port);

  reuseaddr_on = 1;
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on, sizeof(reuseaddr_on));

  if (bind(listenfd, (struct sockaddr *)&listen_addr, sizeof(listen_addr))  < 0) {
    log_sys("Cann't bind to port %d: %s ", port, strerror(errno));
  }
  
  /*Increase maximum backlog from system when large number of clients send their request concurrently */
  if (listen(listenfd, SOMAXCONN) < 0) {
    log_sys("Listen on the socket failed");
  }

  /* Set the socket to non-blocking, this is essential in event
   * based programming with libevent. */
  if (evutil_make_socket_nonblocking(listenfd) < 0) {
    log_sys("Failed to set server socket to non-blocking:");
    
  }

  evthread_use_pthreads();
  

  /*  Crate base event of libevent */
  if ((evbase_accept = event_base_new()) == NULL) {
    close(listenfd);
    //workqueue_shutdown(&metadata_query_workqueue);
    //workqueue_shutdown(&reorganization_request_workqueue);
    log_quit("Unable to create socket accept event base");
  }

  /* We now have a listening socket, we create a read event to
   * be notified when a client connects. */
  ev_accept = event_new(evbase_accept, listenfd, EV_READ|EV_PERSIST, on_accept, (void *)evbase_accept);
  event_add(ev_accept, NULL);
  
  log_msg("Listen thread at SDS Server is running.");

  /* 
   *Start the libevent loop. 
   * It will not terminate until user stop it
   */
  event_base_dispatch(evbase_accept);
  event_base_free(evbase_accept);
  evbase_accept = NULL;
  close(listenfd);
  printf("Listen thread at SDS Server is shutdown.\n");
  return 0;
}

//Do nothing 
void write_cb(struct bufferevent *bev, void *arg) {}

void error_cb(struct bufferevent *bev, short event, void *arg)
{
    evutil_socket_t fd = bufferevent_getfd(bev);
    printf("fd = %u, ", fd);
    if (event & BEV_EVENT_TIMEOUT) {
        printf("Timed out\n"); //if bufferevent_set_timeouts() called
    }
    else if (event & BEV_EVENT_EOF) {
        printf("connection closed\n");
    }
    else if (event & BEV_EVENT_ERROR) {
        printf("some other error\n");
    }
    bufferevent_free(bev);
}

/**
 * This function will be called by libevent when there is a connection
 * ready to be accepted.
 */
void on_accept(evutil_socket_t fd, short ev, void *arg) {
  int                client_fd;
  struct sockaddr_in client_addr;
  socklen_t          client_len = sizeof(client_addr);
  struct event_base *evbase_accept = (struct event_base *)arg;

  client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
  if (client_fd < 0) {
    log_ret("Socket accept failed");
    return;
  }

  /* Set the client socket to non-blocking mode, required by libevent */
  if (evutil_make_socket_nonblocking(client_fd) < 0) {
    log_ret("Failed to set client socket to non-blocking");
    close(client_fd);
    return;
  }

  /* Create the buffered event.Just listen on the read event */
  int options = BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE ;
  struct bufferevent *bev = bufferevent_socket_new(evbase_accept, client_fd, options);
  //struct bufferevent *bev = bufferevent_socket_new(base, client_fd, options);
  bufferevent_setcb(bev, buffered_on_read, NULL, NULL, arg);
  //bufferevent_enable(bev, EV_READ);
  bufferevent_enable(bev, EV_READ|EV_WRITE|EV_PERSIST);
  return;
}


/**
 * Called by libevent when there is data to read.
 */
void buffered_on_read(struct bufferevent *bev, void *arg) {
  uint8_t              request_data[MAX_MESSAGE_LENGTH]={0};
  int                  request_size;
  ClientRequest       *request;
  client_t            *client;
  job_t               *job;

  
  /* Read the actuall request, and unpack it with the function generated by protobuf-c  */
  request_size = bufferevent_read(bev, request_data, MAX_MESSAGE_LENGTH);       
  request      = client_request__unpack(NULL, request_size, request_data);
   
  /* Create a client object. */
  if ((client = malloc(sizeof(client_t))) == NULL) {
    log_sys("failed to allocate memory for client state");
    return;
  }
  
  /* Extract info from request package,and store them in client */
  memset(client, 0, sizeof(client_t));
  //Client's soket id, stored for response
  client->fd = bufferevent_getfd(bev);                  
  //client->message_type = request->type;           
  client->request = request;
  
  //client->bev   =  bev;
  /* Create a job object and add it to specific work queue. */
  if ((job = malloc(sizeof(job_t))) == NULL) {
    log_msg("failed to allocate memory for job state");
    close_client(client);
    return;
  }
  job->user_data = client;

  /* Based on request type, assgin a job to different queue */
  switch(request->type){
    case MESSAGE_TYPE__QUERY:
      log_msg("Receive a metadata query request !");
      workqueue_add_job(&metadata_query_workqueue, job);
      break;
    case MESSAGE_TYPE__REORG:
      log_msg("Receive a reorganization query !");
      workqueue_add_job(&reorganization_request_workqueue, job);
      break;
    case MESSAGE_TYPE__ANALY:
      //Add it to the reorganization query
      log_msg("Receive a analysis request !");
      workqueue_add_job(&reorganization_request_workqueue, job);
      break;
    case MESSAGE_TYPE__ADMIN:
      log_msg("No Support to ADMIN message in current version ");
      break;
    case MESSAGE_TYPE__TRACE:
      log_msg("Receive a trace log analysis request !");
      workqueue_add_job(&reorganization_request_workqueue, job);
      break;
    default:
      log_msg("Unsupported request from client");
      break;
  }

  //sds_client_request__free_unpacked(request, NULL);
  //  return 0;
}

void close_client(client_t *client) {
  if (client != NULL) {
    //if(client->bev != NULL)
    // bufferevent_free(client->bev);
    if (client->fd >= 0) {
      shutdown(client->fd,2);
      //close(client->fd);
      client->fd = -1;
    }
    free(client);
  }
}
