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
 * This file defines SDS Object and the functions associated. 
 * SDS_Object is a high-level abstract of data set, such HDF5 dataset, SciDB Attribute, Table Column, etc.
 *  
 * Author: Bin Dong <dbin at lbl.gov >
 * Copyright 2015 the Regents of the University of California
 * 
 */

#include "sds-object.h"
#include "sds-hdf5.h"
#include "sds-binary.h"
#include "sds-netcdf.h"

/* 
 * Initialize a Object variable 
 * -- For a single HDF5 variable, it is a single dataset identified by (filename, group, dsetname). 
 * -- For a binary variable, it is a single file identified by (filename). 
 * Each variable has its own datatype, e.g. float, int.
 */
SDS_Object     *SDS_Object_init(char*filename, char *group, char *dsetname, SDS_File_type file_type, SDS_Data_type data_type){
  SDS_Object   *sds_obj;
  int len;

  sds_obj = malloc(sizeof(SDS_Object));
  if(filename == NULL){
    log_quit("File name must be provided to SDS_Object_init !\n");
  }else{
    len = strlen(filename);
    sds_obj->filename = malloc(len * sizeof(char));
    strcpy(sds_obj->filename, filename);
  }

  if(file_type == SDS_HDF5 ){
    if(group == NULL || dsetname == NULL ){
      log_quit("Group and dataset names must be provided to SDS_Variable_Init for a HDF5 file !\n");
    }
      
    len = strlen(dsetname);
    sds_obj->dsetname = malloc(len * sizeof(char));
    strcpy(sds_obj->dsetname, dsetname);
    
    len = strlen(group);
    sds_obj->group = malloc(len * sizeof(char));
    strcpy(sds_obj->group, group);
  }else{
    log_quit("No other file types (only HDF5) are supported in object init !");
  }
  
  //Try to automacially find the data type
  if(data_type == SDS_UNKNOWN_TYPE){
    switch(file_type){
      case SDS_HDF5:
        data_type = hdf5_dataset_datatype(filename, group, dsetname);
        break;
      default:
        log_quit("Please provide datatype to initialize SDS_Objects ");
        break;
    }
    if(data_type == SDS_UNKNOWN_TYPE){
      log_quit("Error: can't find datatype of the file !");
    }
  }
  
  sds_obj->file_type = file_type;
  sds_obj->data_type = data_type;
  
  //sds_obj->array_attribute  ;
  sds_obj->index_files        = NULL; 
  sds_obj->reorg_files        = NULL;
  sds_obj->data_buffer        = NULL;
  sds_obj->data_buffer_filter = NULL;
  sds_obj->sparse_array    = SDS_FALSE;
  //more information, we can extract and store here in furture
  sds_obj->other           = NULL;
  
  return sds_obj;
}


int SDS_Object_set_array_info(SDS_Object  *obj, int dims, long int *length){
  SDS_Array_attribute *new_array_info;
  int i;
  if(dims > SDS_ARRAY_MAX_DIMS)
    log_quit("Increase SDS_ARRAY_MAX_DIMS  in sds-array.h for large array.");
  //new_array_info = malloc(sizeof(SDS_Array_attribute));
  new_array_info->dims = dims;
  //new_array_info->length = malloc(sizeof(long int)*dims);
  for (i = 0; i < dims; i++)
    new_array_info->length[i] = length[i];
  
  return 0;
}

int  SDS_Var_set_index_file(SDS_Object  *obj, SDS_Index_file  *files){
 
}

int  SDS_Var_set_reorg_file(SDS_Object  *obj, SDS_Reorg_file  *file){

}

SDS_Data_type  SDS_Object_get_data_type(SDS_Object    *obj){
  if(obj == NULL){
    log_quit("SDS_Object is NULL in SDS_Object_get_data_type()!\n");
  }
  return obj->data_type;
}
SDS_File_type  SDS_Object_get_file_type(SDS_Object    *obj){
  if(obj == NULL){
    log_quit("SDS_Object is NULL in SDS_Object_get_file_type()!\n");
  }
  return obj->file_type;
}

SDS_Bool     SDS_Object_SDSfile_exist(SDS_Object    *obj){
  if(obj == NULL){
    log_quit("SDS_Object is NULL in SDS_Object_SDSfile_exist()!\n");
  }

  if( (obj->index_files != NULL) || (obj->reorg_files != NULL))
    return SDS_TRUE;

  return SDS_FALSE;
}

