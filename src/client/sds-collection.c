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
 * This file defines SDS Collection and the functions associated. SDS Collection is group of SDS_Object.  
 * In general, SDS Collection could be thought as relational table, SciDB Array, HDF5 Group
 *  
 * Author: Bin Dong <dbin at lbl.gov >
 * Copyright 2015 the Regents of the University of California
 * 
 */

#include "sds-collection.h"
#include "fastquery-c-interface.h"
//Create an empty collection
SDS_Collection  *SDS_Collection_init(int object_count){
  SDS_Collection *new_coll;
  new_coll = malloc(sizeof(SDS_Collection));
  if(new_coll == NULL){
    log_quit("Memeory allocation fail for SDS_Collection_init()\n");
  }

  new_coll->object_array = (SDS_Object **)malloc(sizeof(SDS_Object *) * object_count);
  if( new_coll->object_array == NULL){
    log_quit("Memeory allocation fail for SDS_Collection_init()\n");
  }
  
  new_coll->object_count  = object_count;
  new_coll->object_cursor = 0;
  return new_coll;
}

//Release buffer and collection itself collection
int   SDS_Collection_finalize(SDS_Collection  *coll){
  if(coll ==  NULL)
    return 0;

  int i, ret;
  for(i = 0; i < coll->object_count; i++){
    SDS_Object_finalize(coll->object_array[i]);
  }
  free(coll);
  return 0; 
}

//Release only buffer of the collection
int SDS_Collection_release_buffer(SDS_Collection  *coll){
  if(coll ==  NULL)
    return 0;
  
  int i;
  for(i = 0; i < coll->object_count; i++){
    SDS_Object_release_buffer(coll->object_array[i]);
  }
  
  return 0;
}

//Create am collection based on HDF5 group, each HDF5 dataset (not including subgroup)in the group will be turned into a SDS object
SDS_Collection  *SDS_Collection_HDF5_Group_init(char *hdf5_filename, char *hdf5_group){
  printf("SDS_Collection_HDF5_Group_init is under-development!");
}
//Create am collection based on NetCDF group, each NecCDF variable (not including subgroup) in the group will be turned into a SDS object
SDS_Collection  *SDS_Collection_NetCDF_Group_init(char *netcdf_filename, char *netcdf_group){
  printf("SDS_Collection_NetCDF_Group_init is under-development!");
}

//Merge two collection into the same one
SDS_Collection  *SDS_Collection_merge(SDS_Collection  *coll1, SDS_Collection  *coll2){
  SDS_Collection  *new_coll;
  int i, count;;
  count =  coll1->object_count + coll2->object_count;
  new_coll  =SDS_Collection_init(count);
  for(i = 0; i < coll1->object_count; ){
    SDS_Collection_append(new_coll, coll1->object_array[i]);
  }
  
  for(i = 0; i < coll2->object_count; ){
    SDS_Collection_append(new_coll, coll2->object_array[i]);
  }
      
  return new_coll;
}

//Append one obj to the end of the collection
int SDS_Collection_append(SDS_Collection  *coll, SDS_Object *obj){
  if(coll->object_cursor > coll->object_count){
    log_quit("Collection is too small to contain all objects!");
  }else{
    coll->object_array[coll->object_cursor] = obj;
  }
  coll->object_cursor = coll->object_cursor + 1;
  return coll->object_cursor;
}

//Set the nth  objects 
int SDS_Collection_set(SDS_Collection  *coll, SDS_Object *obj, int index){
  coll->object_array[index] = obj;
}

//Get the nth objects  
SDS_Object      *SDS_Collection_get_object(SDS_Collection  *coll, int index){
  return coll->object_array[index];
}


int SDS_Collection_get_object_index(SDS_Collection *coll, char *name){
  int   i;
  char *obj_name;
  for(i = 0; i < coll->object_count; i++){
    obj_name = SDS_Object_get_name(coll->object_array[i]);
    //To add more test, like space, ca
    if((strcmp(obj_name, name) == 0)){
      return i;
    }
  }
  
  //No object with "char *name" is found 
  return -1; 
}

//Get the number of objects  
int SDS_Collection_get_size(SDS_Collection  *coll){
  return coll->object_count;
}

