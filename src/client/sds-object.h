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

#ifndef __SDS_OBJECT_H__
#define __SDS_OBJECT_H__

#include "sds-common.h"
#include "sds-array.h"
#include "sds-index-file.h"
#include "sds-reorg-file.h" 
#include "sds-histgram.h"
#include "message.protoc.pb-c.h"

//#include "sds-hdf5.h"
//#include "sds-binary.h"
//#include "sds-netcdf.h"
 


//Wrapper struct for the variable used within a query
typedef struct SDS_Object{
  //Following items are called ID 
  char                        *dsetname;            //Name of the dataset for HDF5 file; NULL for binary format. 
  char                        *group;               //Group path for HDF5 file; NULL for binary format
  char                        *filename;            //Name of the file containing name and group for HDF5 file;
                                                    //For binary file, it can be used as a variable in condition string) 
  SDS_Data_type                data_type;           //Type of the data to handle
  SDS_File_type                file_type;           //Type of the file containing the variable
                                            
  //Following items are optional for initialization, SDS use them for cache
  SDS_Bool                     coodinate_space;     //SDS_FALSE by default. 
  SDS_Array_attribute          array_attribute;     //Dimension and length of each dimension, filled by user or SDS server
  SDS_Index_file              *index_files;         //Index file (bitmap index, sorted index) to use,    filled by user or SDS server
  SDS_Reorg_file              *reorg_files;         //Reorganized file (sorted, transposed, etc) to use, filled by user or SDS server
  ResponseTraceData           *ht_reorg_file;       // Tang: reorganized layout metadata
  void                        *data_buffer;         //Buffer for the data in memory: row major linear for n-D array 
  SDS_Bool                    *data_buffer_filter;  //Mark the selected data SDS_TRUE or SDS_FALSE 
  SDS_Bool                     sparse_array;        //
  SDS_Offset                   my_size;             //Only 1D size 
  SDS_Offset                   my_offset;           //Only 1D offset
  void                        *other;               //Other information, cache, can be added.   
}SDS_Object;


SDS_Object    *SDS_Object_init(char*filename, char *group, char *dsetname, SDS_File_type file_type, SDS_Data_type data_type);
int            SDS_Object_finalize(SDS_Object    *obj);

int            SDS_Object_release_buffer(SDS_Object    *obj);

int            SDS_Object_set_array_attribute(SDS_Object  *obj, int dims, long int *length);
int            SDS_Object_set_index_file(SDS_Object  *obj, SDS_Index_file  *files);
int            SDS_Object_set_reorganized_file(SDS_Object  *obj, SDS_Reorg_file  *file);
int            SDS_Object_set_coodinate_space(SDS_Object   *obj, SDS_Bool bool);


int              SDS_Object_get_id(SDS_Object    *obj); 
int              SDS_Object_print_id(SDS_Object    *obj); 
SDS_Index_file  *SDS_Object_get_index_file(SDS_Object    *obj);
int              SDS_Object_get_reorganized_file(SDS_Object    *obj, SDS_Reorg_file  *file); 
SDS_Data_type    SDS_Object_get_data_type(SDS_Object    *obj);
SDS_File_type    SDS_Object_get_file_type(SDS_Object    *obj);
SDS_Index_type   SDS_Object_get_index_type(SDS_Object    *obj); 
SDS_Offset       SDS_Object_get_data_size(SDS_Object    *obj);
void             SDS_Object_dup_data(SDS_Object    *obj, void *buf);


void           SDS_Object_print_data(SDS_Object    *obj);

SDS_Bool       SDS_Object_SDSfile_exist(SDS_Object    *obj);

int            SDS_Object_read_all_data(SDS_Object  *obj, SDS_Query_comm comm);
int            SDS_Object_write_all_data(SDS_Object *obj, SDS_Bool merge, SDS_Query_comm comm);

SDS_Bool       SDS_Object_get_data(SDS_Object    *obj, SDS_Offset index, SDS_Value_union *value);
int            SDS_Object_set_data_filter(SDS_Object    *obj, SDS_Offset index, SDS_Bool bool);
int            SDS_Object_set_compress(SDS_Object    *obj, SDS_Offset index, SDS_Bool bool);

void           SDS_print_value(SDS_Data_type type, SDS_Value_union value);
void           SDS_Object_fetch_sds_files(SDS_Object    *obj); 

char           *SDS_Object_get_name(SDS_Object    *obj);
#endif
