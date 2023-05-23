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
#include "attribute-db-access.h"
#include "sds-common.h"
#include "db-message.protoc.pb-c.h"
#include "reorganization-job.h"
#include "data-reorganizer.h"
#include "metadata-db-access.h"
#include "sds-error.h"


#include <stdlib.h>
#include <db.h>
#include <sys/stat.h>

extern sds_dbs_t  g_sds_dbs;

int attribute_db_write_new_record(DB *dbpp,SDSMetadataDbEntryKey *key, SdsAttributeValue *value){
  DBT       db_key, db_data;
  uint8_t  *key_buf,     *val_buf;
  int       key_buf_size, val_buf_size;
  int       ret;

  memset(&db_key,  0, sizeof(DBT));
  memset(&db_data, 0, sizeof(DBT));

  key_buf_size   = sds_metadata_db_entry_key__get_packed_size(key);
  val_buf_size   = sds_attribute_value__get_packed_size(value);

  key_buf        = malloc(key_buf_size);
  val_buf        = malloc(val_buf_size);

  sds_metadata_db_entry_key__pack(key, key_buf);
  sds_attribute_value__pack(value, val_buf);

  db_key.size  = key_buf_size;
  db_key.data  = key_buf;

  db_data.size = val_buf_size;
  db_data.data = val_buf;

  //printf("key size %d, buf size %d \n", key_buf_size, val_buf_size);

  //dbpp->put(dbpp, NULL, &db_key, &db_data, DB_NOOVERWRITE);
  ret = dbpp->put(dbpp, NULL, &db_key, &db_data, 0);

  dbpp->sync(dbpp, 0);

  free(key_buf);
  free(val_buf);
  return ret;
}


int attribute_db_read_new_record(DB *dbpp, SDSMetadataDbEntryKey *key, SdsAttributeValue *value){
  DBT                db_key, db_data;
  uint8_t           *key_buf;
  int                key_buf_size;
  SdsAttributeValue *value_temp;
  int                ret;



  memset(&db_key, 0, sizeof(DBT));
  memset(&db_data, 0, sizeof(DBT));

  //printf("or= %s \n", orginal_file_name);    
  key_buf_size   = sds_metadata_db_entry_key__get_packed_size(key);
  key_buf        = malloc(key_buf_size);
  if (key_buf == NULL){
    log_sys("Allocate memory for DB record write fails.");
  }

  sds_metadata_db_entry_key__pack(key, key_buf);
  db_key.data = key_buf;
  db_key.size = key_buf_size;

  /* Let DB allocation memory for us */
  db_data.flags = DB_DBT_MALLOC;
  ret = dbpp->get(dbpp, NULL, &db_key, &db_data, 0);
  
  /*
   * The key is not found 
   * Todo:  More errors like DB_LOCK_DEADLOCK need to be handled
   */
  if (ret == DB_NOTFOUND){
    free(key_buf);
    return DB_NOTFOUND;
  }



  value_temp = sds_attribute_value__unpack(NULL, db_data.size, db_data.data);
  
  *value = *value_temp;

  printf("row =%d, col=%d , db_data.size=%d, n table = %d, sizeof(db_data.data)=%d \n", value_temp->sort_attribute->row, value_temp->sort_attribute->col, db_data.size, value_temp->sort_attribute->n_table, sizeof(db_data.data));

  //printf("row =%d, col=%d , size=%d \n", value->row, value->col, sizeof(value->value));

  free(key_buf);
  return ret;
}


int store_attribute_data(reorganization_job_t *job){
  char                         attribute_file_name[MAX_FILE_NAME_LENGTH+MAX_LOCATION_NAME_LENGTH];

  SDSMetadataDbEntryKey        key            = SDS_METADATA_DB_ENTRY_KEY__INIT;
  SdsAttributeValue            value          = SDS_ATTRIBUTE_VALUE__INIT; 
  SdsSortAttribute             sort_attribute = SDS_SORT_ATTRIBUTE__INIT; 
  FILE                        *fd;
  int                          ret;
  float                        min, max;
  unsigned long long           offset_start, offset_end;
  int                          i;

  get_job_attribute_file_name(attribute_file_name,job);
  log_msg("Storing attribute %s ! ", attribute_file_name);

 

  key.location = job->original_file_info.file_path;
  key.file     = job->original_file_info.file_name;
  key.group    = job->original_file_info.group_name;
  key.dataset  = job->original_file_info.dataset_name;

  value.reorganization_type = job->reorganization_type;
  if (job->reorganization_type ==  SORT ){
    sort_attribute.col   = 4;
    sort_attribute.attribute_file_name = attribute_file_name;
    sort_attribute.row        = job->number_of_cores;
  }else{
    sort_attribute.col   = 0;
  }
  

  /*
  fd =  fopen(attribute_file_name, "r");
  if(fd == NULL){
    log_quit("Error open the attribute file ! \n");
  }

  sort_attribute.table = (SdsSortAttributeTableRecord **)malloc(sizeof(SdsSortAttributeTableRecord *)*(sort_attribute.row));
  sort_attribute.n_table = sort_attribute.row;
  for (i = 0 ; i < sort_attribute.row; i++){
    sort_attribute.table[i] =  (SdsSortAttributeTableRecord *)malloc(sizeof(SdsSortAttributeTableRecord));
    sds_sort_attribute_table_record__init(sort_attribute.table[i]);
    fscanf(fd, "%f %f %llu %llu", &min, &max, &offset_start, &offset_end);
    sort_attribute.table[i]->min = min;
    sort_attribute.table[i]->max = max;
    sort_attribute.table[i]->offset_start = offset_start;
    sort_attribute.table[i]->offset_end   = offset_end;
    //printf("%f %f %llu %llu\n", min, max, offset_start, offset_end);
  }
  */ 

  value.sort_attribute = &sort_attribute;
 
  ret = attribute_db_write_new_record(g_sds_dbs.sds_reorg_file_attri_dbp, &key, &value);
  if(ret != 0){
    log_sys("Error happens in storeing attribute !");
  }
  

}


