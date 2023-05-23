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
#ifndef __SDS_EXTERNAL_VOL_H__
#define __SDS_EXTERNAL_VOL_H__
#define H5F_FRIEND

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include "hdf5.h"
//#include "mpi.h"
#include <string.h>

#define H5_REQUEST_NULL NULL

typedef struct H5VL_sds_external_t {
  void           *under_object;
  const char     *dir_name;
  const char     *file_name;
  const char     *group_name;
  const char     *dataset_name;
  hid_t           under_plugin;
  unsigned        flags;
  hid_t           fapl_id; 
  hid_t           fxpl_id; 
  hid_t           dapl_id;
  hid_t           dxpl_id; 
  hid_t           fcpl_id;
  hid_t           req;
  H5VL_loc_params_t loc_params;
} H5VL_sds_external_t;

#define SDS_EXTERNAL 506

extern const H5VL_class_t H5VL_sds_external_g;
extern hid_t native_plugin_id;

#endif
