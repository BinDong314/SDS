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
 * This file is the interfaces for metadata Berkeley DB
 * 
 *  Author: Bin Dong <dbin at lbl.gov >
 *  Copyright 2015 the Regents of the University of California
 * 
 */

#include "metadata-db-access.h"

extern sds_dbs_t  g_sds_dbs;



/* Open existing Table for SDS Metadta, Reorganized File Attribute and  IO Pattern  */
int open_sds_metadata_db(){
  open_db(SDS_METADATA_TABLE_NAME, &(g_sds_dbs.sds_metadata_dbp), DB_CREATE|DB_THREAD);
  return 1;
}

/* Create Table for SDS Metadata, Reorganized File Attribute and  IO Pattern*/
int create_sds_metadata_db(){
  open_db(SDS_METADATA_TABLE_NAME, &(g_sds_dbs.sds_metadata_dbp), DB_CREATE|DB_THREAD);
  return 1;
}

int close_sds_metadata_db(){
  int ret;
  ret = close_db(g_sds_dbs.sds_metadata_dbp);
  if (ret != 0)
    fprintf(stderr, "SDS Metadata database close failed: %s\n",db_strerror(ret));
  
  return ret;
}

int delete_metadata_record(DB *dbpp, SDSMetadataDbKey *key){
  DBT       db_key, db_data;
  uint8_t  *key_buf;
  int       key_buf_size, ret;

  memset(&db_key, 0, sizeof(DBT));
  memset(&db_data, 0, sizeof(DBT));

  key_buf_size = sds_metadata_db_key__get_packed_size(key);
  key_buf      = (uint8_t*)malloc(key_buf_size);

  sds_metadata_db_key__pack(key, key_buf);

  db_key.size  = key_buf_size;
  db_key.data  = key_buf;

  ret = dbpp->del(dbpp, NULL, &db_key, 0);
  dbpp->sync(dbpp, 0);
  
  free(key_buf);
  return ret;
}

int write_metadata_record(DB *dbpp,SDSMetadataDbKey *key, SDSMetadataDbValue*value){
  DBT       db_key, db_data;
  uint8_t  *key_buf,     *val_buf;
  int       key_buf_size, val_buf_size;
  int       ret;

  memset(&db_key,  0, sizeof(DBT));
  memset(&db_data, 0, sizeof(DBT));

  key_buf_size   = sds_metadata_db_key__get_packed_size(key);
  val_buf_size   = sds_metadata_db_value__get_packed_size(value);

  key_buf        = (uint8_t *)malloc(key_buf_size);
  val_buf        = (uint8_t *)malloc(val_buf_size);

  sds_metadata_db_key__pack(key, key_buf);
  sds_metadata_db_value__pack(value, val_buf);

  db_key.size  = key_buf_size;
  db_key.data  = key_buf;

  db_data.size = val_buf_size;
  db_data.data = val_buf;

  /* fprintf(stderr, "key size %d, buf size %d \n", key_buf_size, val_buf_size); */
  //dbpp->put(dbpp, NULL, &db_key, &db_data, DB_NOOVERWRITE);
  ret = dbpp->put(dbpp, NULL, &db_key, &db_data, 0);
  dbpp->sync(dbpp, 0);
  
  free(key_buf);
  free(val_buf);
  return ret;
}


int read_metadata_record(DB *dbpp, SDSMetadataDbKey *key, SDSMetadataDbValue *value){
  DBT                      db_key, db_data;
  uint8_t                 *key_buf;
  int                      key_buf_size;
  SDSMetadataDbValue      *value_temp;
  int                      ret;
  
  memset(&db_key, 0, sizeof(DBT));
  memset(&db_data, 0, sizeof(DBT));

  //printf("or= %s \n", orginal_file_name);    
  key_buf_size   = sds_metadata_db_key__get_packed_size(key);
  key_buf        = (uint8_t  *)malloc(key_buf_size);
  if (key_buf == NULL){
    log_sys("Allocate memory for DB record write fails.");
  }
  
  //Package the key with the function generaged by protobuf-c
  sds_metadata_db_key__pack(key, key_buf);
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
  value_temp  = sds_metadata_db_value__unpack(NULL, db_data.size, db_data.data);
  *value      = *value_temp;
  
  //printf("type %d, type %d \n", (*value).sds_file[0]->reorganization_type,  (*value_temp).sds_file[0]->reorganization_type);
  
  free(db_data.data);
  free(key_buf);
  return ret;
}



