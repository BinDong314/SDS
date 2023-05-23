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
 * This file defines the method to build command for mpirun
 *  Author: Bin Dong <dbin at lbl.gov >
 *  Copyright 2015 the Regents of the University of California
 * 
 */

#include "mpirun.h"
#include "sds-job.h"
char *create_mpirun_command(char *service_name, char *orig_fname,  char *orig_group, char *orig_dset, char *reorg_fname, int process_count, char *other_paramters, int job_type, int job_subtype){
  char *cmd_buf;
  int   cmd_size = 0;
  int   extral_len=255; //for the "mpirun -n ..."
  
  //Get the size of the command buf
  cmd_size = extral_len + strlen(service_name)+strlen(orig_fname)+strlen(orig_group)+strlen(orig_dset)+strlen(reorg_fname)+strlen(other_paramters);
  cmd_buf = malloc(cmd_size * sizeof(char));
  
  //mpirun -n "" exect
  //Here we assume the file is HDF5.
  //Todo: for other format (e.g. binary) files 
  switch(job_type){
    case BUILD_INDEX:
      switch(job_subtype){
        case BITMAP_INDEX:
          sprintf(cmd_buf, "mpirun -n %d  %s -f %s  -p %s  -n %s -i %s  %s", process_count, service_name,  orig_fname, orig_group, orig_dset, reorg_fname, other_paramters);
          break;
        default:
          err_msg("Only support one index service: bitmap !");
          return NULL;
      }
      break;
    case REORGANIZE_DATA:
      switch(job_subtype){
        case SORT:
          sprintf(cmd_buf, "mpirun -n %d  %s -f %s  -p %s  -n %s -i %s  %s", process_count, service_name,  orig_fname, orig_group, orig_dset, reorg_fname, other_paramters);
          break;
        case TRANSFORM:
          sprintf(cmd_buf, "mpirun -n %d  %s -f %s  -g %s  -d %s -o %s  -t 5", process_count, service_name,  orig_fname, orig_group, orig_dset, reorg_fname);
          break;
        default:
          err_msg("Only support two reorganization services: sorting and transforming !");
          return NULL;
      }
      break;
    default:
      err_msg("Only support two services: build index and reorganize data !");
      return NULL;
  }
  
  return cmd_buf;
}
