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
#include "sds-job.h"
#include "workqueue.h"
#include "data-reorganizer.h"
#include "sds-common.h"
#include "qsub.h"
#include "sbatch.h"
#include "sds-error.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>


/*
 * Try to start one reorganization work
 *    -Create the job script at first
 *    -Use popen function call to submit the job script
 *    -Job ID is read back from popen output stream
 */
int start_job(reorganization_job_t *job){
  int           status = 0;
  reorganizer_t reorganizer;
  char          reorganizer_file[MAX_FILE_NAME_LENGTH];
  char          group[MAX_FILE_NAME_LENGTH];
  char          dataset[MAX_FILE_NAME_LENGTH];
  char          input_file[MAX_FILE_NAME_LENGTH];
  char          output_file[MAX_FILE_NAME_LENGTH];
  char          attribute_file[MAX_FILE_NAME_LENGTH];
  char          script_file_name[MAX_FILE_NAME_LENGTH];
  char          qsub_command[MAX_FILE_NAME_LENGTH];
  char          wall_time[MAX_FILE_NAME_LENGTH];
  char         *job_id = NULL;
  FILE         *fpid;


  //Find reorganizer
  find_reorganizer(job->reorganization_type, &reorganizer);

  //Use pre-defined method to obtain different file name
  get_job_reorganizer_file_name(reorganizer_file, job);
  get_job_input_file_name(input_file,  job);
  get_job_group_name(group,  job);
  get_job_dataset_name(dataset,  job);
  get_job_output_file_name(output_file, job);
  get_job_script_file_name(script_file_name, job);
  get_job_attribute_file_name(attribute_file, job);

 
  //Get the number of core
  if(job->number_of_cores == 0){
    job->number_of_cores = estimate_core_requirement(input_file, output_file);
  }//Othewise, user has specified the cores

  //Get the walltime 
  if(job->time_secs == 0){
    estimate_wall_time(wall_time);
  }else{
    convert_wall_time(wall_time, job->time_secs);
  }

  //Todo: try to figure out the number of process for running the oreganization
  //Just use 100 for small test
  //
  // Tang: switch between qsub and sbatch based on environment variable
  char *job_submit_sys;
  job_submit_sys = getenv ("SLURM_ROOT");
  if (job_submit_sys!=NULL) {
    // We are using SLURM
    create_sbatch_script(reorganizer_file,input_file, group, dataset, output_file, job->number_of_cores, script_file_name, wall_time, job->other_parameters, job->job_type, job->job_subtype);

    //Submit the batch script and record the job id
    sprintf(qsub_command, "sbatch %s\n", script_file_name);
    if((fpid = popen(qsub_command, "r")) == NULL)
      return 1;
  }
  else {
    // We are using PBS
    create_qsub_script(reorganizer_file,input_file, group, dataset, output_file, job->number_of_cores, script_file_name, wall_time, job->other_parameters, job->job_type, job->job_subtype);

    //Submit the batch script and record the job id
    sprintf(qsub_command, "sbatch %s\n", script_file_name);
    if((fpid = popen(qsub_command, "r")) == NULL) 
      return 1; 
  }

 
  //Submit the batch script and record the job id
  job_id = malloc(MAX_FILE_NAME_LENGTH * sizeof(char)); 
  if(fgets(job_id, MAX_FILE_NAME_LENGTH, fpid) == NULL)  
    return 1;

  
  job->job_id = job_id;
  printf("JOB ID == %s ", job_id);
  
  pclose(fpid);


  log_msg("Start a reorganization: [%s, %s] -> [%s %s], reorganizer name [%s]. Jod ID = %s \n", job->original_file_info.file_path, job->original_file_info.file_name, job->reorganized_file_info.file_path, job->reorganized_file_info.file_name, reorganizer.reorganizer_name, job_id);

  
  return status;
}



/* 
 * It the stderr file size is zero, there is no error happening 
 * Otherwise, the reorganization finishes successfully.
 */
enum reorganization_work_status get_finish_state(char *stdout_file_name, char *stderr_file_name){
  struct stat st;
  
  //Todo: use "qstat job_id" to check the job is not in the queue first
  if (stat(stderr_file_name, &st) != 0)
    return FINISHIED_WITH_ERROR;
  
  printf("error size %d \n", st.st_size);
  if(st.st_size > 100){
    return FINISHIED_WITH_ERROR;
  }else{
    return FINISHIED_WITHOUT_ERROR;
  }
  
}

/* 
 * USE the stdout and stderr files generated by batch system to determine the job status
 *   If both files exist, the job is finished. Otherwise, it is still running
 *   When the job is finished and the size of stderr file is non-zero, job is finished with some errror
 *   When the job is finished and the size of stderr file is zero, job is finished withour errror. 
 * 
 * ToDO: more accurate method to determine the job status. 
 *   1) analyze the output ot qstat 
 *   2) Let reorganizer tell sds-server directly.
 */
