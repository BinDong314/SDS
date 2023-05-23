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
 * This file defines the SDS Query interface to init/run/finalize a query tree
 * Author: Bin Dong <dbin at lbl.gov >
 * Copyright 2015 the Regents of the University of California
 * 
 */

#include "sds-query.h"
#include "sds-common.h"
//#include "sds-client-communicator.h"

//ROOT directory to store the reorganized files and temporary resutls 
char        client_sds_root_path[MAX_FILE_NAME_LENGTH];
//Is this a cluster version
SDS_Bool    client_cluster_version;
//Server IP
char        server_ip[MAX_FILE_NAME_LENGTH];
//Server Port
int         server_port;

char        ini_file[MAX_FILE_NAME_LENGTH] = "./sds.conf";
//Create a query_handle based on a pre-built query tree 
//The comm is like mpi comm:  local and global 
SDS_Query_handle *SDS_Query_init(SDS_Query_tree *root, SDS_Query_comm comm){
  SDS_Query_handle *handle;
  if(root == NULL){
    log_quit("SDS_Query_tree in NULL in SDS_Query_init!");
  }
  handle = malloc(sizeof(SDS_Query_handle));
  handle->root = root;
  handle->comm = comm;
  parse_client_file(ini_file);

  return handle;
}

//This function is called by SDS_Query_run to evaluate the tree 
//from bottom-up (post) order.
SDS_Collection  *SDS_Query_run_evaluate_tree(SDS_Query_tree *root, SDS_Query_comm comm){
  //Some operator's right tree is empty
  if(root == NULL){
    return NULL;
  }

  //leaf node return its collection
  if(root->left == NULL && root->right == NULL)
    return root->collection;
  
  SDS_Collection  *left_coll, *right_coll;
  left_coll  = SDS_Query_run_evaluate_tree(root->left, comm);
  right_coll = SDS_Query_run_evaluate_tree(root->right, comm);
  int *temp_index;
  //stop at each non-leaf node for evalution
  switch(root->operator){
    case SDS_SELECT:
      //log_msg("Evaluating query tree, node type SELECT ...  ! \n");
      root->collection = SDS_Collection_select(left_coll, root->operand, comm);
      break;
    case SDS_PROJECT:
      root->collection = SDS_Collection_project(left_coll, root->operand, comm);
      break;
    case SDS_JOIN:
      *temp_index = root->operand;
      root->collection = SDS_Collection_join(left_coll, temp_index[0], right_coll,  temp_index[1], comm);
      break;
    case SDS_UDF:
      root->collection = SDS_Collection_udf(left_coll, root->operand, comm);
      break;
    case SDS_HISTGRAM:
      root->collection = SDS_Collection_histgram(left_coll, root->operand, comm);
      break;
    default:
      log_quit("Unknwn operator type %d in  SDS_Query_run_evaluate_tree()");
      break;
  }
  //Release the memory
  SDS_Collection_release_buffer(left_coll);
  SDS_Collection_release_buffer(right_coll);
  return root->collection;
}

//Get the result collection 
SDS_Collection     *SDS_Query_get_result(SDS_Query_handle     *handle){
  return handle->result;
}

//Run a run at client side
int SDS_Query_run(SDS_Query_handle   *handle){
  SDS_Query_tree         *root =  handle->root;
  handle->result = SDS_Query_run_evaluate_tree(root,  handle->comm);
  return 1;
}

//Run the query_tree on a remote server (SDS Server)
//Rightnow, it supports a single node
//To do: run the query_tree on a remmote cluster.
//       and let SDS Server to start a batch job to run it  
int SDS_Query_rrun(SDS_Query_handle   *handle){
  SDS_Query_tree         *root =  handle->root;
  int                     i;

  handle->result = SDS_Remote_analyze(root);
  //Read all the data into memory
  for(i = 0; i < handle->result->object_count; i++)
    SDS_Object_read_all_data(handle->result->object_array[i], handle->comm);

  return 1;
}

//Release the query handle 
int SDS_Query_finalize(SDS_Query_handle   *handle){
  //To free data in object of the result collection and the collecion itself
  SDS_Collection_finalize(handle->result);
  free(handle);
}