//List all objects  
int SDS_Collection_list_objects(SDS_Collection  *coll){
  int i;
  printf("Collection has  %d objects, they are :  \n",coll->object_count );
  for(i = 0; i < coll->object_count; i++){
    SDS_Object_print_id(coll->object_array[i]);

  }

}


//List objct data 
int              SDS_Collection_list_data(SDS_Collection  *coll, int index){
  SDS_Object_print_data(coll->object_array[index]);
}


//Select the object inside the collection 
SDS_Collection  *SDS_Collection_project(SDS_Collection  *coll, int *selected_obj_indexs, SDS_Query_comm comm){
  int i, n;
  SDS_Collection  *new_coll;
  n = sizeof(selected_obj_indexs)/sizeof(selected_obj_indexs[0]);
  new_coll = SDS_Collection_init(n);
  for(i = 0; i < n; i++){
    SDS_Collection_append(new_coll, coll->object_array[selected_obj_indexs[i]]);
  }
  return new_coll;
}
//Find the object index in "coll" based on the name in "cond_tree"
void sync_ctree_index(SDS_Collection  *coll, SDS_Condition_tree *cond_tree){
  SDS_Condition_tree    **s_tree;
  int n = 0, i = 0, object_index;
  //printf("I am here !");
  SDS_Condition_tree_n_node(cond_tree, &n);
  //printf("Size of cond_tree %d \n", n);

  s_tree = malloc(sizeof(SDS_Condition_tree *) * n);
  SDS_Condition_tree_serialize(cond_tree, s_tree, &i);
  for ( i = 0; i < n; i++){
    if(s_tree[i] != NULL){
      if(s_tree[i]->type == LEAF){
        object_index = SDS_Collection_get_object_index(coll, s_tree[i]->object_name);
        if(object_index < 0){
          log_quit("Unknown variable name (%s) in condition string.", s_tree[i]->object_name);
        }
        s_tree[i]->object_index = object_index;
        //printf("variable name in cond_tree %s, and index %d \n", s_tree[i]->object_name, object_index);
      }
      //free(s_tree[i]);
    }
  }
  free(s_tree);
}

//Select each boject based on the cond_tree 
SDS_Collection  *SDS_Collection_select(SDS_Collection  *coll, char *cond_str, SDS_Query_comm comm){
  SDS_Bool        *index_covered = malloc(coll->object_count);
  int              ret, i, covered_count = 0;
  SDS_Collection  *cond_coll, *pop_coll;
  SDS_Condition_tree *cond_tree;
  //Match the name the the index of object in collection
  //The cond_tree is based on variable name
  //The evalatuion on cond_tree is based on index of object in "coll"
  parse_condition_tree(cond_str, &cond_tree);
  sync_ctree_index(coll, cond_tree);

  ret = SDS_Condition_tree_object_covered(cond_tree, index_covered);  
  if(ret != 0){
    log_quit("Condition tree is not valid in SDS_Collection_select(...)!");
  }
  
  for(i = 0; i < coll->object_count; i++){
    if(index_covered[i] == SDS_TRUE){
      covered_count++;
    }
  }
  //Evaluate the coll and cond_tree to make sure it is valid
  //The number of objects in coll is less than or equal to coll->object_count 
  if(covered_count > coll->object_count){
    log_quit("Number of Objects in Condition tree larger than objects in collection. !");
  }

  //int ret;
  log_msg("Condition tree covers [%d/%d ] objects in collcetion ", covered_count, coll->object_count );
  //Check "do we have SDS files in memory"  --> non-SDS_Query_treee node
  if(SDS_Collection_data_exist(coll) != SDS_TRUE){ 
    log_msg("Check data in memeroy: NO (data on disk) !");
    //No data in memory --> Fist time to read the data into memory (Most SELECTs need this)
    //Contact SDS Server for possible bitmap index and reorganization files for collection.
    ret = SDS_Collection_fetch_sds_files(coll);
    if(ret > 0){
      //Check "do we have SDS files existing for collection ?" 
      if(SDS_Collection_SDSfile_exist(coll) == SDS_TRUE){
        log_msg("Check index file exists: successed, using SDS files.");
        SDS_Collection_read_disk_index(coll, index_covered, cond_tree, cond_str);
      }else{
        //Full scan the data from disk to answer the query
        log_msg("Check index file exists: none index/reorg files are found, using Full-scan. ");
        SDS_Collection_read_disk_fullscan(coll, index_covered, cond_tree,comm);
      }
    }else{
      //Full scan the data from disk to answer the query
      log_msg("Using Full-scan (by default).");
      SDS_Collection_read_disk_fullscan(coll, index_covered, cond_tree,comm);
    }

  }else{
    //Filter the data in memory, no index are necessary and just scan it. 
    log_msg("Check data exists for collection sucessds (data in memory)! ");
    SDS_Collection_read_memory_fullscan(coll, cond_tree);
  }
  return coll;
}
  
