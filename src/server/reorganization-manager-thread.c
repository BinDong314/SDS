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
 * This file is the main thread who receives "build reorganization/build index/run an * alysis" request from listen thread
 * Author: Bin Dong <dbin at lbl.gov >
 * Copyright 2015 the Regents of the University of California
 * 
 */

#include "reorganization-manager-thread.h"
#include "sds-config-server.h"
#include "trace_log.h"


/* Global variable defined in sds-server.c */
extern sds_dbs_t g_sds_dbs;

/* Global variable to maitain the running jobs' status */
static sds_job_list job_queue;

/* Set a timeout for pthread_cond_timedwait */
void maketimeout(struct timespec *tsp, long sconds) {
  struct timeval now;
  /* get the current time */
  gettimeofday(&now, NULL);
  tsp->tv_sec = now.tv_sec;
  /* usec to nsec */ 
  /* add the offset to get timeout value */
  tsp->tv_nsec = now.tv_usec * 1000; 
  tsp->tv_sec += sconds;
}


/* Find  the reorganization candidate from read statictis in metadata database*/
int find_most_recently_read_file(SDSMetadataDbKey *key, SDSMetadataDbValue *value){
  int                     no_hot_file = 0;
  no_hot_file = iterate_sds_db_for_hot_file(g_sds_dbs.sds_metadata_dbp, key, value);
  return no_hot_file;
}


/*
 * Try to find an better reorganization
 * It uses the "Reorganization Recommonder"--TODO  
 */
int update_reorganization_type(int expect_reorganization_type){
  return expect_reorganization_type;
}


/*
 *Check existing of the reorganization/index of type
 */
SDS_Bool check_ir_exist(int type, SdsFile **files, int n){
  SDS_Bool exist = SDS_FALSE;
  int i;
  for(i = 0; i < n; i++){
    if(files[i] != NULL)
      if(files[i]->ir_type == type){
        exist = SDS_TRUE;
      }
  }
  return exist;
}


/* 
 * Create a job for new reorganization and start it 
 */
int prepare_and_start_one_reorganization(SdsObject *object, int job_type, int job_subtype, int cores, int time_secs, char *other_pamaters){
  sds_job  *new_job;

  char     dir_name[MAX_FILE_NAME_LENGTH];
  char     file_name[MAX_FILE_NAME_LENGTH];

  //Get the dir and file names (wihout the slash "/" at the end of dir_name or at the beginning of file_name)
  split_path(object->filename, dir_name, file_name);

  //Create and intialize a job struct
  new_job   = malloc(sizeof(sds_job));
  if(new_job == NULL){
    log_sys("Failed to allocation memory for reorganization job !");
  }

  //Generall information for the job
  new_job->job_type                                  = job_type;
  new_job->job_subtype                               = job_subtype;
  new_job->status                                    = SUBMITTED;
  new_job->number_of_cores                           = cores;
  new_job->time_secs                                 = time_secs;
  strcpy(new_job->other_parameters, other_pamaters);

  //Add original file infomation
  new_job->original_file_info.file_name              = object->filename;
  new_job->original_file_info.group_name             = object->group;
  new_job->original_file_info.dataset_name           = object->dsetname;
  new_job->original_file_info.data_type              = object->data_type;
  new_job->original_file_info.file_type              = object->file_type;


  //Store the SDS file into SDS-ROOT-DIRECTORY
  new_job->new_file_info.file_name = malloc(MAX_FILE_NAME_LENGTH*2);
  sprintf(new_job->new_file_info.file_name, "%s/%s", sds_root_path, file_name);
  new_job->new_file_info.group_name          = object->group;
  new_job->new_file_info.dataset_name        = object->dsetname;
  new_job->new_file_info.data_type           = object->data_type;
  new_job->new_file_info.file_type           = object->file_type;
  
 
  //Start the job
  if(cluster_version == SDS_TRUE){
    if(start_batch_job(new_job) != 0){
      log_msg("Start reorganization job fail ! \n");
      return 1;
    }else{
      //Add to reorganization job list for tracking
      LL_ADD(new_job, job_queue.jobs);
      job_queue.job_count++;
    }
  }else{
    log_msg("Start the job as none batch version ! \n");
    if(start_synch_job(new_job) != 0){
      log_msg("Job finished with error ! \n");
      handle_result_with_error(new_job);
      return 1;
    }else{
      log_msg("Job is finished without error ! \n");
      handle_result_without_error(new_job);
    }
  }
  
}

