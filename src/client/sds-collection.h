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

#ifndef __SDS_COLLECTION_H__
#define __SDS_COLLECTION_H__

#include "sds-object.h"
#include "sds-condition-tree.h"

//struct for SDS collection
//For possible extension, each collection has a sub collection
typedef struct SDS_Collection{
  int                          object_cursor; 
  int                          object_count;
  SDS_Object                 **object_array;
}SDS_Collection;

//Create an empty collection
SDS_Collection  *SDS_Collection_init(int object_count);
//Release buffer and collection itself collection
int              SDS_Collection_finalize(SDS_Collection  *coll);
//Release only buffer of the collection
int              SDS_Collection_release_buffer(SDS_Collection  *coll);

//Create am collection based on HDF5 group, each HDF5 dataset (not including subgroup)in the group will be turned into a SDS object
SDS_Collection  *SDS_Collection_init_HDF5_group(char *hdf5_filename, char *hdf5_group);
//Create am collection based on NetCDF group, each NecCDF variable (not including subgroup) in the group will be turned into a SDS object
SDS_Collection  *SDS_Collection_init_NetCDF_group(char *netcdf_filename, char *netcdf_group);
//Merge two collection into the same one
SDS_Collection  *SDS_Collection_merge(SDS_Collection  *coll1, SDS_Collection  *coll2);

//Append one obj to the end of the collection
int             SDS_Collection_append(SDS_Collection  *coll, SDS_Object *obj);
//Set the nth  objects 
int             SDS_Collection_set(SDS_Collection  *coll, SDS_Object *obj, int index);
//Get the nth objects  
SDS_Object     *SDS_Collection_get_object(SDS_Collection  *coll, int index);
//Get the number of objects  
int             SDS_Collection_get_size(SDS_Collection  *coll);
//List all objects  
int              SDS_Collection_list_objects(SDS_Collection  *coll);
//List objct data 
int              SDS_Collection_list_data(SDS_Collection  *coll, int index);
//Check whether we have some sds files at SDS Server
//Instead of checking for each object with multiple communication, we choose to check
//the whole Collection  with a single communication 
int              SDS_Collection_fetch_sds_files(SDS_Collection  *coll);
SDS_Bool         SDS_Collection_SDSfile_exist(SDS_Collection  *coll);
int              SDS_Collection_SDSfile_read(SDS_Collection  *coll, int *index_covered, SDS_Condition_tree *cond_tree);


//Select the object inside the collection 
SDS_Collection  *SDS_Collection_project(SDS_Collection  *coll, int *selected_obj_indexs, SDS_Query_comm comm);
//Select each boject based on the cond_tree 
SDS_Collection  *SDS_Collection_select(SDS_Collection  *coll, char *cond_str, SDS_Query_comm comm);
//User-defined code on a collection
SDS_Collection  *SDS_Collection_udf(SDS_Collection  *coll, void *udf_code_pointer, SDS_Query_comm comm);
//Eq-join two collections based on their key object index
SDS_Collection  *SDS_Collection_join(SDS_Collection  *coll1, int key_obj_index1, SDS_Collection  *coll2, int key_obj_index2, SDS_Query_comm comm);
//Build histgram on coll
SDS_Collection  *SDS_Collection_histgram(SDS_Collection  *coll, void *hist_par_p, SDS_Query_comm comm);
SDS_Collection  *SDS_Collection_build_histgram_index(SDS_Collection    *coll,  SDS_Histgram_operand *hist_par_p);
SDS_Collection  *SDS_Collection_build_histram_fullscan(SDS_Collection  *coll,  SDS_Histgram_operand *hist_par_p);
SDS_Collection  *SDS_Collection_build_histgram_memory(SDS_Collection   *coll,  SDS_Histgram_operand *hist_par_p);

int SDS_Collection_read_sort(SDS_Collection  *coll, int *index_covered, SDS_Condition_tree *cond_tree);
int SDS_Collection_read_bitmap(SDS_Collection  *coll, int *index_covered, SDS_Condition_tree *cond_tree, char *cond_str);
int SDS_Collection_read_mdbin(SDS_Collection  *coll, int *index_covered, SDS_Condition_tree *cond_tree);

SDS_Index_type SDS_Collection_SDSfile_type(SDS_Collection  *coll);
int  SDS_Collection_read_memory_fullscan(SDS_Collection  *coll, SDS_Condition_tree *cond_tree);

SDS_Index_type SDS_Collection_index_type(SDS_Collection  *coll);

int  SDS_Collection_save_results(SDS_Collection  *coll, SDS_Bool merge, SDS_Query_comm comm);

#endif
