#include "sds-query.h"
#include "reorgnize.h"

/*
 *  Following example code starts a build index reorganizaion 
 *  on dataset x of the group /Step#0/ of the HDF5 file test.h5.
 */
int time_to_secs(char *time){
  int secs = 0;
  char *pch;
  
  pch =  strtok(time, ":");
  if(pch != NULL){
    secs = 60 * 60 * atoi(pch);
  }else{
    printf("Formart for -t, time of job running is wrong ! \n");
    exit(-1);
  }

  pch = strtok(NULL,":");
  if(pch != NULL){
    secs =secs +  60 * atoi(pch);
  }else{
    printf("Formart for -t, time of job running is wrong ! \n");
    exit(-1);
  }

  pch = strtok(NULL,":");
  if(pch != NULL){
    secs = secs + atoi(pch);
  }else{
    printf("Formart for -t, time of job running is wrong ! \n");
    exit(-1);
  }

  return secs;
}


int main(int argc, char *argv[]){
  SDS_Object     **x;
  char            *file  = "./testf.h5p";
  char            *group = "/testg";
  char            *dset  = "testd";
  int              index_type[1], reorg_type[1], cores[1], time[1];
  char            *augs[1]={"-r -l 10"};
  SDS_Reorg_status status;
  
  //Intialize the variable in conditional string
  x = malloc(sizeof(SDS_Object *));
  x[0] = SDS_Object_init(file, group, dset, SDS_HDF5, SDS_FLOAT);
  index_type[0]  = BITMAP_INDEX; //0: sort. 1: transform. 2: index
  reorg_type[0]  = NONE_REORG;
  cores[0] = 2;
  time[0]  = 10;
  //Start reorganization on x
  SDS_start_collection_reorg(x, index_type, reorg_type, cores, time, augs, 1, status);
  
  SDS_Object_finalize(x[0]);
  free(x);
}
