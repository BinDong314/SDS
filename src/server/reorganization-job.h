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
 * This file defines the two methods to run SDS Job
 *  One is for batch system and it is async
 *  Another one is for single node and it is sync
 *
 *  Author: Bin Dong <dbin at lbl.gov >
 *  Copyright 2015 the Regents of the University of California
 * 
 */

#ifndef __SDS_JOB_H__
#define __SDS_JOB_H__

#include "sds-common.h"

enum job_type{
  BUILD_INDEX               = 0,
  REORGANIZE_DATA           = 1,
  RUN_ANALYZE               = 2,
};


typedef enum reorganization_work_status{
  SUBMITTED               = 0,
  WAIT                    = 1,
  RUNNING                 = 2,
  FINISHIED_WITHOUT_ERROR = 3,
  FINISHIED_WITH_ERROR    = 4
}reorganization_work_status;

typedef struct reorganization_file_info{
  char                       *file_name;
  char                       *group_name;
  char                       *dataset_name;
  SDS_Data_type               data_type;           //Type of the data to handle
  SDS_File_type               file_type;           //Type of the file containing the variable

}reorganization_file_info_t;

typedef struct reorganization_job {
  struct reorganization_job       *prev;
  struct reorganization_job       *next;
  reorganization_file_info_t       original_file_info;
  reorganization_file_info_t       reorganized_file_info;
  int                              job_type;     //Three types: build index, reorganize data, run an analysis program 
                                                 // Defined as job_type above
  int                              job_subtype; // Sub type (defined by SDS_Reorg_type and SDS_Index_type in ../common.h)
  char                            *job_id;
  int                              number_of_cores;
  int                              time_secs;
  char                             other_parameters[MAX_FILE_NAME_LENGTH];
  enum reorganization_work_status  status;
}reorganization_job_t;

typedef struct reorganization_job_list{
  struct reorganization_job       *jobs;
  int                              job_count;
}reorganization_job_list_t;


void  initialize_reorganization_job_list(reorganization_job_list_t *job);
void  add_to_reorganization_job_list(reorganization_job_list_t *reorganization_job_list, reorganization_job_t *job);
//This is used to start job for batch system 
int   start_batch_job(reorganization_job_t *job);
//This is used to start job for single-node system
int   start_synch_job(reorganization_job_t *job);

enum reorganization_work_status check_job_status(reorganization_job_t *job);

void  get_job_reorganizer_file_name(char *file_name, reorganization_job_t *job);
void  get_job_input_file_name(char *file_name, reorganization_job_t *job);
void  get_job_output_file_name(char *file_name, reorganization_job_t *job);
void  get_job_script_file_name(char *file_name, reorganization_job_t *job);
void  get_job_attribute_file_name(char *file_name, reorganization_job_t *job);
void  get_job_group_name(char *dataset,  reorganization_job_t *job);
void  get_job_dataset_name(char *group,  reorganization_job_t *job);
int   estimate_core_requirement(char *input_file, char *output_file);
void  estimate_wall_time(char *wall_time);
void  convert_wall_time(char * wall_time, int time_secs);
#endif
