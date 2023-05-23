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
//This file is the thread to answer the metadata-related query
//  Author: Bin Dong <dbin at lbl.gov >
//  Copyright 2015 the Regents of the University of California

#include "query-handler-thread.h"
#include  "workqueue.h"
#include  "sds-server.h"
#include  "message.protoc.pb-c.h"
#include  "metadata-db-access.h"
#include  "sds-error.h"
//#include  "attribute-db-access.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>


extern sds_dbs_t g_sds_dbs;


#define fill_response_struct(response, _file_path, _file_name, _group_path, _dataset_name, _message_type, _reorganization_type, _error_node) \
      response.reorganized_file_path            = _file_path; \
      response.reorganized_file_name            = _file_name; \
      response.reorganized_gorup_path           = _group_path; \
      response.reorganized_dataset_name         = _dataset_name; \
      response.message_type                     = _message_type; \
      response.reorganization_type              = _reorganization_type; \
      response.error_code                       = _error_node
/* 
 * Main function of metadata query handler thread. 
 *    1),  wait on the metadata_query_workqueue until listen thread 
 *         wakes it up after adding new query to the workqueue
 *    2),  get and delete the requst from metadata_query_workqueue
 *    3),  call do_query function to answer the query
 */
void *query_handler_thread_function(void *ptr) {
  worker_t *worker = (worker_t *)ptr;
  job_t *job;
  
  while (1) {
    /* Wait until it get notified by listern_thread. */
    pthread_mutex_lock(&worker->workqueue->jobs_mutex);
    while (worker->workqueue->waiting_jobs == NULL) {
      pthread_cond_wait(&worker->workqueue->jobs_cond,
                        &worker->workqueue->jobs_mutex);
    }
    job = worker->workqueue->waiting_jobs;
    if (job != NULL) {
      LL_REMOVE(job, worker->workqueue->waiting_jobs);
    }
    pthread_mutex_unlock(&worker->workqueue->jobs_mutex);

    /* If we're supposed to terminate, break out of our continuous loop. */
    if (worker->terminate) break;

    /* If we didn't get a job, then there's nothing to do at this time. */
    if (job == NULL) continue;
    
    /* Execute the job. */
    do_query(job->user_data);
    free(job);
  }
    
  free(worker);
  pthread_exit(NULL);
}


/* 
 * Fill the response object defined in "../common/message.proto"
 */
void sds_response_fill_object(SdsObject *sds_object, SdsFile *sds_file){
  if(sds_file->filename != NULL)
    sds_object->filename  = strdup(sds_file->filename);
  if(sds_file->group != NULL)
    sds_object->group = strdup(sds_file->group);
  if(sds_file->dsetname != NULL)
    sds_object->dsetname = strdup(sds_file->dsetname);   
  sds_object->has_file_type  = 1;
  sds_object->has_data_type  = 1;
  sds_object->file_type = sds_file->filetype;
  sds_object->data_type = sds_file->datatype;
}

/*  
 * 1) obtain the metadata of the  requested file
 * 2) evaluate the existing organization based on the request (Todo)
 * 3) send response to client
 */