//Eq-join two collections based on their key object index
SDS_Collection  *SDS_Collection_join(SDS_Collection  *coll1, int key_obj_index1, SDS_Collection  *coll2, int key_obj_index2, SDS_Query_comm comm){
  
}

//User-defined code on a collection
SDS_Collection  *SDS_Collection_udf(SDS_Collection  *coll, void *udf_code_pointer,  SDS_Query_comm comm){


}


SDS_Bool SDS_Collection_SDSfile_exist(SDS_Collection  *coll){
  int i;
  SDS_Bool  ret = SDS_FALSE;

  for (i =0 ; i < coll->object_count; i++){
    if(SDS_Object_SDSfile_exist(coll->object_array[i]) == SDS_TRUE){
      return SDS_TRUE; //If one existing, then all will be there
    }
  }
  
  return ret;
}

//Check whether we have some sds files at SDS Server
//Instead of checking for each object with multiple communication, we choose to check
//the whole Collection  with a single communication 
int SDS_Collection_fetch_sds_files(SDS_Collection  *coll){
  return SDS_read_collection_metadata(coll->object_array, coll->object_count); 
}


int SDS_Collection_read_disk_index(SDS_Collection  *coll, int *index_covered, SDS_Condition_tree *cond_tree, char *cond_str){
  int sds_index_type;
  int ret;
  sds_index_type = SDS_Collection_index_type(coll);
  
  switch(sds_index_type){
    case SORT_INDEX :
      ret=SDS_Collection_read_sort(coll, index_covered, cond_tree);
      break;
    case BITMAP_INDEX:
      ret=SDS_Collection_read_bitmap(coll, index_covered, cond_tree, cond_str);
      break;
    case MDBIN_INDEX:
      ret=SDS_Collection_read_mdbin(coll, index_covered, cond_tree);
      break;
    default:
      log_quit("Un-recorganized Index type in SDS_Collection_SDSfile_read!");
      break;
  }
 
  return ret;
}