//For automatically finding the hot file to perform reorganization
int iterate_sds_db_for_hot_file(DB *dbpp, SDSMetadataDbKey *key, SDSMetadataDbValue *value){
  DBC *cursorp; 
  DBT  db_key, db_data; 
  int  ret;
  int  max_read_count = 0, no_hot_file = 0;
  SDSMetadataDbValue *value_temp, *max_value;
  SDSMetadataDbKey    *max_key;

  /* Database open omitted for clarity */
  /* Get a cursor */
  dbpp->cursor(dbpp, NULL, &cursorp, 0);

  /* Initialize our DBTs. */
  memset(&db_key, 0, sizeof(DBT)); 
  memset(&db_data, 0, sizeof(DBT));
  
  db_data.flags = DB_DBT_MALLOC;

  /* Iterate over the database, retrieving each record in turn. */ 
  while ((ret = cursorp->get(cursorp, &db_key, &db_data, DB_NEXT)) == 0) {
    value_temp = sds_metadata_db_value__unpack(NULL, db_data.size, db_data.data);
    if(value_temp->read_count->interval_count > max_read_count && \
       value_temp->orig_file_status == FILE_STATUS__IN_SERVICE){
      max_read_count = value_temp->read_count->interval_count;
      max_key        = sds_metadata_db_key__unpack(NULL, db_key.size, db_key.data);
      max_value      = value_temp;
      no_hot_file    = 1;
    }
  }

  if(no_hot_file == 1){
    *key   = *max_key;
    *value = *max_value;
    //update_orig_file_status(dbpp, max_key, ORIG_FILE_STATUS__UNDER_EVALUATION);
  }else{
    key   = NULL;
    value = NULL;
  }
  /* Cursors must be closed */ 
  if (cursorp != NULL)
    cursorp->close(cursorp);
 
  return no_hot_file;
}


void update_orig_file_status(DB *dbpp, SDSMetadataDbKey *key, FileStatus stat){
  SDSMetadataDbValue value;
  read_metadata_record(dbpp, key, &value);
  value.orig_file_status = stat;
  write_metadata_record(dbpp, key, &value);
}


void reset_read_count(DB *dbpp, SDSMetadataDbKey *key, int read_count){
  SDSMetadataDbValue value;
  read_metadata_record(dbpp, key, &value);
  value.read_count->interval_count = read_count;
  value.read_count->previous_interval = read_count;
  write_metadata_record(dbpp, key, &value);
}

void increase_read_count_by_one(DB *dbpp, SDSMetadataDbKey *key){
  SDSMetadataDbValue  value = SDS_METADATA_DB_VALUE__INIT;
  
  //Read metadata record
  read_metadata_record(dbpp, key, &value);
  
  //Increase statistics
  value.read_count->interval_count = value.read_count->interval_count + 1;
  value.read_count->life_time      = value.read_count->life_time + 1;
  
  //Write it back
  write_metadata_record(dbpp, key, &value);
}


int  db_create_empty_record(DB *dbpp, SDSMetadataDbKey *key, FileStatus state){
  SDSMetadataDbValue       value = SDS_METADATA_DB_VALUE__INIT;
  ReadCount                rf_entry;
  //rf_entry  = malloc(sizeof(ReadFrequencyEntry));
  read_count__init(&rf_entry);
  rf_entry.previous_interval        = 1;
  rf_entry.previous_400_interval    = 1;
  rf_entry.previous_10000_interval  = 1;
  rf_entry.life_time                = 1;
  
  value.read_count                  = &rf_entry;
  value.n_index_files               = 0;
  value.index_files                 = NULL;
  value.n_reorg_files               = 0;
  value.reorg_files                 = NULL;
  value.orig_file_status            = state; //ORIG_FILE_STATE__UNDER_SERVICE;
  value.owner_id                    = 0;
  value.group_id                    = 0;
  value.owner_bit                   = 0;
  value.group_bit                   = 0;
  value.other_bit                   = 0;
  
  write_metadata_record(dbpp, key, &value);
}



/*
 * The following two functions (write_fake_data & read_fake_data) 
 * are used to write/read faked data. (only for test, not used now)
 */

