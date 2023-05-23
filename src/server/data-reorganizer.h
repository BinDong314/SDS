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
 * This file defines the mapping between index/reorganization type and its corresponding service code 
 *
 *  Author: Bin Dong <dbin at lbl.gov >
 *  Copyright 2015 the Regents of the University of California
 * 
 */

#ifndef  __DATA_REORGANIZER_H__
#define  __DATA_REORGANIZER_H__

//#include "reorganization-job-list.h"
/* #include "transformer.h" */
/* #include "user_organizer.h" */
/* #include "sorter.h" */
/* #include "index.h" */
#include "sds-common.h"
#include "reorganization-job.h"
#include <stdlib.h>


typedef struct reorganizer{
  int      type;     //Defined by job_type in reorganization-job.h
  int      subtype;  //Defined by SDS_Index_type and SDS_Reorg_type in sds-common.h
  char    *reorganizer_name;
  char    *reorganizer_execute_file_location;
  char    *reorganizer_execute_file_name;
  //  void   (* reorganizer_callback_function)(reorganization_file_info_t *, reorganization_file_info_t *); //These call-back functions are not implemented right now
}reorganizer_t;


static reorganizer_t reorganizer_array[8] =
{
  {
    REORGANIZE_DATA,
    SORT,
    "VPIC Energy Sorter",
    "./",
    "vpic_energy_sorter",
    //    sorter_callback,
  },
  {
    REORGANIZE_DATA,
    TRANSFORM,
    "OpenMSI Transposer",
    "./",
    "openmsi_transposer",
    //  transformer_callback,
  },
  {
    BUILD_INDEX,
    BITMAP_INDEX,
    "Indexer",
    "./",
    "fq_index_builder",
    // index_callback,
  },
  {
    REORGANIZE_DATA,
    MDBIN,
    "MDBIN",
    "./",
    "",
    // index_callback,
  },
  {
    REORGANIZE_DATA,
    SFC,
    "Space Filling Curve",
    "./",
    "sfc",
    // index_callback,
  },
  {
    REORGANIZE_DATA,
    CONCATENATION,
    "Concatenation",
    "./",
    "concatenation",
    // index_callback,
  },
  {
    USER_DEFINED,
    0,
    "User Defined Reorganizer",
    NULL,
    NULL,
    // execute_user_defined_callbak,
  }
};

void  find_reorganizer(int type, int sub_type, reorganizer_t *reorganizer);

#endif
