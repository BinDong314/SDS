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
 * This file implement the communication interface from client to server
 * Author: Bin Dong <dbin at lbl.gov >
 * Copyright 2015 the Regents of the University of California
 * 
 */

#include "sds-client-communicator.h"
#include "sds-config-client.h"


//Start a socket with SERVER_ADDRESS and SERVER_PORT defined in ../common/sds-common.h
int SDS_Socket_start(){
  int                    sockfd=0;
  struct  hostent       *server_he;
  struct  sockaddr_in    server_addr;
  int                    ret;
  //char                  *ip =   server_ip;   //Defined in sds-config-client.h and sds-query.c
  //int                    port = server_port; //Defined in sds-config-client.h and sds-query.c

  printf("Server IP %s , port %d \n", server_ip, server_port);

  //printf("Try to create a get host ! ... \n ");
  if((server_he= gethostbyname(server_ip)) == NULL){
    log_quit("Error with gethostbyname() at SDS Client");
  }

  //printf("Try to create a socket ! ...");
  if((sockfd = socket(AF_INET,SOCK_STREAM, 0)) == -1){
    log_quit("Error with socket() at SDS Client");
  }
  
  server_addr.sin_family = AF_INET;
  server_addr.sin_port   = htons(server_port);
  server_addr.sin_addr   = *((struct in_addr *)server_he->h_addr);
  memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));
  
  //printf("Try to connect with server ! ...");
  ret = connect(sockfd,(struct sockaddr *)&server_addr, sizeof(struct sockaddr));
  if(ret == -1){
    log_msg("Not able to establish a communication with the SDS server!");
    return ret;
  }

  return sockfd;
}

//Receive data from socket
int SDS_Socket_recv(int sockfd, void *buf, int length){
  int size;
  size = recv(sockfd, buf, length, 0);
  if(size == -1)
    log_quit("Error with recv() at SDS Client.");
  
  return size;
}

//Send data to socket
int SDS_Socket_sent(int sockfd, void *buf, int length){
  if(send(sockfd, buf, length, 0) == -1)
    log_quit("Error with send() data from SDS Client to SDS Server");
  return 0;
}

//Stop the socket
int SDS_Socket_stop(int sockfd){
  shutdown(sockfd,2);
}


//Read metadata for the collection from SDS Server with one communication 
int SDS_read_collection_metadata(SDS_Object    **obj_array, int size){
  int                    i, sockfd;
  ClientRequest          query    =   CLIENT_REQUEST__INIT;
  ClientResponse        *response;
  int                    q_len,   r_len;
  void                  *q_buf,  *r_buf; 
  
  //Pasre the client configure file
  parse_client_file(ini_file);

  //Start a socket
  sockfd = SDS_Socket_start();
  if(sockfd < 0){
    return -1;
  }
  
  //Fill the query
  query.type          =  MESSAGE_TYPE__QUERY; 
  query.query_data    =  malloc(sizeof(RequestQueryData));
  request_query_data__init(query.query_data);
  query.query_data->n_objects = size;
  query.query_data->objects   = malloc(sizeof(SdsObject *) * size);
  
  
  SdsObject *new_obj;
  for(i = 0 ; i < size; i++){
    new_obj = malloc(sizeof(SdsObject));
    sds_object__init(new_obj);
    if(obj_array[i] != NULL){
      query_fill_object(new_obj, obj_array[i]);
      //printf("Fill %s \n", new_node->dataset_name);
    }
    query.query_data->objects[i]=new_obj;
  }
  
  q_len = client_request__get_packed_size(&query);
  
  //printf("request size %d \n", q_len);
  q_buf = malloc(q_len);
  client_request__pack(&query, q_buf);
  //Send to SDS Server
  SDS_Socket_sent(sockfd, q_buf,    q_len);

  r_buf=  malloc(MAX_MESSAGE_LENGTH);
  memset(r_buf, 0, MAX_MESSAGE_LENGTH);
  
  //Receive from SDS Server
  r_len = SDS_Socket_recv(sockfd, r_buf, MAX_MESSAGE_LENGTH);
  response = client_response__unpack(NULL, r_len, r_buf);
  
  if(response->error_code != QUERY_STATUS__SUCCESSFUL){
    log_quit("Response received at Client with error \n");
  }
  log_msg("Response received at Client ! ");
  //Handle the results.
  SDS_Index_file              *index_file;
  SDS_Reorg_file              *reorg_file;
  for(i = 0 ; i < size; i++){
    if(response->query_data->index_type[i] != NONE_INDEX){
      index_file =  malloc(sizeof(SDS_Index_file));
      query_fill_index(index_file, response->query_data->index_objects[i]);
      index_file->index_type = response->query_data->index_type[i];
      index_file->parameters = strdup(response->query_data->index_parameters[i]);
      obj_array[i]->index_files = index_file;
      log_msg("response->query_data->index_objects[i]->filename %s ", response->query_data->index_objects[i]->filename);
    }else{
      obj_array[i]->index_files = NULL;
    }

    if(response->query_data->reorg_type[i] != NONE_REORG){
      //printf("Find reorganized file !\n");
      reorg_file = malloc(sizeof(SDS_Reorg_file));
      query_fill_reorg(reorg_file, response->query_data->reorg_objects[i]);
      reorg_file->reorg_type = response->query_data->reorg_type[i];
      reorg_file->parameters = strdup(response->query_data->reorg_parameters[i]);
      obj_array[i]->reorg_files = reorg_file;
    }else{
      obj_array[i]->reorg_files = NULL;
    }
  }
 
  //Stop the socket
  SDS_Socket_stop(sockfd);

  ClientRequest_free(&query);
  free(q_buf);
  free(r_buf);
  return 1;
}

