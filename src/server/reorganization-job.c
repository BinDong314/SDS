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

#include "reorganization-job.h"
#include "workqueue.h"
#include "data-reorganizer.h"
#include "sds-common.h"
#include "qsub.h"
#include "sbatch.h"
#include "sds-error.h"
#include  "mpirun.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


#define READ 0
#define WRITE 1

//
//Implement another popen2 to return Childen's pid 
//"infp and outfp" could be used to communicate with child process
//"command" is the command string with parameter
//
pid_t popen2(const char *command, int *infp, int *outfp)
{
    int   p_stdin[2], p_stdout[2];
    pid_t pid;

    //Create the pipe between parent and client
    if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0)
        return -1;

    pid = fork();

    if (pid < 0){
      log_quit("Failed to create new thread in popen2 !");
      return pid;
    }else if (pid == 0){
      close(p_stdin[WRITE]);
      dup2(p_stdin[READ], READ);
      close(p_stdout[READ]);
      dup2(p_stdout[WRITE], WRITE);
      //Run the command 
      execl("/bin/sh", "sh", "-c", command, NULL);
      perror("execl");
      exit(1);
    }
    
    if (infp == NULL)
      close(p_stdin[WRITE]);
    else
      *infp = p_stdin[WRITE];
    
    if (outfp == NULL)
      close(p_stdout[READ]);
    else
      *outfp = p_stdout[READ];
    
    return pid;
}


//This is used to start job for single-node system
int   start_synch_job(reorganization_job_t *job){
  pid_t         child_pid;
  int           infp, outfp; //Not used now
  char         *mpirun_command;
  char          service_name[MAX_FILE_NAME_LENGTH];
  int           statval, ret;
  
  //Find the name of SDS Service
  get_job_reorganizer_file_name(service_name, job);

  //Create the command to run
  mpirun_command = create_mpirun_command(service_name, job->original_file_info.file_name, job->original_file_info.group_name, job->original_file_info.dataset_name, job->reorganized_file_info.file_name, job->number_of_cores, job->other_parameters, job->job_type, job->job_subtype);
  
  printf("job->job_type %d, job->job_subtype %d \n", job->job_type, job->job_subtype);
  
  log_msg("Command to run job: ");
  log_msg("     %s", mpirun_command);


  //Start run command with mpirun and obtain the pid
  child_pid = popen2(mpirun_command, &infp, &outfp);
  
  //Wait the client to finish
  printf("Waiting for SDS Service to finish PID %d:\n", getpid());
  waitpid(child_pid, &statval, WUNTRACED 
#ifdef WCONTINUED       /* Not all implementations support this */
           | WCONTINUED
#endif
           );
  
  if(WIFEXITED(statval)){
    ret = WEXITSTATUS(statval);
    if(ret == 0){
      return 0;
    }else{
      return -1;
    }
  }else{
    log_msg("Running the SDS Job with error. !\n");
    return -1;
  }  
  
  return 0;
}

/*
 * Try to start one reorganization work
 *    -Create the job script at first
 *    -Use popen function call to submit the job script
 *    -Job ID is read back from popen output stream
 */
int start_batch_job(reorganization_job_t *job){
  int           status = 0;
  reorganizer_t reorganizer;
  char          reorganizer_file[MAX_FILE_NAME_LENGTH];
  char          group[MAX_FILE_NAME_LENGTH];
  char          dataset[MAX_FILE_NAME_LENGTH];
  char          input_file[MAX_FILE_NAME_LENGTH];
  char          output_file[MAX_FILE_NAME_LENGTH];
  //char          attribute_file[MAX_FILE_NAME_LENGTH];
  char          script_file_name[MAX_FILE_NAME_LENGTH];
  char          qsub_command[MAX_FILE_NAME_LENGTH];
  char          wall_time[MAX_FILE_NAME_LENGTH];
  char         *job_id = NULL;
  FILE         *fpid;


  //Find reorganizer
  find_reorganizer(job->job_type, job->job_subtype, &reorganizer);

  //Use pre-defined method to obtain different file name
  get_job_reorganizer_file_name(reorganizer_file, job);
  get_job_input_file_name(input_file,  job);
  get_job_group_name(group,  job);
  get_job_dataset_name(dataset,  job);
  get_job_output_file_name(output_file, job);
  get_job_script_file_name(script_file_name, job);
  //get_job_attribute_file_name(attribute_file, job);

 
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
  //create_qsub_script(reorganizer_file, input_file, group, dataset,  output_file, job->number_of_cores, attribute_file, script_file_name, wall_time, job->reorganization_type, job->other_parameters);
  //int create_qsub_script(char *service_name, char *orig_fname,  char *orig_group, char *orig_dset, char *reorg_fname, int process_count, char *script_fname, char *walltime, char *other_paramters)
  //


  // Tang: switch between qsub and sbatch based on environment variable
  char *job_submit_sys;
  job_submit_sys = getenv("SLURM_ROOT");
  fprintf(stderr, "Job system: %s\n", job_submit_sys);
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
    sprintf(qsub_command, "qsub %s\n", script_file_name);
    if((fpid = popen(qsub_command, "r")) == NULL) 
      return 1; 
  }
  
  job_id = malloc(MAX_FILE_NAME_LENGTH * sizeof(char)); 
  if(fgets(job_id, MAX_FILE_NAME_LENGTH, fpid) == NULL)  
    return 1;
  //The return of sbatch is as blow. So, We need to skip the header string
  //  to find the id
  //
  //edison bin $ sbatch testf.h5-openmsi_transposer.pbs
  //   Submitted batch job 813145
  //
  if (job_submit_sys!=NULL) {
    int header_lenth= strlen("Submitted batch job ");
    job_id = job_id+header_lenth;
    job_id[strlen(job_id) - 1] = '\0';
  }

  job->job_id = job_id;
  pclose(fpid);

  log_msg("Start a reorganization: [%s] -> [%s], reorganizer name [%s]. Jod ID = %s \n", input_file, output_file, reorganizer.reorganizer_name, job_id);

  
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

 
  find_reorganizer(job->job_type, job->job_subtype,  &reorganizer);
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
  find_reorganizer(job->job_type, job->job_subtype, &reorganizer);
  sprintf(file_name, "%s%s", reorganizer.reorganizer_execute_file_location, reorganizer.reorganizer_execute_file_name);
}

void get_job_input_file_name(char *file_name, reorganization_job_t *job){
  int l;
  sprintf(file_name,       "%s", job->original_file_info.file_name);
  
}

void get_job_output_file_name(char *file_name, reorganization_job_t *job){
  sprintf(file_name,      "%s", job->reorganized_file_info.file_name);
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
  find_reorganizer(job->job_type, job->job_subtype, &reorganizer);
  sprintf(file_name, "%s-%s.pbs", job->original_file_info.file_name, reorganizer.reorganizer_execute_file_name);
}

//void get_job_attribute_file_name(char *file_name, reorganization_job_t *job){
//  reorganizer_t           reorganizer;
//  find_reorganizer(job->job_type, job->job_subtype, &reorganizer);
  //Put the attribute file and reorganized file together
//  sprintf(file_name,      "%s%s.attribute", job->reorganized_file_info.file_path, job->reorganized_file_info.file_name);
  //sprintf(file_name, "%s-%s.pbs.attribute", job->original_file_info.file_name, reorganizer.reorganizer_execute_file_name);
//}


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