void evaluate_new_reorganization(){
  SDSMetadataDbKey        key;
  SDSMetadataDbValue      value;
  int                     has_hot_file;
  int                     expect_reorganization_type, final_reorganization_type;
  

  //Find the most recently read file from metadata database
  has_hot_file = find_most_recently_read_file(&key, &value);
  if(has_hot_file == 0){
    log_msg("No hot file is found ! \n");
    return;
  }

  //The reorganization type specified by the user
  expect_reorganization_type = BITMAP_INDEX;  //value.expect_reorganization_type;

  //Use "Reorganization Recommander" (TODO) to figure out most optimal organization
  final_reorganization_type  = BITMAP_INDEX; // update_reorganization_type(expect_reorganization_type);
  
  //Check whether such reorganization exists
  //if (check_ir_exist(final_reorganization_type, value.sds_file, value.n_sds_file) == 1 ){
  //  reset_sds_file_read_count(g_sds_dbs.sds_metadata_dbp, &key, 0);
  //  log_msg("The organization to be sponsored exists ! \n");
  //  return;
  //}

  //Do the preparing work and start a reorganization
  //prepare_and_start_one_reorganization(key.location, key.file, key.group, key.dataset, final_reorganization_type, 0, 0, NULL);

  //Set file under reorganization to prevent access to the reorganized file
  //update_orig_file_status(g_sds_dbs.sds_metadata_dbp, &key, ORIG_FILE_STATE__UNDER_REORGANIZATION);

  //Clear the read statistics 
  //reset_sds_file_read_count(g_sds_dbs.sds_metadata_dbp, &key, 0);
}
         

/* 
 * Add information of the reorganized file into metadata database 
 */
