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
 * This file is the interfaces for query Berkeley DB
 * 
 *  Author: Bin Dong <dbin at lbl.gov >
 *  Copyright 2015 the Regents of the University of California
 * 
 */
#include "query-db-access.h"

extern sds_dbs_t  g_sds_dbs;


// Open existing Table for query
int open_sds_query_db(){
  open_db(SDS_QUERY_TABLE_NAME,    &(g_sds_dbs.sds_query_dbp),    DB_CREATE|DB_THREAD);
  return 1;
}

// Create Table for query
int create_sds_query_db(){
  open_db(SDS_QUERY_TABLE_NAME,    &(g_sds_dbs.sds_query_dbp),    DB_CREATE|DB_THREAD);
  return 1;
}

//Close query db
int close_sds_query_db(sds_dbs_t *sds_dbs){
  int ret;
  ret = close_db(sds_dbs->sds_query_dbp);
  if (ret != 0)
    fprintf(stderr, "SDS Query database close failed: %s\n",db_strerror(ret));
  return ret;
}


int write_query_record(DB *dbpp, RequestAnalyData *key, ResponseAnalyData *value){
  DBT       db_key, db_data;
  uint8_t  *key_buf,     *val_buf;
  int       key_buf_size, val_buf_size;

  memset(&db_key, 0, sizeof(DBT));
  memset(&db_data, 0, sizeof(DBT));

  key_buf_size   = request_analy_data__get_packed_size(key);
  val_buf_size   = response_analy_data__get_packed_size(value);

  key_buf        = (uint8_t  *)malloc(key_buf_size);
  val_buf        = (uint8_t  *)malloc(val_buf_size);

  request_analy_data__pack(key, key_buf);
  response_analy_data__pack(value, val_buf);


  db_key.size  = key_buf_size;
  db_key.data  = key_buf;

  db_data.size = val_buf_size;
  db_data.data = val_buf;

  //printf("key size %d, buf size %d \n", key_buf_size, val_buf_size);
  //dbpp->put(dbpp, NULL, &db_key, &db_data, DB_NOOVERWRITE);
  dbpp->put(dbpp, NULL, &db_key, &db_data, 0);

  free(key_buf);
  free(val_buf);
  return 1;
}


int read_query_record(DB *dbpp, RequestAnalyData *key, ResponseAnalyData *value){
  DBT                      db_key, db_data;
  uint8_t                 *key_buf;
  int                      key_buf_size;
  ResponseAnalyData       *value_temp;
  int                      ret;
  
  memset(&db_key, 0, sizeof(DBT));
  memset(&db_data, 0, sizeof(DBT));

  //printf("or= %s \n", orginal_file_name);    
  key_buf_size   = request_analy_data__get_packed_size(key);
  key_buf        = (uint8_t  *)malloc(key_buf_size);
  if (key_buf == NULL){
    log_sys("Allocate memory for DB record write fails.");
  }
  
  //Package the key with the function generaged by protobuf-c
  request_analy_data__pack(key, key_buf);
  db_key.data = key_buf;
  db_key.size = key_buf_size;

  /* Let DB allocation memory for us */
  db_data.flags = DB_DBT_MALLOC;
  ret = dbpp->get(dbpp, NULL, &db_key, &db_data, 0);
  
  /*
   * The key is not found 
   * Todo:  More errors like DB_LOCK_DEADLOCK need to be handled
   * https://docs.oracle.com/cd/E17276_01/html/programmer_reference/program_errorret.html
   */
  if (ret == DB_NOTFOUND){
    free(key_buf);
    return DB_NOTFOUND;
  }
  
  if(ret > 0){
    log_quit("Berkeley DB has  system error !");
    return ret;
  }

  //Assume ret is zero now
   value_temp  =  response_analy_data__unpack(NULL, db_data.size, db_data.data);
  *value       = *value_temp;
  
  
  free(db_data.data);
  free(key_buf);
  return ret;
}