//Start  admin work on server
void SDS_admin(){

}


//Start reorganization jobs on objections in a collection
int SDS_start_collection_reorg(SDS_Object    **obj_array, int *index_type, int *reorg_type, int *cores, int *time, char **parameters, int size, int *reorg_status){
  int                    i, sockfd;
  ClientRequest          query    = CLIENT_REQUEST__INIT;
  ClientResponse        *response;
  int                    q_len,   r_len;
  void                  *q_buf,  *r_buf; 

  parse_client_file(ini_file);
  
  //Start a socket
  sockfd = SDS_Socket_start();
  if(sockfd < 0){
    return -1;
  }

  
  //Fill the query
  query.type          =  MESSAGE_TYPE__REORG; 
  query.reorg_data    =  malloc(sizeof(RequestReorgData));
  request_reorg_data__init(query.reorg_data);
  query.reorg_data->n_objects = size;
  query.reorg_data->n_index_type  = size;
  query.reorg_data->n_reorg_type  = size;
  query.reorg_data->n_index_cores       = size;
  query.reorg_data->n_reorg_cores       = size;
  query.reorg_data->n_index_time_secs   = size;
  query.reorg_data->n_reorg_time_secs   = size;
  query.reorg_data->n_index_parameters  = size;
  query.reorg_data->n_reorg_parameters  = size;

  query.reorg_data->objects     = malloc(sizeof(SdsObject *) * size);
  query.reorg_data->index_type  = index_type;   //malloc(sizeof(int) * size);
  query.reorg_data->reorg_type  = reorg_type;   //malloc(sizeof(int) * size);
  query.reorg_data->reorg_cores       = cores;        //malloc(sizeof(int) * size);
  query.reorg_data->index_cores       = cores;        //malloc(sizeof(int) * size);
  query.reorg_data->index_time_secs   = time;         //malloc(sizeof(int) * size);
  query.reorg_data->reorg_time_secs   = time;         //malloc(sizeof(int) * size);
  query.reorg_data->index_parameters  = parameters;   // malloc(sizeof(char *) * size);
  query.reorg_data->reorg_parameters  = parameters;   // malloc(sizeof(char *) * size);

 
  SdsObject *new_obj;
  for(i = 0 ; i < size; i++){
    new_obj = malloc(sizeof(SdsObject));
    sds_object__init(new_obj);
    if(obj_array[i] != NULL){
      query_fill_object(new_obj, obj_array[i]);
      printf("Fill %s \n", new_obj->dsetname);
    }
    query.reorg_data->objects[i]=new_obj;
  }
  
  q_len = client_request__get_packed_size(&query);
  
  printf("request size %d \n", q_len);
  q_buf = malloc(q_len);
  client_request__pack(&query, q_buf);
  //Send to SDS Server
  SDS_Socket_sent(sockfd, q_buf,    q_len);

  r_buf=  malloc(MAX_MESSAGE_LENGTH);
  memset(r_buf, 0, MAX_MESSAGE_LENGTH);
  
  //Receive from SDS Server
  r_len = SDS_Socket_recv(sockfd, r_buf, MAX_MESSAGE_LENGTH);
  response = client_response__unpack(NULL, r_len, r_buf);
  
  if(response->error_code != QUERY_STATUS__SUCCESSFUL){
    log_quit("Response received at Client with error ");
  }
  log_msg("Response received at Client  and status is ! ");
  for(i = 0; i < response->reorg_data->n_status; i++){
    switch(response->reorg_data->status[i]){
      case REORG_STATUS__ACCEPTED:
        printf(" Accepted \n");
        break;
      case REORG_STATUS__SUBMITTED:
        printf(" SUBMITTED \n");
        break;
      case REORG_STATUS__RUNNING:
        printf(" RUNNING \n");
        break;
      case REORG_STATUS__FINISH_WITHOUT_ERROR:
        printf(" FINISH without error \n");
        break;
      case REORG_STATUS__FINISH_WITH_ERROR:
        printf(" FINISH with errors  \n");
        break;
      default:
        printf("Unknow status! \n");
        break;
    }
  }
  
  //Stop the socket
  SDS_Socket_stop(sockfd);

  //Release all memory
  ClientRequest_free(&query);
  free(q_buf);
  free(r_buf);

  return 1;
}



