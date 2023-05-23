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
#include "reorgnize.h"
#include "server-connector.h"

//Convert time hh:minitue:secons to decimal seconds 
//TODO: checnk the valid of string: time
void SDS_Fill_reorg(ReorgRequest *new_node, SDS_Object *var, int type, int cores, int time, char *argv){
  if(var->filename  != NULL)
    new_node->file_name  = strdup(var->filename);
  if(var->group != NULL)
    new_node->group_path = strdup(var->group);
  if(var->dsetname != NULL)
    new_node->dataset_name = strdup(var->dsetname);  

  new_node->has_reorganization_type_wanted = 1;
  new_node->reorganization_type_wanted = type;
  
  new_node->has_reorganization_cores_num = 1;
  new_node->reorganization_cores_num = cores;
  
  new_node->has_reorganization_time_secs = 1;
  new_node->reorganization_time_secs = time;
  
  if(argv != NULL)
    new_node->reorganization_paramters = strdup(argv);
  
}


int  SDS_Var_start_reorg(SDS_Object **vars, int *type, int *cores, int *time, char *argv[], int size,   SDS_Reorg_status status){
  int                    sockfd, i;
  SdsClientRequest       query = SDS_CLIENT_REQUEST__INIT;
  int                    q_len,   r_len;
  void                  *q_buf,  *r_buf; 
  SdsServerResponse     *response;
  
  //Start a socket
  sockfd = SDS_Socket_start();
  
  //Fill the query
  query.type     =  SDS_CLIENT_REQUEST__REQUEST_TYPE__REORG;
  query.n_reorg  =  size;
  query.reorg    =  malloc(sizeof(ReorgRequest *) * size);
  
  ReorgRequest *new_node = NULL;
  int           sec;
  for(i = 0 ; i < size; i++){
    new_node = malloc(sizeof(ReorgRequest));
    reorg_request__init(new_node);
    SDS_Fill_reorg(new_node, vars[i], type[i], cores[i], time[i], argv[i]);
    //printf("%d %d %d %d %s \n", vars[i], type[i], cores[i], time[i], argv[i] );
    query.reorg[i]=new_node;
  }
  
  q_len = sds_client_request__get_packed_size(&query);
  q_buf = malloc(q_len);
  sds_client_request__pack(&query, q_buf);
  
  //Send to SDS Server
  SDS_Socket_sent(sockfd, q_buf,  q_len);
  
  r_buf=  malloc(MAX_MESSAGE_LENGTH);
  memset(r_buf, 0, MAX_MESSAGE_LENGTH);
  
  //Receive from SDS Server
  //r_len = SDS_Socket_recv(sockfd, r_buf, MAX_MESSAGE_LENGTH);
  //response = sds_server_response__unpack(NULL, r_len, r_buf);
  //if(response->error_code == SDS_SERVER_RESPONSE__QUERY_ERR_CODE__NO_ERROR){
  //  printf("Response received at  Client \n");
  //  return 0;
  //}
  
  //Stop the socket
  SDS_Socket_stop(sockfd);

  //Release all memory
  for(i = 0 ; i < size; i++){
    if(query.reorg[i] != NULL){
      new_node = query.reorg[i];
      free(new_node->file_path);
      free(new_node->file_name);
      free(new_node->group_path);
      free(new_node->dataset_name);
      free(new_node->reorganization_paramters);
    }
  }
  free(query.reorg);
  free(q_buf);
  free(r_buf);
}