int handle_result_without_error(sds_job *job){
  log_msg("Job for file [%s] is finished and deleted from job list ! \n ", job->original_file_info.file_name);
  
  SDSMetadataDbKey         key   = SDS_METADATA_DB_KEY__INIT;
  SDSMetadataDbValue       value = SDS_METADATA_DB_VALUE__INIT;
  SDSMetadataDbValue       value2 = SDS_METADATA_DB_VALUE__INIT;

  ReadCount                rf_entry;

  //Read the orginal file record
  key.filename  = job->original_file_info.file_name;
  key.group     = job->original_file_info.group_name;
  key.dsetname  = job->original_file_info.dataset_name;
  key.datatype  = job->original_file_info.data_type;
  key.filetype  = job->original_file_info.file_type;
   
  
  //Read record
  read_metadata_record(g_sds_dbs.sds_metadata_dbp,  &key, &value);
  
  //Assign reorganized file into record
  SdsFile **index_files, *temp_index_file;
  SdsFile **reorg_files, *temp_reorg_file;
  int i, n;
  switch(job->job_type){
    case BUILD_INDEX:
      n = value.n_index_files + 1;
      index_files     = malloc(sizeof(SdsFile *));
      temp_index_file = malloc(sizeof(SdsFile));
      sds_file__init(temp_index_file);
      temp_index_file->filename = job->new_file_info.file_name;
      temp_index_file->group    = job->new_file_info.group_name;
      temp_index_file->dsetname = job->new_file_info.dataset_name;
      temp_index_file->filetype = job->new_file_info.file_type;
      temp_index_file->datatype = job->new_file_info.data_type;
      temp_index_file->ir_type  = job->job_subtype;
      temp_index_file->parameters = job->other_parameters;
      //printf("temp_index_file->parameters %s\n", temp_index_file->parameters);
      index_files[0] = temp_index_file;
      value.n_index_files = n;
      for(i = 1 ; i < n ; i++){
        index_files[i] = value.index_files[i];
        //free(value.index_files[i]);
      }
      value.index_files=index_files;
      break;
    case REORGANIZE_DATA:
      n = value.n_reorg_files + 1;
      reorg_files = malloc(sizeof(SdsFile *) * n);
      temp_reorg_file = malloc(sizeof(SdsFile));
      sds_file__init(temp_reorg_file);
      temp_reorg_file->filename = job->new_file_info.file_name;
      temp_reorg_file->group    = job->new_file_info.group_name;
      temp_reorg_file->dsetname = job->new_file_info.dataset_name;
      temp_reorg_file->filetype = job->new_file_info.file_type;
      temp_reorg_file->datatype = job->new_file_info.data_type;
      temp_reorg_file->ir_type  = job->job_subtype;
      temp_reorg_file->parameters = job->other_parameters;
      //printf("temp_reorg_file->parameters %s\n", temp_reorg_file->parameters);
      reorg_files[0] = temp_reorg_file;
      value.n_reorg_files  = n;
      for(i = 1 ; i < n ; i++)
        reorg_files[i] = value.reorg_files[i];
      value.reorg_files = reorg_files;
      break;
    default:
      log_msg("Unknow Job Type !");
  }
  
  //Update the status from ORIG_FILE_STATE__UNDER_REORGANIZATION to ORIG_FILE_STATE__UNDER_SERVICE
  value.orig_file_status = FILE_STATUS__IN_SERVICE;

  //Set all read count to zero
  read_count__init(&rf_entry);
  rf_entry.previous_interval        = 0;
  rf_entry.previous_400_interval    = 0;
  rf_entry.previous_10000_interval  = 0;
  rf_entry.life_time                = 0;
  value.read_count                  = &rf_entry;
  
  //Use zero right now
  value.owner_id                    = 0;
  value.group_id                    = 0;
  value.owner_bit                   = 0;
  value.group_bit                   = 0;
  value.other_bit                   = 0;

  log_msg("file [%s], group [%s], dataset [%s]", key.filename, key.group, key.dsetname);
  //Write new record back
  write_metadata_record(g_sds_dbs.sds_metadata_dbp,  &key, &value);
  //printf("type %d \n ", value.sds_file[0]->reorganization_type);
  //Read record
  read_metadata_record(g_sds_dbs.sds_metadata_dbp,  &key, &value2);
  printf("after writing  value2.n_index_files %d \n", value2.n_index_files);
  return 0;
}

/*
 *When an error happens in reorganization,  
 * change the status of orginal file back to ORIG_FILE_STATE__UNDER_SERVICE
 * Todo: Try to find and record the error information 
 */
int handle_result_with_error(sds_job *job){
  SDSMetadataDbKey     key = SDS_METADATA_DB_KEY__INIT;
  SDSMetadataDbValue   value = SDS_METADATA_DB_VALUE__INIT;
  ReadCount            rf_entry;

  //Read the orginal file record
  key.filename  = job->original_file_info.file_name;
  key.group     = job->original_file_info.group_name;
  key.dsetname  = job->original_file_info.dataset_name;
  key.datatype  = job->original_file_info.data_type;
  key.filetype  = job->original_file_info.file_type;
  
  
  read_metadata_record(g_sds_dbs.sds_metadata_dbp,  &key, &value);

  //Update the status from FILE_STATUS__NOT_IN_SERVICE to FILE_STATUS__IN_SERVICE
  value.orig_file_status = FILE_STATUS__IN_SERVICE;

  //Set all read count to zero
  read_count__init(&rf_entry);
  rf_entry.previous_interval        = 0;
  rf_entry.previous_400_interval    = 0;
  rf_entry.previous_10000_interval  = 0;
  rf_entry.life_time                = 0;
  value.read_count                  = &rf_entry;
  
  //Use zero right now
  value.owner_id                    = 0;
  value.group_id                    = 0;
  value.owner_bit                   = 0;
  value.group_bit                   = 0;
  value.other_bit                   = 0;

  //Write new record back
  write_metadata_record(g_sds_dbs.sds_metadata_dbp,  &key, &value);
  return 0;
}


/*
 * Update the status for each job recorded in reorganization_job_list
 */
