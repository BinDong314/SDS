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
#include "server-connector.h"
#include "sds-common.h"
#include "sds-query.h"
#include "sds-error.h"


int IsSDSfile(char *dirname){
  //if dir name containing SDS-ROOT_DIR at the end, it is reorganized file
  int    ret = 1, name_len, root_len,  i;
  char  *root_name="SDS-ROOT-DIR";
  name_len = strlen(dirname);
  root_len = strlen(root_name);  
  
  if(dirname[name_len] == '/'){
    name_len = name_len - 1;
  }
  
  for(i = 0 ; i < root_len; i++){
    if(dirname[name_len-root_len+i]  != root_name[i]){
      ret = 0;
      break;
    }
  }
  
  return ret;
}


void *sds_client_query(const char *dir, const char *fname, const char *group, const char *dset, sds_file_info_t *sds_metadata){
  //128.55.44.227   edimom14	nid02433
  char *server_address = "128.55.44.227";
  inquire_sds_server(dir, fname,  group, dset, SDS_CLIENT_QUERY__QUERY_TYPE__QUERY_METADATA_QUERY, 0, 0, 0, sds_metadata, NULL, server_address, NULL);
}

int inquire_sds_server(const char *location, const char *file_name,  const char *group, const char *dataset_name, int inquire_type, int reorganization_type, int cores,  int time_secs,  sds_file_info_t *sds_metadata, int *reorganization_status, char *host_ip, char *extended_paramters){
  int                    sockfd=0;
  struct  hostent       *he;
  struct  sockaddr_in    their_addr;
  char                  *hostname; // = "10.1.68.170"; //="10.128.16.155";
  int                    port=90001;
 
  SdsClientQuery         query = SDS_CLIENT_QUERY__INIT;
  int                    query_len;
  void                  *query_buf; 
  uint8_t               *response_data; 
  int                    response_size;
  SdsServerResponse     *response;
  int                    ret;
  int                    j;
  //int                    reorganization_type_wanted = INDEX;


  hostname = (char *)malloc(strlen(host_ip) + 1);
  strcpy(hostname, host_ip);

  // printf("host _ip %s \n", host_ip);

  response_data =  malloc( sizeof(uint8_t) * MAX_MESSAGE_LENGTH);
        
  if((he= gethostbyname(hostname)) == NULL){
    printf("Error with gethostbyname()\n");
    return -1;/*OTHERS*/
  }
 
  if((sockfd = socket(AF_INET,SOCK_STREAM, 0)) == -1){
    printf("Error with socket()\n");
    return -1;/*UNKNOWN*/
  }
 
  their_addr.sin_family = AF_INET;
  their_addr.sin_port   = htons(port);
  their_addr.sin_addr   = *((struct in_addr *)he->h_addr);
  memset(their_addr.sin_zero, 0, sizeof(their_addr.sin_zero));
 
  if(connect(sockfd,(struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1){
    /*printf("Error with connect()\n");*/
    printf("Not able to establish a communication with the SDS server!\n");
    return -1;/*UNKNOWN*/
  }

  query.file_path     = malloc(MAX_LOCATION_NAME_LENGTH);
  query.file_name     = malloc(MAX_FILE_NAME_LENGTH);
  query.group_path    = malloc(MAX_GROUP_NAME_LENGTH);
  query.dataset_name  = malloc(MAX_DATASET_NAME_LENGTH);

  strcpy(query.file_path, location);
  strcpy(query.file_name, file_name);
  strcpy(query.group_path, group);
  strcpy(query.dataset_name, dataset_name);
  query.message_type = inquire_type;
  if(inquire_type == SDS_CLIENT_QUERY__QUERY_TYPE__QUERY_REORGANIZATION_START){
    query.has_reorganization_type_wanted = 1;
    query.reorganization_type_wanted = reorganization_type;
          
    query.has_reorganization_cores_num = 1;
    query.reorganization_cores_num     = cores;

    query.has_reorganization_time_secs = 1;
    query.reorganization_time_secs     = time_secs;

    //query.has_reorganization_paramters = 1;
    query.reorganization_paramters = malloc(MAX_DATASET_NAME_LENGTH); 
    strcpy(query.reorganization_paramters, extended_paramters);
  }

  query_len = sds_client_query__get_packed_size(&query);
  query_buf = malloc(query_len);
  sds_client_query__pack(&query, query_buf);
        
  if(send(sockfd, query_buf, query_len, 0) == -1){
    printf("Error with send()\n");
    return -1;
  }
        
  memset(response_data, 0, sizeof(uint8_t) * MAX_MESSAGE_LENGTH);
  response_size = recv(sockfd, response_data, sizeof(uint8_t) * MAX_MESSAGE_LENGTH, 0);
  if(response_size == -1){
    printf("Error with recv()\n");
    return -1;
  }
        
  //printf("Max size %d, Reponse size: %d \n", MAX_MESSAGE_LENGTH, response_size);
  response = sds_server_response__unpack(NULL, response_size, response_data);
	
  if (response->error_code == SDS_SERVER_RESPONSE__QUERY_ERR_CODE__NO_REORGANIZATION ){
    sds_metadata->error_code = NO_REORGANIZATION;
    return response->error_code;
  }

  if(response->error_code == SDS_SERVER_RESPONSE__QUERY_ERR_CODE__NO_ERROR){
    ret = response->error_code;
    sds_metadata->error_code = HAS_REORGANIZATION;
    strcpy(sds_metadata->sds_file_location, response->reorganized_file_path);
    strcpy(sds_metadata->sds_file_name,     response->reorganized_file_name);
    strcpy(sds_metadata->sds_dataset_name,  response->reorganized_dataset_name);
    strcpy(sds_metadata->sds_group,  response->reorganized_gorup_path);

    if (response->attribute != NULL){
      sds_metadata->attribute = (sds_attribute_value_t *)malloc(sizeof(sds_attribute_value_t));
      sds_metadata->attribute->reorganization_type = response->attribute->reorganization_type;
      sds_metadata->attribute->sort_attribute = (sds_sort_attribute_t *)malloc(sizeof(sds_sort_attribute_t));
      sds_metadata->attribute->sort_attribute->row   = response->attribute->sort_attribute->row;
      sds_metadata->attribute->sort_attribute->col   = response->attribute->sort_attribute->col;
      //printf("Attribute name %s \n ", response->attribute->sort_attribute->attribute_file_name);
      sds_metadata->attribute->sort_attribute->attribute_file_name = response->attribute->sort_attribute->attribute_file_name;
    }
  }
        
  ret                         = response->error_code;
  sds_metadata->sds_reor_type = response->reorganization_type;
  sds_metadata->reor_status   = response->reorganization_status;
        
  //printf("type %d \n", sds_metadata->sds_reor_type);
  shutdown(sockfd,2);
  free(query.file_path);
  free(query.file_name);
  free(query.group_path);
  free(query.dataset_name);
  free(query_buf);
        
  return ret;
}


int SDS_Socket_start(){
  int                    sockfd=0;
  struct  hostent       *server_he;
  struct  sockaddr_in    server_addr;
  char                  *server_ip = SERVER_ADDRESS  ; 
  int                    server_port = SERVER_PORT;
  
  if((server_he= gethostbyname(server_ip)) == NULL){
    log_quit("Error with gethostbyname() at SDS Client");
  }
  
  if((sockfd = socket(AF_INET,SOCK_STREAM, 0)) == -1){
    log_quit("Error with socket() at SDS Client");
  }
  
  server_addr.sin_family = AF_INET;
  server_addr.sin_port   = htons(server_port);
  server_addr.sin_addr   = *((struct in_addr *)server_he->h_addr);
  memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));
  
  if(connect(sockfd,(struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1){
    log_quit("Not able to establish a communication with the SDS server!\n");
  }

  return sockfd;
}

int SDS_Socket_recv(int sockfd, void *buf, int length){
  int size;
  size = recv(sockfd, buf, length, 0);
  if(size == -1)
    log_quit("Error with recv() at SDS Client.");
  
  return size;
}

int SDS_Socket_sent(int sockfd, void *buf, int length){
  if(send(sockfd, buf, length, 0) == -1)
    log_quit("Error with send() data from SDS Client to SDS Server");
  return 0;
}

int SDS_Socket_stop(int sockfd){
  shutdown(sockfd,2);
}

/*
void SDS_Fill_query(CondTreeNode *new_node, SDS_Variable *var){
  //copy_str(new_node->file_path,    var->file_path);
  //copy_str(new_node->file_name,    var->filename);
  //copy_str(new_node->group_path,   var->group);
  //copy_str(new_node->dataset_name, var->dsetname);
  if(var->filename != NULL)
    new_node->file_name  = strdup(var->filename);
  if(var->group != NULL)
    new_node->group_path = strdup(var->group);
  if(var->dsetname != NULL)
    new_node->dataset_name = strdup(var->dsetname);    
  
  printf("fill %s of %s \n", new_node->dataset_name, var->dsetname);
  if(var->left_range != NULL){
    new_node->left_range = malloc(sizeof(Range));
    range__init(new_node->left_range);
    new_node->left_range->has_type = 1;
    new_node->left_range->type  = var->left_range->type;
    new_node->left_range->has_value = 1;
    new_node->left_range->value = var->left_range->value; 
  }

  if(var->right_range != NULL){
    new_node->right_range = malloc(sizeof(Range));
    range__init(new_node->right_range);
    new_node->right_range->has_type = 1;
    new_node->right_range->type  = var->right_range->type;
    new_node->right_range->has_value = 1;
    new_node->right_range->value = var->right_range->value; 
  }
  
  new_node->has_combine_type = 1;
  new_node->combine_type = var->combine_type;
  
  new_node->has_file_type = 1;
  new_node->file_type = var->file_type;
  
  new_node->has_data_type = 1;
  new_node->data_type = var->data_type; 
  
  new_node->has_node_type = 1;
  new_node->node_type = var->tree_node_type;
}
*/
/*
void SDS_Send_query(SDS_Variable **list_pre_order, int size, SDS_Query_status status){
  int                    i, sockfd;
  SdsClientRequest       query = SDS_CLIENT_REQUEST__INIT;
  int                    q_len,   r_len;
  void                  *q_buf,  *r_buf; 
  SdsServerResponse     *response;
  
  //Start a socket
  sockfd = SDS_Socket_start();

  //Fill the query
  query.type     =  SDS_CLIENT_REQUEST__REQUEST_TYPE__QUERY; 
  query.n_query  =  size;
  query.query    =  malloc(sizeof(CondTreeNode *) * size);
  
  CondTreeNode *new_node;
  for(i = 0 ; i < size; i++){
    new_node = malloc(sizeof(CondTreeNode));
    cond_tree_node__init(new_node);
    if(list_pre_order[i] != NULL){
      SDS_Fill_query(new_node, list_pre_order[i]);
      printf("Fill %s \n", new_node->dataset_name);
    }
    query.query[i]=new_node;
  }
  
  q_len = sds_client_request__get_packed_size(&query);
  printf("request size %d \n", q_len);
  q_buf = malloc(q_len);
  sds_client_request__pack(&query, q_buf);
  //Send to SDS Server
  SDS_Socket_sent(sockfd, q_buf,    q_len);

  r_buf=  malloc(MAX_MESSAGE_LENGTH);
  memset(r_buf, 0, MAX_MESSAGE_LENGTH);
  
  //Receive from SDS Server
  //r_len = SDS_Socket_recv(sockfd, r_buf, MAX_MESSAGE_LENGTH);
  //response = sds_server_response__unpack(NULL, r_len, r_buf);
  
  //if(response->error_code == SDS_SERVER_RESPONSE__QUERY_ERR_CODE__NO_ERROR){
  //  printf("Response received at  Client \n");
  //  return 0;
  //}
  //Handle the results.


  //Stop the socket
  SDS_Socket_stop(sockfd);

  //Release all memory
  for(i = 0 ; i < size; i++){
    if(query.query[i] != NULL){
      new_node = query.query[i];
      if(new_node->file_path != NULL)
        free(new_node->file_path);
      if(new_node->file_name != NULL)
        free(new_node->file_name);
      if(new_node->group_path != NULL)
        free(new_node->group_path);
      if(new_node->dataset_name != NULL)
        free(new_node->dataset_name);
    }
  }
  free(query.query);
  free(q_buf);
  free(r_buf);
}
*/