int SDS_Collection_read_disk_fullscan(SDS_Collection  *coll, int *index_covered, SDS_Condition_tree *cond_tree, SDS_Query_comm comm){
  int   i, covered_n = 0, non_covered_n = 0;

  for(i = 0; i < coll->object_count; i++){
    if(index_covered[i] == SDS_TRUE)
      covered_n++;
  }
  non_covered_n  = coll->object_count - covered_n;
  int                            *covered_object_index;
  SDS_Operation                  *covered_object_operand;
  SDS_Object_operation_type      *covered_merge_operand;

  covered_object_index     = malloc(sizeof(int) * covered_n);
  covered_object_operand   = malloc(sizeof(SDS_Operation) * covered_n);
  covered_merge_operand    = malloc(sizeof(SDS_Object_operation_type) * (covered_n-1));

  int covered_obj_i = 0, covered_merge_i = 0;
  SDS_Condition_tree_object_info(cond_tree, covered_object_index, covered_object_operand, &covered_obj_i, covered_merge_operand, &covered_merge_i);
  
  log_msg("Covered index %d\n", covered_object_index[0]);
  
  SDS_Offset               data_size, buffer_index;
  SDS_Value_union          data;
  int                      object_index;
  SDS_Operation            operand;
  SDS_Bool                 prev_bool = SDS_TRUE, current_bool;
  SDS_Data_type            data_type;
  //Read all the data into memory
  //Todo: Find the smallest range to dump first when necessary. 
  for(i = 0; i < coll->object_count; i++)
    SDS_Object_read_all_data(coll->object_array[i], comm);

  //log_msg("All data are read into memory !");
  SDS_Object_print_data(coll->object_array[0]);

  //Use my_size of any covered object as then have same size
  data_size = coll->object_array[covered_object_index[0]]->my_size;
  //all data in buffer
  for(buffer_index = 0; buffer_index < data_size; buffer_index++){
    //Evaluate condition on covered objects 
    prev_bool = SDS_FALSE;
    for(i = 0 ; i < covered_n; i++){
      object_index = covered_object_index[i];
      if(SDS_Object_get_data(coll->object_array[object_index], buffer_index, &data) == SDS_FALSE){
        //No value 
        prev_bool = SDS_FALSE;
        break;
      }
      data_type    = SDS_Object_get_data_type(coll->object_array[object_index]);
      operand      = covered_object_operand[object_index];
      current_bool = SDS_Value_compare(data, data_type, &operand);
     
      if(i == 0){
        prev_bool = current_bool;
        continue;
      }
      
      switch(covered_merge_operand[i]){
        case SDS_AND:
          if(prev_bool && current_bool){
            prev_bool = SDS_TRUE;
          }else{
            prev_bool = SDS_FALSE;
          }
          break;
        case SDS_OR:
          if(prev_bool || current_bool){
            prev_bool = SDS_TRUE;
          }else{
            prev_bool = SDS_FALSE;
          }
          break;
        default:
          break;
      }
      if(prev_bool == SDS_FALSE)
        break;
    }
    
    //Set the results
    for(i = 0 ; i < coll->object_count; i++){
      SDS_Object_set_data_filter(coll->object_array[i], buffer_index, prev_bool);
    }
    //log_msg("%d value %d ( operand.op_type = %d, operand.op_operand =%f) , result %d\n", buffer_index, data.i, operand.op_type[0], operand.op_operand[0].d, prev_bool);

  }
  
  //log_msg("Full scan is done with SDS_Collection_read_disk_fullscan()");
  log_msg("Full scan evaluation is done.");

}

SDS_Bool SDS_Collection_data_exist(SDS_Collection  *coll){
  int i;
  SDS_Bool ret = SDS_FALSE;
  for(i = 0; i < coll->object_count; i++){
    ret = SDS_Object_data_exist(coll->object_array[i]);
    if(ret == SDS_FALSE)
      return ret;
  }
  
  return ret;
}

int SDS_Collection_read_sort(SDS_Collection  *coll, int *index_covered, SDS_Condition_tree *cond_tree){

}

//Here we assume the objects in "coll" are datasets in the same HDF5 group
int SDS_Collection_read_bitmap(SDS_Collection  *coll, int *index_covered, SDS_Condition_tree *cond_tree, char *cond_str){
  //log_msg("Here we aree ...");
  //return fq_multi_dsets(cond_str, coll);
  SDS_Value_union **result_buf;
  char            **dset;
  char             *filename;
  char             *groupname;
  char             *indexfile;
  int               i, j, ret = 0;
  unsigned int      fq_hits;
  
  dset       = malloc(sizeof(char *) * coll->object_count);
  result_buf = malloc(sizeof(SDS_Value_union *) * coll->object_count);

  for(i = 0; i < coll->object_count; i++){
    dset[i]=strdup(coll->object_array[i]->dsetname);
    //dset[i]=coll->object_array[i]->dsetname;
  }

  //printf("%s %d  \n ", coll->object_array[0]->dsetname, coll->object_count);
  filename  = strdup(coll->object_array[0]->filename);
  groupname = strdup(coll->object_array[0]->group);
  indexfile = strdup(coll->object_array[0]->index_files->filename);
  
  char      *temp_parameter, *lp, *temp;
  int        fq_length;

  temp_parameter = strdup(coll->object_array[0]->index_files->parameters);
  temp           = strdup(temp_parameter); 
  lp  = strstr(temp_parameter, "-l");
  if(lp != NULL){
    lp = lp + sizeof("-l");
    while(*lp == ' ')
      lp++;
    sscanf(lp, "%d%s", &fq_length, temp);
  }else{
    fq_length = 0;
  }
  free(temp);
  free(temp_parameter);

  //printf(" %s, %d \n ", coll->object_array[0]->index_files->parameters, fq_length);
  ret = fq_multi_dsets(cond_str, filename, groupname, dset, coll->object_count, indexfile, fq_length, result_buf, &fq_hits, NULL);
  
  //printf("hits %d \n", fq_hits);
  //for(i = 0; i < fq_hits ; i++){
  //  printf("%d \n", result_buf[0][i].i);
  //}
  
  for(i = 0; i < coll->object_count; i++){
    coll->object_array[i]->data_buffer_filter = malloc(sizeof(SDS_Bool) * fq_hits);
    for (j = 0;  j < fq_hits; j++)
      coll->object_array[i]->data_buffer_filter[j] = SDS_TRUE;
    coll->object_array[i]->data_buffer = result_buf[i];
    coll->object_array[i]->my_size = fq_hits;
    free(dset[i]);
  }
  free(result_buf);
  free(dset);

  return ret;

}

