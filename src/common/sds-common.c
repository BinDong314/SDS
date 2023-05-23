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
#include "sds-common.h"
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <string.h>
#include <sys/stat.h>

//Does the dir path have a "/" as the last character
SDS_Bool dir_with_last_slash(char *dir){
  size_t len = strlen(dir);
  if((len > 0) && (dir[len-1] == '/'))
	return SDS_TRUE;
  return SDS_FALSE;
}

//Does the file exist.
SDS_Bool file_exist(char *filename){
  struct stat   buffer;   
  if(stat (filename, &buffer) == 0){
    return SDS_TRUE;
  }
  return SDS_FALSE;
}

//User must allocate memory for this
//http://linux.die.net/man/3/basename
int      split_path(char  *path, char *dir, char *filename){
  char *slash;
  int   dir_l;      /* Length of the dir, not including NUL.  */
  int   filename_l; /* Length of the dir, not including NUL.  */
  
  slash = strrchr (path, '/');
  if (slash == NULL){
    /* File is in the current directory.  */
    dir_l      = 0;
    filename_l = strlen(path);
    
    strncpy(dir, path, dir_l);
    dir[dir_l] = 0;

    //Replicate the file name
    strncpy(filename, path+dir_l, filename_l);
    filename[filename_l] = 0;
  }else{
    /* File is in somme directory.  */
    dir_l      = slash - path + 1;
    filename_l = strlen(path) - (dir_l + 1);
    
    strncpy(dir, path, dir_l);
    dir[dir_l] = 0;

    //Skip the "/"
    strncpy(filename, path+dir_l, filename_l);
    filename[filename_l] = 0;
  }
  
  return 0;
}


//Covert time in "hh:mm::ss" format to second (number)
int time_to_secs(char *time){
  int secs = 0;
  char *pch;
  
  printf("%s \n", time);
  pch =  strtok(time, ":");
  if(pch != NULL){
    secs = 60 * 60 * atoi(pch);
  }else{
    log_quit("Could not analyze [%s]!", time);
  }

  pch = strtok(NULL,":");
  if(pch != NULL){
    secs =secs +  60 * atoi(pch);
  }else{
    log_quit("Could not analyze [%s]!", time);
  }

  pch = strtok(NULL,":");
  if(pch != NULL){
    secs = secs + atoi(pch);
  }else{
    log_quit("Could not analyze [%s]!", time);
  }

  printf("%s \n", time);

  return secs;
}
