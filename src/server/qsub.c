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
#include "qsub.h"
#include <stdio.h>
#include <stdlib.h>
#include "sds-error.h"
#include "sds-common.h"
#include "sds-job.h"
/* 
 * Create the qsub script.  
 */
int create_qsub_script(char *service_name, char *orig_fname,  char *orig_group, char *orig_dset, char *reorg_fname, int process_count, char *script_fname, char *walltime, char *other_paramters, int job_type, int job_subtype){
  FILE *file_ptr;
  file_ptr =fopen(script_fname, "w");
  if (!file_ptr){
    printf("cann't create script file [%s]. \n", script_fname);
    return -1;
  }
 
  /*
  #PBS -q debug 
  #PBS -l nodes=16:ppn=8
  #PBS -l walltime=00:10:00
  #PBS -N my_job
  #PBS -e my_job.$PBS_JOBID.err
  #PBS -o my_job.$PBS_JOBID.out
  cd $PBS_O_WORKDIR
  mpirun -np 128 ./my_executable
  */
  //Only submit to debug queue, and walltime is 30min
  //TODO: choose queue and walltime more smartly
  fprintf(file_ptr, "#PBS  -q debug\n");
  fprintf(file_ptr, "#PBS  -l mppwidth=%d\n", process_count);  
  fprintf(file_ptr, "#PBS  -l walltime=%s\n", walltime);
  fprintf(file_ptr, "cd $PBS_O_WORKDIR\n");
  
  
  //Here we assume the file is HDF5.
  //Todo: for other format (e.g. binary) files 
  //fprintf(file_ptr, "aprun -n %d  %s -f %s  -g %s  -d %s -o %s  %s", process_count, service_name,  orig_fname, orig_group, orig_dset, reorg_fname, other_paramters);
    
    switch(job_type){
    case BUILD_INDEX:
      switch(job_subtype){
        case BITMAP_INDEX:
          fprintf(file_ptr, "aprun -n %d  %s -f %s  -p %s  -n %s -i %s  %s", process_count, service_name,  orig_fname, orig_group, orig_dset, reorg_fname, other_paramters);
          break;
        default:
          err_msg("Only support one index service: bitmap !");
          return NULL;
      }
      break;
    case REORGANIZE_DATA:
      switch(job_subtype){
        case SORT:
          fprintf(file_ptr, "aprun -n %d  %s -f %s  -p %s  -n %s -i %s  %s", process_count, service_name,  orig_fname, orig_group, orig_dset, reorg_fname, other_paramters);
          break;
        case TRANSFORM:
          fprintf(file_ptr, "aprun -n %d  %s -f %s  -g %s  -d %s -o %s  -t 5", process_count, service_name,  orig_fname, orig_group, orig_dset, reorg_fname);
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
    
  fclose(file_ptr);
  
  return  0;
}





