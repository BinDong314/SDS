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
#include "sds-index-file.h"

SDS_Index_file  *SDS_Index_file_init(char*filename, char *group, char *dsetname, SDS_File_type file_type, SDS_Index_type index_type){
  SDS_Index_file  *new_index_file;
  int len;
  
  new_index_file = malloc(sizeof(SDS_Index_file));
  if(filename == NULL){
    log_quit("File name must be provided to SDS_Index_file_init !\n");
  }else{
    len = strlen(filename);
    new_index_file->filename = malloc(len * sizeof(char));
    strcpy(new_index_file->filename, filename);
  }

  if(file_type == SDS_HDF5 ){
    if(group == NULL || dsetname == NULL ){
      log_quit("Group and dataset names must be provided to SDS_Index_file_init for a HDF5 file !\n");
    }
      
    len = strlen(dsetname);
    new_index_file->dsetname = malloc(len * sizeof(char));
    strcpy(new_index_file->dsetname, dsetname);
    
    len = strlen(group);
    new_index_file->group = malloc(len * sizeof(char));
    strcpy(new_index_file->group, group);
        
  }
  
  new_index_file->file_type  = file_type;
  new_index_file->index_type = index_type;

  return new_index_file;
}

int SDS_Index_file_finalize(SDS_Index_file  *index_file){
  if(index_file != NULL)
    if(index_file->file_type == SDS_HDF5 ){
      if(index_file->dsetname != NULL)
        free(index_file->dsetname);
      if(index_file->group != NULL)
        free(index_file->group);
    }

  if(index_file->filename)
    free(index_file->filename);
  
  free(index_file);
}