enum reorganization_work_status check_job_status(reorganization_job_t *job){
  char stdout_file_name[MAX_FILE_NAME_LENGTH], stderr_file_name[MAX_FILE_NAME_LENGTH];
  enum reorganization_work_status status;
  char                    script_file_name[MAX_FILE_NAME_LENGTH];
  char                   *job_id;
  char                   *job_id_without_host;
  reorganizer_t           reorganizer;

 
  find_reorganizer(job->reorganization_type, &reorganizer);
  get_job_script_file_name(script_file_name,job);
  job_id = job->job_id;
  
  //Job id = number.hostname. Get the number part
  job_id_without_host = strsep(&job_id, ".");
  if(job_id_without_host == NULL){
    printf("Job id is wrong ! \n");
    return -1;
  }
  
  sprintf(stdout_file_name, "%s.o%s", script_file_name, job_id_without_host);
  sprintf(stderr_file_name, "%s.e%s", script_file_name, job_id_without_host);
  
  if ( (access(stdout_file_name, F_OK) != -1) && (access(stderr_file_name, F_OK) != -1) ) {
    //File exists, indicating it is finished
    status = get_finish_state(stdout_file_name, stderr_file_name);
    log_msg("Job %s is finished  !", job->job_id);
  }else{
    //Result file doesn't exist, indicating it is still unfinished
    status = RUNNING;   
    log_msg("Job %s is running !", job->job_id);
  }
  
  return status;
}



void get_job_reorganizer_file_name(char *file_name, reorganization_job_t *job){
  reorganizer_t           reorganizer;
  find_reorganizer(job->reorganization_type, &reorganizer);
  sprintf(file_name, "%s%s", reorganizer.reorganizer_execute_file_location, reorganizer.reorganizer_execute_file_name);
}

void get_job_input_file_name(char *file_name, reorganization_job_t *job){
  int l;
  l = strlen(job->original_file_info.file_path);
  if(job->original_file_info.file_path[l] == '/'){
    sprintf(file_name,       "%s%s", job->original_file_info.file_path, job->original_file_info.file_name);
  }else{
    sprintf(file_name,       "%s/%s", job->original_file_info.file_path, job->original_file_info.file_name);
  }
}

void get_job_output_file_name(char *file_name, reorganization_job_t *job){
  sprintf(file_name,      "%s%s", job->reorganized_file_info.file_path, job->reorganized_file_info.file_name);
}

void  get_job_group_name(char *group,  reorganization_job_t *job){
  sprintf(group,     "%s",   job->original_file_info.group_name);
}


void  get_job_dataset_name(char *dataset,  reorganization_job_t *job){
  //sprintf(dataset, "%s/%s",  job->original_file_info.group_name, job->original_file_info.dataset_name);
  sprintf(dataset, "%s",  job->original_file_info.dataset_name);
}


void get_job_script_file_name(char *file_name, reorganization_job_t *job){
  reorganizer_t           reorganizer;
  find_reorganizer(job->reorganization_type, &reorganizer);
  sprintf(file_name, "%s-%s.pbs", job->original_file_info.file_name, reorganizer.reorganizer_execute_file_name);
}

void get_job_attribute_file_name(char *file_name, reorganization_job_t *job){
  reorganizer_t           reorganizer;
  find_reorganizer(job->reorganization_type, &reorganizer);
  //Put the attribute file and reorganized file together
  sprintf(file_name,      "%s%s.attribute", job->reorganized_file_info.file_path, job->reorganized_file_info.file_name);
  //sprintf(file_name, "%s-%s.pbs.attribute", job->original_file_info.file_name, reorganizer.reorganizer_execute_file_name);
}


void  initialize_reorganization_job_list(reorganization_job_list_t *job_list){
  memset(job_list, 0 , sizeof(reorganization_job_list_t));
  job_list->job_count = 0;
}

void  add_to_reorganization_job_list(reorganization_job_list_t *reorganization_job_list, reorganization_job_t *job){
  LL_ADD(job, reorganization_job_list->jobs);
}


/* 
 *Todo: find a method to figure out the number of cores needed by a reorganization 
 */
int   estimate_core_requirement(char *intput_file, char *output_file){
  //For test
  return 4000;
}


/*
 *Todo: one way to figure out  proper wall time 
 */
void  estimate_wall_time(char *wall_time){
  //For test
  sprintf(wall_time, "%d:%d:%d",00, 02, 00);
}

void convert_wall_time(char *wall_time, int time_secs){
  int h, m, s;
  h = time_secs/(60*60);
  m = (time_secs - h * 60 * 60) / 60;
  s = time_secs%60;

  printf("time: %d, %d:%d:%d", time_secs, h, m, s);
  
  sprintf(wall_time, "%d:%d:%d",h, m, s);
      
}
