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
#ifndef __SDS_CLIENT_H__
#define __SDS_CLIENT_H__

#include "message.protoc.pb-c.h" 
//#include "db-message.protoc.pb-c.h"
#include "sds-common.h"


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
 
#include <getopt.h>



typedef struct  sds_sort_attribute_table_record
{
  float      min;
  float      max;
  uint64_t   offset_start;
  uint64_t   offset_end;
}sds_sort_attribute_table_record_t ;


typedef struct  sds_sort_attribute
{
  char                               *attribute_file_name;

  int32_t                             row;
  int32_t                             col;
  sds_sort_attribute_table_record_t **table;
}sds_sort_attribute_t;


typedef struct  sds_attribute_value
{
  int32_t               reorganization_type;
  sds_sort_attribute_t *sort_attribute;
}sds_attribute_value_t;


enum responsecode{
  NO_REORGANIZATION      = 0,
  HAS_REORGANIZATION     = 1,
};

typedef struct sds_file_info{
  int32_t                    error_code;      //0: NO_REORGANIZATION and 1: HAS_REORGANIZATION
  int32_t                    sds_reor_type;   //Reorganization type: sorting , indexing, transposing...
  int32_t                    sds_reor_subtype; //Sub type for each reorganization. For example, transposing can be reorganized based x, or y, or z. 
  char                      *sds_file_location;
  char                      *sds_file_name;
  char                      *sds_group;
  char                      *sds_dataset_name;
  int32_t                    reor_status;
  sds_attribute_value_t     *attribute;
}sds_file_info_t;

typedef struct SDS_Query_status{
  int32_t                    error_code;      //0: NO_REORGANIZATION and 1: HAS_REORGANIZATION
  int32_t                    sds_reor_type;   //Reorganization type: sorting , indexing, transposing...
  int32_t                    sds_reor_subtype; //Sub type for each reorganization. For example, transposing can be reorganized based x, or y, or z. 
  char                      *sds_file_location;
  char                      *sds_file_name;
  char                      *sds_group;
  char                      *sds_dataset_name;
  int32_t                    reor_status;
  sds_attribute_value_t    **attribute;
}SDS_Query_status;





void *sds_client_query(const char *dir, const char *fname, const char *group, const char *dset, sds_file_info_t *sds_metadata);
int inquire_sds_server(const char *location, const char *file_name,  const char *group, const char *dataset_name, int inquire_type, int reorganization_type, int cores, int time_secs,  sds_file_info_t *sds_metadata, int *reorganization_status, char *host_ip, char *extended_paramters);
int IsSDSfile(char *dirname);

int SDS_Socket_start();
int SDS_Socket_recv(int sockfd, void *buf, int length);
int SDS_Socket_sent(int sockfd, void *buf, int length);
int SDS_Socket_stop(int sockfd);

#endif