void update_job_list_status(){
  int                  i, job_count;
  sds_job             *job;
  enum sds_job_status  job_state;

  
  log_msg("Job count in list is %d ", job_queue.job_count);
  job_count = job_queue.job_count;

  for(i = 0 ; i < job_count; i++){
    job = job_queue.jobs;
    //find the job status with check_job_status
    job_state = check_job_status(job);
    job->status = job_state;
    
    if(job_state == FINISHIED_WITHOUT_ERROR){
      //After job finishes witout error, 
      //Store the attribute into attribute data base
      //if (job->reorganization_type == SORT){
      //  store_attribute_data(job);   
      //}
      printf("Finished without error ! \n");
      // Call function handle_result_without_error
      // to update metadata and gather/store attribute
      handle_result_without_error(job);
      //Remove the job from job list
      LL_REMOVE(job, job_queue.jobs);
      job_queue.job_count--;
    }else if(job_state == FINISHIED_WITH_ERROR){
      //After job finishes witout error, 
      // call function handle_result_with_error to update metadata 
      // and gather/store attribute
      printf("Finished with error ! \n");
      handle_result_with_error(job);
      LL_REMOVE(job, job_queue.jobs);
      job_queue.job_count--;
    }
    job++;
  }
}

/* 
 *  Start a user specified reorganization.
 *  and send response to client 
 */
void start_user_specified_reorganization(client_t *client, ReorgStatus *status){
  SDSMetadataDbKey   key = SDS_METADATA_DB_KEY__INIT;
  SDSMetadataDbValue value;
  int                ret;
  int                i, n;
  
  log_msg("Start reorganization object");
  
  RequestReorgData *reorg_data;
  SdsObject        *sds_object;
  reorg_data = client->request->reorg_data;
  n = reorg_data->n_objects;

  for (i = 0; i < n ; i++){
    sds_object = reorg_data->objects[i];
    
    key.filename     = sds_object->filename;
    key.group        = sds_object->group;
    key.dsetname     = sds_object->dsetname;
    key.datatype     = sds_object->data_type;
    key.filetype     = sds_object->file_type;

    log_msg("  file [%s], group [%s], dataset [%s], file type [%d], data type [%d]", key.filename, key.group, key.dsetname, key.filetype, key.datatype);

    //Try to find is there any record for the file
    ret = read_metadata_record(g_sds_dbs.sds_metadata_dbp,  &key, &value);
    if(ret ==  DB_NOTFOUND){
      //No record exists, creat a new one
      log_msg("No record exits for %s. create one ", key.filename);
      db_create_empty_record(g_sds_dbs.sds_metadata_dbp, &key, FILE_STATUS__NOT_IN_SERVICE);
      if(reorg_data->reorg_type[i] != NONE_REORG){
        //Create an reorganization 
        prepare_and_start_one_reorganization(reorg_data->objects[i],    \
                                           REORGANIZE_DATA,             \
                                           reorg_data->reorg_type[i],   \
                                           reorg_data->reorg_cores[i],  \
                                           reorg_data->reorg_time_secs[i], \
                                           reorg_data->reorg_parameters[i]);
      }else if(reorg_data->index_type[i]!= NONE_INDEX){
        //Create a index
        log_msg("Bild index for %s ", key.filename);
        prepare_and_start_one_reorganization(reorg_data->objects[i],    \
                                             BUILD_INDEX,               \
                                             reorg_data->index_type[i], \
                                             reorg_data->index_cores[i], \
                                             reorg_data->index_time_secs[i], \
                                             reorg_data->index_parameters[i]);
      }
      status[i]=REORG_STATUS__ACCEPTED;
    }else{
      // Todo:
      // For a single object(file), we could build bitmap index and performan 
      // sorting at the same time. In this case, we could sort the data at first 
      // Then, build the bitmap index on the sorted dataset 
      // Current implementaion, assume that building bitmap index and sorting data are 
      // independently from each other
      
      //Start a job to reorganize the data 
      if(reorg_data->reorg_type[i] != NONE_REORG){
        //One record exists, check requested reorganization exists
        if(value.n_reorg_files > 0){
          if(check_ir_exist(reorg_data->reorg_type[i], value.reorg_files, value.n_reorg_files) == SDS_TRUE){
            reset_read_count(g_sds_dbs.sds_metadata_dbp, &key, 0);
            status[i]=REORG_STATUS__FINISH_WITHOUT_ERROR;
            log_msg("The requested organization exists !");
          }else{
            //No same reorganization exists and start one
            prepare_and_start_one_reorganization(reorg_data->objects[i], \
                                                 REORGANIZE_DATA,       \
                                                 reorg_data->reorg_type[i], \
                                                 reorg_data->reorg_cores[i], \
                                                 reorg_data->reorg_time_secs[i], \
                                                 reorg_data->reorg_parameters[i]);
            status[i]=REORG_STATUS__ACCEPTED;
          }
        }else{
          //No reorganization exists and start one 
          prepare_and_start_one_reorganization(reorg_data->objects[i],  \
                                               REORGANIZE_DATA,         \
                                               reorg_data->reorg_type[i], \
                                               reorg_data->reorg_cores[i], \
                                               reorg_data->reorg_time_secs[i], \
                                               reorg_data->reorg_parameters[i]);
          status[i]=REORG_STATUS__ACCEPTED;
        }
      }
      
      //Start a job to build index when necessary 
      if(reorg_data->index_type[i]!= NONE_INDEX){
        //One record exists, check requested reorganization exists
        if(value.n_index_files > 0){
          if(check_ir_exist(reorg_data->index_type[i], value.index_files, value.n_index_files) == SDS_TRUE){
            reset_read_count(g_sds_dbs.sds_metadata_dbp, &key, 0);
            log_msg("The requested index file exists !");
            status[i]=REORG_STATUS__FINISH_WITHOUT_ERROR;
          }else{
            //No same reorganization exists and start one
            prepare_and_start_one_reorganization(reorg_data->objects[i], \
                                                 BUILD_INDEX,           \
                                                 reorg_data->index_type[i], \
                                                 reorg_data->index_cores[i], \
                                                 reorg_data->index_time_secs[i], \
                                                 reorg_data->index_parameters[i]);
            status[i]=REORG_STATUS__ACCEPTED;
          }
        }else{
          //No reorganization exists and start one anyway
          prepare_and_start_one_reorganization(reorg_data->objects[i],  \
                                               BUILD_INDEX,             \
                                               reorg_data->index_type[i], \
                                               reorg_data->index_cores[i], \
                                               reorg_data->index_time_secs[i], \
                                               reorg_data->index_parameters[i]);
          status[i]=REORG_STATUS__ACCEPTED;
        }
      }
    }
  }
}

