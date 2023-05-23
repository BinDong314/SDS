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
 * This file defines the functions to run "analysis" request from client
 * Author: Bin Dong <dbin at lbl.gov >
 * Copyright 2015 the Regents of the University of California
 * 
 */

#include "query-db-access.h"

int start_client_analyze_job(client_t *client, int *n_result_object, SdsObject **result_objects,  char *result_data){
  //Check the query database first to see any results existing 
  sds_job            *new_job;
  ClientRequest      *request = client->request;
  RequestAnalyData   *key = request->analy_data;
  ResponseAnalyData   value = REQUEST_ANALY_DATA__INIT;
  int                 ret;
  //Before start the job, check the query data base to see 
  //any previous results there 
  ret = read_query_record(g_sds_dbs.sds_query_dbp, key, &value);
  if(ret == 0){
    //Previous results exists
    *n_result_object =  value.n_result_objects;
    result_objects   =  value.result_objects;
    result_data      =  value.result_data;
    return REORG_STATUS__FINISH_WITHOUT_ERROR;
  }else{
    //No running histrogy for the query,
    //One way : start a job to run it (parallel) and then record 
    //          the results in query database
    //Another way: call the sds client library to run it as single thread.
    //Here, we choose second way. call SDS_Query_run and return the results to clients

    
    //Run the query and wait until it finishs 
    run_analye_job(key, n_result_object, result_objects, result_data);

    //Write the results to query_db_database 
    value.n_result_objects = *n_result_object;
    value.result_objects   =  result_objects;
    value.result_data      =  result_data;
    write_query_record(g_sds_dbs.sds_query_dbp, key, &value);

    return REORG_STATUS__FINISH_WITHOUT_ERROR;
  }

  
}

//On a single node, run the analyze job with the same thread
//This depends on the SDS Client library
void run_analye_job(RequestAnalyData   *analy_data){
  SDS_Query_tree     **query_tree_serialized;
  SDS_Query_tree      *q_tree;
  SDS_Query_tree      *q_tree_node;
  SDS_Query_handle    *query_handle;
  int                  n = 0;
  SDS_Collection      *c_result;
  
  //Get the serizlied tree from RequestAnalyData
  query_tree_serialized = malloc(analy_data->n_query_tree * sizeof(SDS_Query_tree  *));
  
  for (i = 0 ; i < analy_data->n_query_tree; i++){
    q_tree_node =  malloc(sizeof(SDS_Query_tree));
    query_fill_query_tree_node_reverse(q_tree_node, analy_data->query_tree[i]);
    query_tree_serialized[i] = q_tree_node;
  }

  //Re-construct the tree from serilized tree
  q_tree = SDS_Query_tree_deserialize(query_tree_serialized, &n);
  
  //Init a query
  query_handle = SDS_Query_init(q_tree, MPI_COMM_WORLD);
  //Run the query
  SDS_Query_run(query_handle);
 //Print the results
  c_result = SDS_Query_get_result(query_handle);
 
  //Handle the result
  //Store the results into 
  SDS_Collection_save_results(c_result, 1, 1);
  
  *n_result_object = c_result->object_count;
  result_objects = malloc(c_result->object_count * sizeof(SdsObject *));
  SdsObject *new_obj;
  for(i = 0 ; i < c_result->object_count; i++){
    new_obj = malloc(SdsObject);
    sds_object__init(new_obj);
    query_fill_object(new_obj, c_result->object_array[i]);
    result_objects[i] = new_obj;
  }
  
}