SDS_Bool     SDS_Object_data_exist(SDS_Object    *obj){
  if(obj == NULL){
    log_quit("SDS_Object is NULL in SDS_Object_SDSfile_exist()!\n");
  }

  if( (obj->data_buffer != NULL) && (obj->data_buffer_filter != NULL))
    return SDS_TRUE;
  return SDS_FALSE;
}

int     SDS_Object_read_all_data(SDS_Object    *obj, SDS_Query_comm comm){
  if(obj == NULL){
    log_quit("SDS_Object is NULL in SDS_Object_read_all_data!\n");
  }
  
  SDS_File_type file_type = obj->file_type; 
  switch(file_type){
    case SDS_HDF5:
      hdf5_read_all(obj, comm);
      break;
    case SDS_BINARY:
      binary_read_all(obj, comm);
      break;
    case SDS_NETCDF:
      netcdf_read_all(obj, comm);
      break;
    default:
      log_quit("Not supported file formats in SDS_Object_read_all_data");
      break;
  }

  //Read the data first time and assign SDS_TRUE to all data filter
  obj->data_buffer_filter = malloc(sizeof(SDS_Bool)*obj->my_size);
  if(obj->data_buffer_filter == NULL)
    log_quit("Out of memory for SDS_Object_read_all_data()!");
  
  SDS_Offset i;
  for(i = 0 ; i < obj->my_size; i++){
    obj->data_buffer_filter[i] = SDS_TRUE;
  }

}


int     SDS_Object_write_all_data(SDS_Object  *obj, SDS_Bool merge, SDS_Query_comm comm){
  if(obj == NULL){
    log_quit("SDS_Object is NULL in SDS_Object_write_all_data!\n");
  }
  
  SDS_File_type file_type = obj->file_type; 
  switch(file_type){
    case SDS_HDF5:
      hdf5_write_all(obj,   merge,  comm);
      break;
    case SDS_BINARY:
      binary_write_all(obj, merge,  comm);
      break;
    case SDS_NETCDF:
      netcdf_write_all(obj, merge,  comm);
      break;
    default:
      log_quit("Not supported file formats in SDS_Object_write_all_data");
      break;
  }

}

int   SDS_Object_append_result(SDS_Object  *obj, SDS_Bool merge, SDS_Query_comm comm, char *fname){
  if(obj == NULL){
    log_quit("SDS_Object is NULL in SDS_Object_write_all_data!\n");
  }
  
  SDS_File_type file_type = obj->file_type; 
  switch(file_type){
    case SDS_HDF5:
      hdf5_append_all(obj,   merge,  comm, fname);
      break;
    case SDS_BINARY:
      binary_append_all(obj, merge,  comm, fname);
      break;
    case SDS_NETCDF:
      netcdf_append_all(obj, merge,  comm, fname);
      break;
    default:
      log_quit("Not supported file formats in SDS_Object_write_all_data");
      break;
  }
}


SDS_Bool  SDS_Object_get_data(SDS_Object    *obj, SDS_Offset index, SDS_Value_union *value){
  if(obj == NULL){
    log_quit("SDS_Object is NULL in SDS_Object_get_data()!\n");
  }
  if(obj->data_buffer != NULL){
    if(obj->data_buffer_filter[index] == SDS_TRUE){
      SDS_Value_union *data;
      data = obj->data_buffer;
      *value =  data[index];
    }
    return obj->data_buffer_filter[index];
  }else{
    return SDS_FALSE;
  }
}

int SDS_Object_set_data_filter(SDS_Object    *obj, SDS_Offset index, SDS_Bool bool){
  if(obj == NULL){
    log_quit("SDS_Object is NULL in SDS_Object_set_data_filter()!\n");
  }
  obj->data_buffer_filter[index] = bool;
  return 0;
}


int SDS_Object_set_compress(SDS_Object    *obj, SDS_Offset index, SDS_Bool bool){
  if(obj == NULL){
    log_quit("SDS_Object is NULL in SDS_Object_set_compress()!\n");
  }

}


int SDS_Object_release_buffer(SDS_Object    *obj){
  if(obj == NULL){
    log_quit("SDS_Object is NULL!\n");
  }

  if(obj->data_buffer == NULL){
    free(obj->data_buffer);
    obj->data_buffer = NULL;
  }

  if(obj->data_buffer_filter == NULL){
    free(obj->data_buffer_filter);
    obj->data_buffer_filter = NULL;
  }
  
  return 0;
}

