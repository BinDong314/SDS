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
#ifndef __SDS_INDEX_FILE_H__
#define __SDS_INDEX_FILE_H__

#include "sds-common.h"

typedef struct SDS_Index_file{
  SDS_File_type                file_type;           //The type of file containing the index
  SDS_Data_type                data_type;           //Type of the data to handle
  char                        *filename;            //Name of the file containing the index
  char                        *group;               //If file_type id HDF5, it might contain group and dataset
  char                        *dsetname;            //

  SDS_Index_type               index_type;           //Type of the data to handle
  char                        *parameters;           //Other paramteres 
}SDS_Index_file;


SDS_Index_file  *SDS_Index_file_init(char*filename, char *group, char *dsetname, SDS_File_type file_type, SDS_Index_type index_type);
int              SDS_Index_file_finalize(SDS_Index_file  *index_file);



#endif