int SDS_Collection_read_mdbin(SDS_Collection  *coll, int *index_covered, SDS_Condition_tree *cond_tree){

}


SDS_Index_type SDS_Collection_index_type(SDS_Collection  *coll){
  SDS_Index_type  *type;
  int i;
  type = malloc(sizeof(SDS_Index_type)*coll->object_count);
  for(i = 0; i < coll->object_count; i++){
    type[i] = SDS_Object_get_index_type(coll->object_array[i]);
  }

  for(i = 0; i < coll->object_count; i++){
    if(type[i] == MDBIN_INDEX)
      return MDBIN_INDEX;
  }

  for(i = 0; i < coll->object_count; i++){
    if(type[i] == SORT_INDEX)
      return SORT_INDEX;
  }

  for(i = 0; i < coll->object_count; i++){
    if(type[i] == BITMAP_INDEX)
      return BITMAP_INDEX;
  }

  for(i = 0; i < coll->object_count; i++){
    if(type[i] == USER_DEFINED_INDEX) 
      return USER_DEFINED_INDEX;
  }
  
  return NONE_INDEX;
}

int  SDS_Collection_read_memory_fullscan(SDS_Collection  *coll, SDS_Condition_tree *cond_tree){
  //To do all
  return 0;
}

int  SDS_Collection_save_results(SDS_Collection  *coll, SDS_Bool merge, SDS_Query_comm comm){
  int i, n = coll->object_count;
  for (i = 0; i < n ; i++){
    SDS_Object_write_all_data(coll->object_array[i], merge, comm);
  }
}


int  SDS_Collection_append_result(SDS_Collection  *coll, SDS_Bool merge, SDS_Query_comm comm, char *filename){
  int i, n = coll->object_count;
  for (i = 0; i < n ; i++){
    SDS_Object_append_result(coll->object_array[i], merge, comm, filename);
  }
}

SDS_Collection  *SDS_Collection_histgram(SDS_Collection  *coll, void *hist_par_p, SDS_Query_comm comm){
  SDS_Collection  *result_collection;
  //Check "do we have SDS files in memory"  --> non-SDS_Query_treee node
  if(SDS_Collection_data_exist(coll) != SDS_TRUE){ 
    //No data in memory --> Fist time to read the data into memory (Most select need this)
    //Contact SDS Server for possible bitmap index and reorganization files for coll.
    SDS_Collection_fetch_sds_files(coll);
    //Check "do we have SDS files existing for collection ?" 
    if(SDS_Collection_SDSfile_exist(coll) == SDS_TRUE){
      result_collection = SDS_Collection_build_histgram_index(coll, hist_par_p);
    }else{
      //Full scan the data from disk to answer the query
      result_collection = SDS_Collection_build_histram_fullscan(coll, hist_par_p);
      //printf("I am here !");
    }
  }else{
    //Filter the data in memory, no index are necessary and just scan it. 
    result_collection =SDS_Collection_build_histgram_memory(coll, hist_par_p);
  }
  
  return result_collection;
}