/*
int  read_fake_data(){
  //char             orginal_file_name[MAX_FILE_NAME_LENGTH] = "original.h5p";
  //char             reorganized_file_name[MAX_FILE_NAME_LENGTH];
  int              i;
  SDSMetadataDbEntryKey    key  = SDS_METADATA_DB_ENTRY_KEY__INIT;
  SDSMetadataDbEntryValue  value ;
  
  char location[MAX_FILE_NAME_LENGTH], file[MAX_FILE_NAME_LENGTH],group[MAX_FILE_NAME_LENGTH],dataset[MAX_FILE_NAME_LENGTH];

  for(i=0; i < 10; ){
    //sprintf(orginal_file_name, "original-%d.h5p", i);
    //db_read_sds_metadata_record(Sds_dbs.sds_metadata_dbp, orginal_file_name, reorganized_file_name, &type);
    //printf("Read_fake_data:%s, %s, %d \n ", orginal_file_name, reorganized_file_name, type);
    i = i + 1;
  
    sprintf(location,    "/scratch2/scratchdirs/dbin/big-stripe/sds-test/");
    sprintf(file,        "original-file-%d.h5p", i);
    sprintf(group,       "/Step#0");
    sprintf(dataset,     "Energy");
    
    key.location = location;
    key.file = file;
    key.group = group;
    key.dataset = dataset;
    
    db_read_sds_metadata_record2(g_sds_dbs.sds_metadata_dbp, &key, &value);
    printf("file name: %s, read statistics: %d, value orga type %d \n", file, value.read_count->life_time, value.expect_reorganization_type);
  }

  return 0;
}


int  write_fake_data(){
  char orginal_file_name[MAX_FILE_NAME_LENGTH] = "original.h5p";
  //char reorganized_file_name[MAX_FILE_NAME_LENGTH] = "original_sorted.h5p";
  char reorganized_file_attribute[MAX_FILE_NAME_LENGTH] = "SomeThing";
  int  i=0, j=0;

  //db_write_sds_rf_attribute_record(g_sds_dbs.sds_reorg_file_attri_dbp, orginal_file_name, reorganized_file_attribute);

  //db_write_sds_rf_attribute_record(Sds_dbs.sds_reorg_file_attri_dbp, orginal_file_name, reorganized_file_attribute);

  SDSMetadataDbEntryKey   key = SDS_METADATA_DB_ENTRY_KEY__INIT;
  SDSMetadataDbEntryValue value = SDS_METADATA_DB_ENTRY_VALUE__INIT;
  char location[MAX_FILE_NAME_LENGTH], file[MAX_FILE_NAME_LENGTH],group[MAX_FILE_NAME_LENGTH],dataset[MAX_FILE_NAME_LENGTH];
  char sds_file_path[MAX_FILE_NAME_LENGTH];
  char sds_file_name[MAX_FILE_NAME_LENGTH];
  char group_path[MAX_FILE_NAME_LENGTH];
  char dataset_name[MAX_FILE_NAME_LENGTH];

  sprintf(sds_file_path,    "/scratch2/scratchdirs/dbin/big-stripe/sds-test/");
  sprintf(sds_file_name,    "original-file-%d-sorted.h5p", i);
  sprintf(group_path,       "/Step#0");
  sprintf(dataset_name,     "Energy");
 

  SdsFileEntry            *sds_file_list[1];
  sds_file_list[0]       = malloc(sizeof(SdsFileEntry));
  sds_file_entry__init(sds_file_list[0]);

  sds_file_list[0]->sds_file_path       = sds_file_path;
  sds_file_list[0]->file_name           = sds_file_name;
  sds_file_list[0]->group_path          = group_path;
  sds_file_list[0]->dataset_name        = dataset_name;
  sds_file_list[0]->reorganization_type = 10000;
  sds_file_list[0]->io_pattern_type     = 10000;

  ReadCountEntry       rf_entry;
  //rf_entry  = malloc(sizeof(ReadFrequencyEntry));
  read_count_entry__init(&rf_entry);
  rf_entry.previous_interval        = 0;
  rf_entry.previous_400_interval    = 2;
  rf_entry.previous_10000_interval  = 3;
  rf_entry.life_time                = 4;



  //Attribute
  char                         attribute_file_name[MAX_FILE_NAME_LENGTH+MAX_LOCATION_NAME_LENGTH];
  FILE                        *fd;
  SdsAttributeValue            a_value          = SDS_ATTRIBUTE_VALUE__INIT; 
  SdsSortAttribute             sort_attribute = SDS_SORT_ATTRIBUTE__INIT; 
  float                        min, max;
  unsigned long long           offset_start, offset_end;

  fd =  fopen("./attribute", "r");
  if(fd == NULL){
    log_quit("Error open the attribute file ! \n");
  }

  a_value.reorganization_type = 0;
  sort_attribute.row          = 24;
  sort_attribute.col          = 4;
  
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
  a_value.sort_attribute = &sort_attribute;
  

  for(i = 0; i < 1100; i++){
    sprintf(location,    "/scratch2/scratchdirs/dbin/big-stripe/sds-test/");
    sprintf(file,        "original-file-%d.h5p", i);
    sprintf(group,       "/Step#0");
    sprintf(dataset,     "Energy");
  
    key.location = location;
    key.file = file;
    key.group = group;
    key.dataset = dataset ;
    
    value.expect_reorganization_type = SORT;
    value.io_pattern_type         = i+1;
    value.read_count        = &rf_entry;
    value.owner_id          = i;
    value.group_id          = i;
    value.owner_bit         = i;
    value.group_bit         = i;
    value.other_bit         = i;
    value.n_sds_file        = 1;
    value.orig_file_status  = ORIG_FILE_STATE__UNDER_SERVICE;
    value.sds_file          = sds_file_list;
    //value.sds_file          = NULL;
    
    //printf("%d \n ", i);
    db_write_new_sds_metadata_record2(g_sds_dbs.sds_metadata_dbp, &key, &value);
    attribute_db_write_new_record(g_sds_dbs.sds_reorg_file_attri_dbp, &key, &a_value);
    //printf("%s %d \n", key.file, value.owner_id);
  }
 
  free(sds_file_list[0]);
  for (i = 0 ; i < sort_attribute.row; i++){
    free(sort_attribute.table[i]);
  }
  free(sort_attribute.table);
  fclose(fd);
  return 1;
}


int  write_fake_data_5T(){

  int  i=0, j=0;

  SDSMetadataDbEntryKey   key = SDS_METADATA_DB_ENTRY_KEY__INIT;
  SDSMetadataDbEntryValue value = SDS_METADATA_DB_ENTRY_VALUE__INIT;
  char location[MAX_FILE_NAME_LENGTH],      file[MAX_FILE_NAME_LENGTH],          group[MAX_FILE_NAME_LENGTH],      dataset[MAX_FILE_NAME_LENGTH];
  char sds_file_path[MAX_FILE_NAME_LENGTH], sds_file_name[MAX_FILE_NAME_LENGTH], group_path[MAX_FILE_NAME_LENGTH], dataset_name[MAX_FILE_NAME_LENGTH];

  sprintf(sds_file_path,    "/scratch3/scratchdirs/dbin/sds-index-unsorted");
  sprintf(sds_file_name,    "eparticle_T11430_1.1_filter-sorted.h5p");
  sprintf(group_path,       "/Step#0");
  sprintf(dataset_name,     "Energy");
 

  SdsFileEntry            *sds_file_list[1];
  sds_file_list[0]       = malloc(sizeof(SdsFileEntry));
  sds_file_entry__init(sds_file_list[0]);

  sds_file_list[0]->sds_file_path       = sds_file_path;
  sds_file_list[0]->file_name           = sds_file_name;
  sds_file_list[0]->group_path          = group_path;
  sds_file_list[0]->dataset_name        = dataset_name;
  sds_file_list[0]->reorganization_type = 10000;
  sds_file_list[0]->io_pattern_type     = 10000;

  ReadCountEntry       rf_entry;
  //rf_entry  = malloc(sizeof(ReadFrequencyEntry));
  read_count_entry__init(&rf_entry);
  rf_entry.previous_interval        = 0;
  rf_entry.previous_400_interval    = 0;
  rf_entry.previous_10000_interval  = 0;
  rf_entry.life_time                = 0;



  //Attribute
  char                         attribute_file_name[MAX_FILE_NAME_LENGTH];
  FILE                        *fd;
  SdsAttributeValue            a_value          = SDS_ATTRIBUTE_VALUE__INIT; 
  SdsSortAttribute             sort_attribute = SDS_SORT_ATTRIBUTE__INIT; 
  float                        min, max;
  unsigned long long           offset_start, offset_end;


  sprintf(attribute_file_name, "/scratch3/scratchdirs/dbin/sds-index-unsorted/eparticle_T11430_1.1_filter-sorted.attribute");

  a_value.reorganization_type = 0;
  sort_attribute.row          = 16000;
  sort_attribute.col          = 4;
  sort_attribute.attribute_file_name = attribute_file_name;
  sort_attribute.table = NULL;
  sort_attribute.n_table = 0;
  a_value.sort_attribute = &sort_attribute;

  sprintf(location,    "/scratch3/scratchdirs/dbin/sds-index-unsorted");
  sprintf(file,        "eparticle_T11430_1.1_filter.h5p");
  sprintf(group,       "/Step#0");
  sprintf(dataset,     "Energy");
  
  key.location = location;
  key.file = file;
  key.group = group;
  key.dataset = dataset ;
  
  value.expect_reorganization_type = SORT;
  value.io_pattern_type         = i+1;
  value.read_count        = &rf_entry;
  value.owner_id          = i;
  value.group_id          = i;
  value.owner_bit         = i;
  value.group_bit         = i;
  value.other_bit         = i;
  value.n_sds_file        = 1;
  value.orig_file_status  = ORIG_FILE_STATE__UNDER_SERVICE;
  value.sds_file          = sds_file_list;
  
  db_write_new_sds_metadata_record2(g_sds_dbs.sds_metadata_dbp,     &key, &value);
  attribute_db_write_new_record(g_sds_dbs.sds_reorg_file_attri_dbp, &key, &a_value);
     
  free(sds_file_list[0]);

  //fclose(fd);
  return 1;
}


int  write_metadata_test_data(){

  int  i=0, j=0;

  SDSMetadataDbEntryKey   key = SDS_METADATA_DB_ENTRY_KEY__INIT;
  SDSMetadataDbEntryValue value = SDS_METADATA_DB_ENTRY_VALUE__INIT;
  char location[MAX_FILE_NAME_LENGTH],      file[MAX_FILE_NAME_LENGTH],          group[MAX_FILE_NAME_LENGTH],      dataset[MAX_FILE_NAME_LENGTH];
  char sds_file_path[MAX_FILE_NAME_LENGTH], sds_file_name[MAX_FILE_NAME_LENGTH], group_path[MAX_FILE_NAME_LENGTH], dataset_name[MAX_FILE_NAME_LENGTH];

  ///scratch3/scratchdirs/dbin/sds-index/meta-test/original-file-0.h5p
  
  sprintf(sds_file_path,    "/scratch3/scratchdirs/dbin/sds-index/meta-test/");
  sprintf(sds_file_name,    "original-file-1.h5p");
  sprintf(group_path,       "/Step#0");
  sprintf(dataset_name,     "Energy");
 

  SdsFileEntry            *sds_file_list[1];
  sds_file_list[0]       = malloc(sizeof(SdsFileEntry));
  sds_file_entry__init(sds_file_list[0]);

  sds_file_list[0]->sds_file_path       = sds_file_path;
  sds_file_list[0]->group_path          = group_path;
  sds_file_list[0]->file_name           = sds_file_name;
  sds_file_list[0]->dataset_name        = dataset_name;
  sds_file_list[0]->reorganization_type = 10000;
  sds_file_list[0]->io_pattern_type     = 10000;
  
  ReadCountEntry       rf_entry;
  //rf_entry  = malloc(sizeof(ReadFrequencyEntry));
  read_count_entry__init(&rf_entry);
  rf_entry.previous_interval        = 0;
  rf_entry.previous_400_interval    = 0;
  rf_entry.previous_10000_interval  = 0;
  rf_entry.life_time                = 0;


  //Attribute
  char                         attribute_file_name[MAX_FILE_NAME_LENGTH];
  FILE                        *fd;
  SdsAttributeValue            a_value          = SDS_ATTRIBUTE_VALUE__INIT; 
  SdsSortAttribute             sort_attribute = SDS_SORT_ATTRIBUTE__INIT; 
  float                        min, max;
  unsigned long long           offset_start, offset_end;


  sprintf(attribute_file_name, "/scratch3/scratchdirs/dbin/sds-index/meta-test/5T.attribute");

  a_value.reorganization_type = 0;
  sort_attribute.row          = 6000;
  sort_attribute.col          = 4;
  sort_attribute.attribute_file_name = attribute_file_name;
  sort_attribute.table = NULL;
  sort_attribute.n_table = 0;

  sprintf(location,    "/scratch3/scratchdirs/dbin/sds-index/meta-test/");
  sprintf(group,       "/Step#0");
  sprintf(dataset,     "Energy");
  
  key.location = location;
  key.group = group;
  key.dataset = dataset ;
  
  value.expect_reorganization_type = SORT;
  value.io_pattern_type         = i+1;
  value.read_count        = &rf_entry;
  value.owner_id          = i;
  value.group_id          = i;
  value.owner_bit         = i;
  value.group_bit         = i;
  value.other_bit         = i;
  value.n_sds_file        = 1;
  value.orig_file_status  = ORIG_FILE_STATE__UNDER_SERVICE;
  value.sds_file          = sds_file_list;

  a_value.sort_attribute = &sort_attribute;
    
  for (i = 0; i < 5000; i++){
    sprintf(file,        "original-file-%d.h5p", i);
    key.file = file;
   
    db_write_new_sds_metadata_record2(g_sds_dbs.sds_metadata_dbp,     &key, &value);
    attribute_db_write_new_record(g_sds_dbs.sds_reorg_file_attri_dbp, &key, &a_value);
  }
  
  free(sds_file_list[0]);

  //fclose(fd);
  return 1;
}
*/