void do_query(client_t * client){
  SDSMetadataDbKey                 key                = SDS_METADATA_DB_KEY__INIT;
  SDSMetadataDbValue               value              = SDS_METADATA_DB_VALUE__INIT;
  int                              response_size;
  ClientResponse                   response           = CLIENT_RESPONSE__INIT;
  void                            *response_buf;
  int                              ret, ret2, i, j, n;
  SdsObject                       *obj;
  n = client->request->query_data->n_objects;
  response.type = MESSAGE_TYPE__QUERY;
  
  response.query_data = malloc(sizeof(ResponseQueryData));
  response_query_data__init(response.query_data);
  response.query_data->n_index_type    = n;
  response.query_data->index_type      = malloc(sizeof(int) * n);
  response.query_data->n_index_objects = n;
  response.query_data->index_objects   = malloc(sizeof(SdsObject *) * n);
  response.query_data->n_reorg_type    = n;
  response.query_data->reorg_type      = malloc(sizeof(int) * n);
  response.query_data->n_reorg_objects = n;
  response.query_data->reorg_objects   = malloc(sizeof(SdsObject *) * n);
  response.query_data->n_index_parameters = n;
  response.query_data->index_parameters     = malloc(sizeof(char    *) * n);
  response.query_data->n_reorg_parameters = n;
  response.query_data->reorg_parameters     = malloc(sizeof(char    *) * n);
  
  SdsFile    *sds_file;
  SdsObject  *sds_object;
  int         obj_used;
  
  log_msg("Quering  [%d] objects ", n);
  for(i = 0 ; i < n; i++){
    obj = client->request->query_data->objects[i];
    
    /* Create the key */
    key.filename     = obj->filename;
    key.group        = obj->group;
    key.dsetname     = obj->dsetname;
    key.filetype     = obj->file_type;
    key.datatype     = obj->data_type;

    log_msg("file [%s], group [%s], dataset [%s], file type [%d], data type [%d]", key.filename, key.group, key.dsetname, key.filetype, key.datatype);

    /* Try to find the metadata of the key */
    ret = read_metadata_record(g_sds_dbs.sds_metadata_dbp, &key, &value);
    if(ret == 0){
      response.error_code = QUERY_STATUS__SUCCESSFUL;
      
      //log_msg("Read data OK ");
      //NO reorganized file exists.
      //if (value.n_index_files == 0 && value.n_reorg_files  ==  0){
      //  response.query_data = NULL;
      // continue;
      //}
      log_msg("value.n_index_files %d, value.n_reorg_files %d", value.n_index_files, value.n_reorg_files);

      sds_object = malloc(sizeof(SdsObject));
      sds_object__init(sds_object);
      //Use the index file at the head and assume the first one is the newest one 
      //Therefore, it is the best candidate to serve the query
      //Todo: to choose the best one based on the query tree and performance models
      if(value.n_index_files > 0){
        //response.query_data->n_index_objects  = 1;
        //response.query_data->n_index_type     = 1;
        //response.query_data->index_objects    = malloc(sizeof(SdsObject *));
        //response.query_data->index_type       = malloc(sizeof(int));
        obj_used = 0 ; //Todo: choose the best one (here we choose the first one)
        sds_file = value.index_files[obj_used];
        sds_response_fill_object(sds_object, sds_file);
        printf("strlen %d\n", strlen(sds_file->parameters));
        log_msg("Find index file (%s) for original file (%s), parameter [%s]!", sds_object->filename, key.dsetname, sds_file->parameters);
        response.query_data->index_type[i]         =  sds_file->ir_type;
        response.query_data->index_parameters[i]   =  strdup(sds_file->parameters);
      }else{
        response.query_data->index_type[i]         = NONE_INDEX;
        response.query_data->index_parameters[i]   = strdup("NULL");
      }
      response.query_data->index_objects[i]   = sds_object;

      sds_object = malloc(sizeof(SdsObject));
      sds_object__init(sds_object);
      //Use the reorg file at the end and assume the first one is the newest one 
      //Therefore, it is the best candidate to serve the query
      //Todo: to choose the best one based on the query tree and performance models
      if(value.n_reorg_files > 0){
        obj_used = 0;
        sds_file = value.reorg_files[obj_used];
        sds_response_fill_object(sds_object, sds_file);
        response.query_data->reorg_type[i]         = sds_file->ir_type;
        response.query_data->reorg_parameters[i]   =  strdup(sds_file->parameters);
        log_msg("Find reorganizaed file (%s) for original file (%s), parameter [%s]!", sds_object->filename, key.dsetname, sds_file->parameters);

      }else{
        response.query_data->reorg_type[i]    = NONE_REORG;
        response.query_data->reorg_parameters[i] = strdup("NULL");
      }
      response.query_data->reorg_objects[i] = sds_object;
      
      //else{
      //  response.query_data->reorg_objects[i] = NULL;
      //  response.query_data->reorg_type[i]    = NONE_REORG;
      //}
      //It will cause problem in multiple-thread
      //increase_sds_file_read_count_by_one(&key);
    }else if(ret == DB_NOTFOUND){
      response.error_code = QUERY_STATUS__SUCCESSFUL;

      sds_object = malloc(sizeof(SdsObject));
      sds_object__init(sds_object);
      
      response.query_data->index_objects[i] = sds_object;
      response.query_data->index_type[i]    = NONE_INDEX;
      response.query_data->index_parameters[i]   = strdup("NULL");


      sds_object = malloc(sizeof(SdsObject));
      sds_object__init(sds_object);
      response.query_data->reorg_objects[i] = sds_object;
      response.query_data->reorg_type[i]    = NONE_REORG;
      response.query_data->reorg_parameters[i] = strdup("NULL");

      //File record doesn's exist, tell client no reorganization and create a new one
      //Create a new record for the file.
      //db_create_a_new_empty_record(&key, FILE_STATUS__IN_SERVICE);
      db_create_empty_record(g_sds_dbs.sds_metadata_dbp, &key, FILE_STATUS__IN_SERVICE);
      log_msg("No record founded for file %s. Create a new one.",obj->filename);
    }else{
      //Other errror happens when reading metadata record. Just tell client no reorganized file exists
      response.error_code = QUERY_STATUS__FAILED;
      response.query_data->reorg_objects[i] = NULL;
      response.query_data->index_objects[i] = NULL;
      response.query_data->index_type[i]    = NONE_INDEX;
      response.query_data->reorg_type[i]    = NONE_REORG;

      log_msg("Error happens when read metadata !");
      //Break here because Berkeley DB might have some problem
      //Client will use full-scan to answer the query
      break;
    }
  }
  
  //Send response to client
  response_size = client_response__get_packed_size(&response);
  response_buf  = malloc(response_size);
  client_response__pack(&response, response_buf);
  
  if(send(client->fd, response_buf, response_size, 0) == -1){
    log_sys("Cann't send response to client.");
  }
  
  //shutdown(client->fd,2);			
  close_client(client);
  free(response_buf);
}


/* 
 * Start the query handler thread. Basically, it is a thread pool.
 * The number of the threads in the pool is defined in sds-server.h
 */
int query_handler_thread_init(workqueue_t *workqueue, int numWorkers) {
  int i;
  worker_t *worker;
  pthread_cond_t blank_cond = PTHREAD_COND_INITIALIZER;
  pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;

  if (numWorkers < 1) 
    numWorkers = 1;

  memset(workqueue, 0, sizeof(*workqueue));
  memcpy(&workqueue->jobs_mutex, &blank_mutex, sizeof(workqueue->jobs_mutex));
  memcpy(&workqueue->jobs_cond, &blank_cond, sizeof(workqueue->jobs_cond));
    
  for (i = 0; i < numWorkers; i++) {
    if ((worker = malloc(sizeof(worker_t))) == NULL) {
      perror("Failed to allocate all workers");
      return 1;
    }
    memset(worker, 0, sizeof(*worker));
    worker->workqueue = workqueue;
    /* Create the thread from the query_handler_thread_function, defined in this file */
    if (pthread_create(&worker->thread, NULL, query_handler_thread_function, (void *)worker)) {
      perror("Failed to start all worker threads");
      free(worker);
      return 1;
    }
    LL_ADD(worker, worker->workqueue->workers);
  }

  return 0;
}