int  SDS_Object_finalize(SDS_Object    *obj){
  if(obj == NULL){
    log_quit("SDS_Object is NULL!\n");
  }
  
  SDS_Object_release_buffer(obj);
  free(obj);
}

int  SDS_Object_print_id(SDS_Object    *obj){
  if(obj == NULL){
    log_quit("SDS_Object is NULL!\n");
  }
  
  switch(obj->file_type){
    case SDS_HDF5:
      printf(" HDF5 file, id %s \n", obj->dsetname);
      break;
    case SDS_BINARY:
      printf(" Binary file, id %s \n", obj->filename);
      break;
    case SDS_NETCDF:
      printf(" NETCDF file, id %s \n", obj->filename);
      break;
    default:
      printf(" UN-know file file, id %s \n", obj->filename);
      break;
  }
}


SDS_Index_type SDS_Object_get_index_type(SDS_Object    *obj){
  return obj->index_files->index_type;
}

SDS_Index_file  *SDS_Object_get_index_file(SDS_Object    *obj){
  return obj->index_files;
}

SDS_Offset     SDS_Object_get_data_size(SDS_Object    *obj){
  SDS_Offset  i, size;
  size = 0;
  for(i = 0; i < obj->my_size; i++){
    if(obj->data_buffer_filter[i] == SDS_TRUE)
      size = size + 1;
  }
  
  return size;
}


//Buf actually has the type 
void  SDS_Object_dup_data(SDS_Object    *obj, void *buf){
  
  SDS_Value_union *result_buf = obj->data_buffer;
  SDS_Offset i, j=0;
  
  
  switch(obj->data_type){
    case SDS_INT:
      for(i = 0; i < obj->my_size; i++){
        if(obj->data_buffer_filter[i] == SDS_TRUE){
          ((int *)buf)[j] = result_buf[i].i;
          j++;
        }
      }
      break;
    case SDS_DOUBLE:
      for(i = 0; i < obj->my_size; i++){
        if(obj->data_buffer_filter[i] == SDS_TRUE){
          ((double *)buf)[j] = result_buf[i].d;
          //printf("%f \n", ((double *)buf)[j]);
          j++;
        }
      }
      break;
    case SDS_FLOAT:
      for(i = 0; i < obj->my_size; i++){
        if(obj->data_buffer_filter[i] == SDS_TRUE){
          ((float *)buf)[j] = result_buf[i].f;
          j++;
        }
      }
      break;
    default:
      log_quit("Unknown data type in SDS_Object_dup_data()");
      break;
  }
}



void SDS_print_value(SDS_Data_type type, SDS_Value_union value){
    switch(type){
    case SDS_INT:
      printf("%d, ", value.i);
      break;
    case SDS_FLOAT:
      printf("%f, ", value.f);
      break;
    case SDS_DOUBLE:
      printf("%lf, ", value.d);
      break;
    default:     
      log_quit("Unknown data type in SDS_print_value; !");
      break;
  }

}

void           SDS_Object_print_data(SDS_Object    *obj){
  int i;
  SDS_Value_union *data;
  SDS_Offset  size;
  fflush(stdout);
  data = obj->data_buffer;
  size = obj->my_size;
  if(size > 10 ){
    for(i = 0 ; i < 5; i++){
      if(obj->data_buffer_filter[i] == SDS_TRUE){
        SDS_print_value(obj->data_type, data[i]);
      }else{
        printf("NULL, ");
      }
    }
    
    printf(" ... ");
    for(i = 5 ; i > 0; i--){
      if(obj->data_buffer_filter[size - i] == SDS_TRUE){
        SDS_print_value(obj->data_type, data[size - i]);
      }else{
        printf("NULL, ");
      }
    }
  }else{
    for(i = 0 ; i < size; i++){
      if(obj->data_buffer_filter[i] == SDS_TRUE){
        SDS_print_value(obj->data_type, data[i]);
      }else{
        printf("NULL, ");
      }
    }
  }
  
  printf("\n");
  fflush(stdout);
}



void  SDS_Object_fetch_sds_files(SDS_Object    *obj){

  
}




char *SDS_Object_get_name(SDS_Object    *obj){
  //The name for a SDS object
  return obj->dsetname;
}