SDS_Collection  *SDS_Collection_build_histgram_index(SDS_Collection  *coll, SDS_Histgram_operand *hist_par_p){
  char              *filename, *group, **dataset;
  int                i, dim, mpi_length, *count, fq_hits;
  SDS_Object        *obj;
  SDS_Index_file    *index_file;
  
  dim     = SDS_Collection_get_size(coll);
  dataset = malloc(dim * sizeof(char *));
  
  //Use the index of the first object
  obj         = SDS_Collection_get_object(coll, 0);
  index_file  = SDS_Object_get_index_file(obj);
  group       = strdup(obj->group);
  filename    = strdup(obj->filename);
  
  for(i = 0; i < dim; i++){
    obj = SDS_Collection_get_object(coll, i);
    dataset[i] = strdup(obj->dsetname);
  }
  //Assume every dataset has same file/group names
  
  mpi_length = get_mpi_length(index_file->parameters);
  
  fq_hist(hist_par_p->query_str, filename, group, dataset, dim, index_file->filename, mpi_length, hist_par_p->begin_str, hist_par_p->stripe_str, hist_par_p->end_str, count, &fq_hits);

  SDS_Collection_release_buffer(coll);

  SDS_Collection  *result_collection;
  result_collection = SDS_Collection_init(1);
  SDS_Object *hist_obj;
  hist_obj = SDS_Object_init("hist", "hist", "hist", SDS_MEMORY, SDS_INT);
  hist_obj->data_buffer = count;
  hist_obj->my_size  = fq_hits;
  SDS_Collection_append(result_collection, hist_obj);
  //obj = SDS_Collection_get_object(coll, 0);
  //obj->other = count;
  return result_collection;
}

SDS_Collection  *SDS_Collection_build_histram_fullscan(SDS_Collection  *coll, SDS_Histgram_operand *hist_par_p){
  char            *index_file,*filename, *group, **dataset;
  int              i, dim, *count, fq_count_size;
  SDS_Object      *obj;
  
  dim     = SDS_Collection_get_size(coll);
  dataset = malloc(dim * sizeof(char *));
  
  //Use the index of the first object
  obj         = SDS_Collection_get_object(coll, 0);
  group       = strdup(obj->group);
  filename    = strdup(obj->filename);
  
  for(i = 0; i < dim; i++){
    obj = SDS_Collection_get_object(coll, i);
    dataset[i] = strdup(obj->dsetname);
  }
  //Assume every dataset has same file/group names
  
  fq_hist(hist_par_p->query_str, filename, group, dataset, dim, NULL, -1, hist_par_p->begin_str, hist_par_p->stripe_str, hist_par_p->end_str, &count, &fq_count_size);
  
  //SDS_Collection_finalize(c_result);
  //SDS_Collection_release_buffer(coll);

  //printf("hits %d, c[0]= %d, c[1]= %d \n", fq_count_size, count[0], count[1]);
  SDS_Collection  *result_collection;
  SDS_Object      *hist_obj;
  SDS_Value_union *data;
  hist_obj = SDS_Object_init("hist", "hist", "hist", SDS_HDF5, SDS_INT);
  data = (SDS_Value_union *)malloc(sizeof(SDS_Value_union) * fq_count_size);;
  hist_obj->data_buffer_filter = malloc(sizeof(int) * fq_count_size);
  for(i = 0; i < fq_count_size; i++){
    data[i].i=count[i];
    hist_obj->data_buffer_filter[i] = SDS_TRUE;
    //printf("%d  ", count[i]);
  }
  hist_obj->data_buffer = data;
  hist_obj->my_size  = fq_count_size;
  
  if(count != NULL)
    free(count);
  
  result_collection = SDS_Collection_init(1);
  SDS_Collection_append(result_collection, hist_obj);
  //obj = SDS_Collection_get_object(coll, 0);
  //obj->other = count;
  //printf("I am here after fullscan!");
  return result_collection;
}


SDS_Collection  *SDS_Collection_build_histgram_memory(SDS_Collection  *coll, SDS_Histgram_operand *hist_par_p){
  
}