/*
 * The main function of reorganization manager thread
 * It is waked up by either a new client reorganization or 
 *  the timeout of function pthread_cond_timedwait   
 */
static void *reorganization_thread_function(void *ptr) {
  worker_t               *worker = (worker_t *)ptr;
  job_t                  *job;
  struct   timespec       ts;
  int                     ret;
  client_t               *client;
  int                     response_size;
  
  ClientResponse          response = CLIENT_RESPONSE__INIT;
  void                   *response_buf;
  ReorgStatus            *reorg_status;
  int                     reorg_status_n;
  ResponseReorgData       reorg_data = RESPONSE_REORG_DATA__INIT;
  ResponseAnalyData       analy_data = RESPONSE_ANALY_DATA__INIT;

  QueryStatus             job_status;
  size_t                  n_result_objects;
  SdsObject             **result_objects;
  char                   *result_data;


  while (1) {
    /* Wait until a new request coming or time out happens. */
    pthread_mutex_lock(&worker->workqueue->jobs_mutex);
    while (worker->workqueue->waiting_jobs == NULL) {
      maketimeout(&ts, monitor_interval);
      if(pthread_cond_timedwait(&worker->workqueue->jobs_cond, &worker->workqueue->jobs_mutex, &ts) == ETIMEDOUT){
        //if it is timeout, we need to update status of the
        // ongoing reorganization work. Also, we try to sponsor 
        // a new reorganization
        log_msg("Reorganization manager thread is invokded by timer ! ");
        update_job_list_status();
        evaluate_new_reorganization();
      }
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

    //When it comes here, we accepts a new request from client
    //Start a job to do it, depending on the type 
    //response =  CLIENT_RESPONSE__INIT;
    client = (client_t *)job->user_data;
    switch(client->request->type){
      case MESSAGE_TYPE__REORG:
        reorg_status_n = client->request->reorg_data->n_objects;
        reorg_status   = malloc(sizeof(ReorgStatus) * reorg_status_n);
        /* Start a user specified reorganization or index building */
        start_user_specified_reorganization(job->user_data, reorg_status);
        client = job->user_data;

        //Fill the data
        //reorg_data = RESPONSE_REORG_DATA__INIT;
        reorg_data.n_status                    = reorg_status_n;
        reorg_data.status                      = reorg_status;

        //Fill the status
        response.error_code                   = QUERY_STATUS__SUCCESSFUL;
        response.type                         = MESSAGE_TYPE__REORG;
        response.reorg_data                   = &reorg_data; 
        break;
      case MESSAGE_TYPE__ANALY:
        //Start a  job to run analysis 
        //It returns REORG_STATUS__FINISH_WITHOUT_ERROR and do nothing when the query of this query exists
        //Otherwise, it returns REORG_STATUS__ACCEPTED and starts a job the run the query.         //job_status = start_client_analyze_job(job->user_data, &n_result_objects,  result_objects,  result_data);

        //Fill the data
        //analy_data = RESPONSE_ANALY_DATA__INIT;
        analy_data.result_data       = result_data;
        analy_data.n_result_objects = n_result_objects;
        analy_data.result_objects    = result_objects;
        analy_data.status            = job_status;

        //Fill the status
        response.error_code                   = QUERY_STATUS__SUCCESSFUL;
        response.type                         = MESSAGE_TYPE__ANALY;
        response.analy_data                   = &analy_data;
        break;
      case MESSAGE_TYPE__TRACE:
        response.error_code                   = QUERY_STATUS__SUCCESSFUL;
        response.type                         = MESSAGE_TYPE__TRACE;
        //start to analyze the data
        response.trace_data                   = trace_log_analysis(job->user_data);
        /* if (response.trace_data != NULL) */ 
        /*     log_msg("response.trace_data->n_opt_hyperslab: %d", response.trace_data->n_opt_hyperslab); */
        break;
      default:
        log_msg("Unknow type of query to reorganization-manager-thread (ingoring it !)");
        response.error_code          = QUERY_STATUS__FAILED;
        response.type                = client->request->type;
        break;
    }
    
    //Package the data and send the result
    response_size                         = client_response__get_packed_size(&response);
    response_buf                          = malloc(response_size);
    client_response__pack(&response, response_buf);
    
    if(send(client->fd, response_buf, response_size, 0) == -1){
      log_msg("Unexpected error on send()\n");
    }
    
    shutdown(client->fd,2);
  }
  free(worker);
  pthread_exit(NULL);
  
  return 0;
}


int reorganization_manager_thread_init(workqueue_t *workqueue, int numWorkers) {
  int i;
  worker_t *worker;
  pthread_cond_t  blank_cond  = PTHREAD_COND_INITIALIZER;
  pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;
  
  /* Initialize the reorganization_job_list */
  initialize_reorganization_job_list(&job_queue);
        
  /*
   * Initialize the work queue between this thread and listern thread
   * Start a this thread 
   */
  if (numWorkers < 1) numWorkers = 1;
  memset(workqueue, 0, sizeof(*workqueue));
  memcpy(&workqueue->jobs_mutex, &blank_mutex, sizeof(workqueue->jobs_mutex));
  memcpy(&workqueue->jobs_cond,  &blank_cond, sizeof(workqueue->jobs_cond));
  
  for (i = 0; i < numWorkers; i++) {
    if ((worker = malloc(sizeof(worker_t))) == NULL) {
      perror("Failed to allocate all workers");
      return 1;
    }
    memset(worker, 0, sizeof(*worker));
    worker->workqueue = workqueue;
    if (pthread_create(&worker->thread, NULL, reorganization_thread_function,
                       (void *)worker)) {
      perror("Failed to start all worker threads");
      free(worker);
      return 1;
    }
    LL_ADD(worker, worker->workqueue->workers);
  }
  
  return 0;
}