//Start a analysis job on server for the obj_array 
SDS_Collection    *SDS_Remote_analyze(SDS_Query_tree *query_tree){
  int                    i, sockfd, n_node=0, i_node=0;
  ClientRequest          query    = CLIENT_REQUEST__INIT;
  SDS_Query_tree       **query_tree_serialized;
  ClientResponse        *response;
  int                    q_len,   r_len;
  void                  *q_buf,  *r_buf; 
  SDS_Collection        *result_collection;

  SDS_Query_tree_size(query_tree, &n_node);
  query_tree_serialized = malloc(sizeof(SDS_Query_tree *) * n_node);
  SDS_Query_tree_serialize(query_tree, query_tree_serialized, &i_node);
  
  //Start a socket
  sockfd = SDS_Socket_start();

  //Fill the query
  query.type          =  MESSAGE_TYPE__ANALY; 
  query.analy_data    =  malloc(sizeof(RequestAnalyData));
  request_analy_data__init(query.analy_data);
  query.analy_data->n_query_tree       = n_node;
  query.analy_data->query_tree         = malloc(sizeof(QueryTreeNode *) * n_node);
  
  
  QueryTreeNode *tree_node;
  for(i = 0 ; i < n_node; i++){
    if(query_tree_serialized[i] != NULL){
      tree_node = malloc(sizeof(QueryTreeNode));
      //tree_node__init(tree_node);
      query_fill_query_tree_node(tree_node, query_tree_serialized[i]);
      query.analy_data->query_tree[i] = tree_node;
    }else{
      query.analy_data->query_tree[i] = NULL;
    }
  }
  
  q_len = client_request__get_packed_size(&query);
  
  //printf("request size %d \n", q_len);
  q_buf = malloc(q_len);
  client_request__pack(&query, q_buf);
  //Send to SDS Server
  SDS_Socket_sent(sockfd, q_buf,    q_len);

  r_buf=  malloc(MAX_MESSAGE_LENGTH);
  memset(r_buf, 0, MAX_MESSAGE_LENGTH);
  
  //Receive from SDS Server
  r_len = SDS_Socket_recv(sockfd, r_buf, MAX_MESSAGE_LENGTH);
  response = client_response__unpack(NULL, r_len, r_buf);
  
  if(response->error_code != QUERY_STATUS__SUCCESSFUL){
    log_quit("Response received at Client with error \n");
  }
  log_msg("Response received at Client ! \n");

  result_collection = SDS_Collection_init(response->analy_data->n_result_objects);
  //Handle the results.
  SDS_Object *new_obj;
  for(i = 0 ; i < response->analy_data->n_result_objects; i++){
    new_obj = malloc(sizeof(SDS_Object));
    query_fill_object_inverse(new_obj, response->analy_data->result_objects[i]);
    SDS_Collection_append(result_collection, new_obj);
  }
  
  //Stop the socket
  SDS_Socket_stop(sockfd);

  //Release all memory
  ClientRequest_free(&query);

  free(q_buf);
  free(r_buf);

  return result_collection;
}

int SDS_start_trace_analysis(SDS_Object    *obj, int mpi_rank, char *dir_name, char *app_name, int dim_rank){
  int                    i, sockfd;
  ClientRequest          query    = CLIENT_REQUEST__INIT;
  ClientResponse        *response;
  int                    q_len,   r_len;
  void                  *q_buf,  *r_buf; 

  parse_client_file(ini_file);
  
  //Start a socket
  sockfd = SDS_Socket_start();
  if(sockfd < 0){
    return -1;
  }
  
  //Fill the query
  query.type                   =  MESSAGE_TYPE__TRACE; 
  query.trace_data             =  malloc(sizeof(RequestTraceData));
  request_trace_data__init(query.trace_data);
  query.trace_data->object     = malloc(sizeof(SdsObject));
  sds_object__init(query.trace_data->object);
  query_fill_object(query.trace_data->object, obj);
  query.trace_data->mpi_rank   = mpi_rank;
  /* query.trace_data->dim_rank   = dim_rank; */
  /* query.trace_data->app_name   = app_name; */
  /* query.trace_data->dir_name   = dir_name; */

  
  q_len = client_request__get_packed_size(&query);
  
  q_buf = malloc(q_len);
  
  client_request__pack(&query, q_buf);
  //Send to SDS Server
  SDS_Socket_sent(sockfd, q_buf,    q_len);

  r_buf=  malloc(MAX_MESSAGE_LENGTH);
  memset(r_buf, 0, MAX_MESSAGE_LENGTH);
  
  //Receive from SDS Server
  r_len = SDS_Socket_recv(sockfd, r_buf, MAX_MESSAGE_LENGTH);
  response = client_response__unpack(NULL, r_len, r_buf);
  
  if(response->error_code != QUERY_STATUS__SUCCESSFUL){
    log_quit("Response received at SDS Client with error for trace log !");
    exit(-1);
  }else{
    log_msg("The analysis is started !");
  }
  
  //Stop the socket
  SDS_Socket_stop(sockfd);

  //Release all memory
  ClientRequest_free(&query);
  free(q_buf);
  free(r_buf);

  return 1;
 
}

