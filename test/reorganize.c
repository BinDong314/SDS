/* 
 * This is example of start a reorganization manually
 *  reorganize   -f filename -p path to file -d data set -t type of reorganization  
 *
 */

#include <unistd.h>
#include "sds-query.h"
#include "reorgnize.h"
#include "string.h"

#define REORGANIZATION_TYPE 0
#define INDEX_TYPE          1


//Please refer to "enum reorganization_type" defined in file common/sds-common.h
int organization_type(char *typename, int *type, int *sub_type){
  if(strcmp(typename, "sort")==0 || strcmp(typename, "s") == 0 ){
    *sub_type= SORT;
    *type = REORGANIZATION_TYPE;
  }else if(strcmp(typename, "transform")==0 || strcmp(typename, "t") == 0){
    *sub_type= TRANSFORM;
    *type = REORGANIZATION_TYPE;
  }else if(strcmp(typename, "index")==0 || strcmp(typename, "i") == 0){
    *sub_type= BITMAP_INDEX;
    *type = INDEX_TYPE;
  }else{
    printf("Unsupported organization type. Current supported includes sort(s), transfor(t), and index(i) !\n");
    exit(-1);
  }
  return 0;
}


//Convert time hh:minitue:secons to decimal seconds 
//TODO: checnk the valid of string: time
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

void  print_help(){
   char *msg="Usage: %s [OPTION] \n\
      	  -h help (--help)\n\
          -f name of the file (only HDF5 file in current version) \n\
          -g group path within HDF5 file to data set \n\
          -d name of the dataset within HDF5 file to be reorganized \n\
          -o organization type: sort(s), transform(t), index(i). \n\
          -n number of cores for reorganization job \n\
          -t wall time for reorganization job (00:00:00) \n\
          -e extended parameters for reoganization \n\
          Example:   ./reorganize -f ./testf.h5p  -g /testg -d testd -n 24 -o i -t \"00:02:00\" \n";
   fprintf(stdout, msg, "reorganize");
}

int main(int argc, char *argv[]){
  char           *file, *group, *dataset, organization[255], time_str[255];
  char           *e_parameters; 
  int             has_e_parameters = 0;
  int             numberofcores; 
  SDS_Reorg_status status;
  static const char *options="hf:g:d:o:n:t:e:";
  extern char *optarg;
  int c;
  
  while ((c = getopt(argc, argv, options)) != -1) {
    switch (c) {
      case 'f':
      case 'F': 
        //strcpy(file, optarg); 
        file=strdup(optarg);
        break;
      case 'g':
      case 'G': 
        //strcpy(group, optarg); 
        group = strdup(optarg);
        break;
      case 'd':
      case 'D': 
        //strcpy(dataset, optarg); 
        dataset = strdup(optarg);
        break;
      case 'o':
      case 'O':
        strcpy(organization, optarg);
        break;
      case 'h':
      case 'H':
        print_help();
        return 0;
      case 'n':
      case 'N':
        numberofcores = atoi(optarg);
        break;
      case 't':
      case 'T':
        strcpy(time_str, optarg);
        break;
      case 'e':
      case 'E':
        e_parameters=strdup(optarg);
        has_e_parameters = 1;
        //strcpy(extended_paramters, optarg);
        break;
      default: break;
    } // switch
  } // while
  
  int type, sub_type;
  int time_secs;
  organization_type(organization, &type, &sub_type);
  time_secs = time_to_secs(time_str);
  
  //Todo: test the maximum of cores 
  if(numberofcores == 0 || numberofcores < 0){
    printf("The number of cores specified is wrong %d !", numberofcores);
    exit(-1);
  }
  
  if (file[0] == '\0' || group[0] == '\0' || dataset[0] == '\0'){
    printf("The path, filename, group, and dataset should be specified !\n ");
    exit(-1);
  }
 

  SDS_Object     **x;
  int              index_type[1], reorg_type[1], cores[1], time[1];
  char            *augs[1];
  x = malloc(sizeof(SDS_Object *));
  x[0] = SDS_Object_init(file, group, dataset, SDS_HDF5, SDS_UNKNOWN_TYPE);
  if(type == INDEX_TYPE){
    index_type[0]  = sub_type; 
    reorg_type[0]  = NONE_REORG;
  }else{
    index_type[0]  = NONE_INDEX; //0: sort. 1: transform. 2: index
    reorg_type[0]  = sub_type;
  }
  cores[0] = numberofcores;
  time[0]  = time_secs;
  if(has_e_parameters){
    augs[0] = e_parameters;
  }else{
    augs[0] = " ";
  }

  
  //Start reorganization on x
  SDS_start_collection_reorg(x, index_type, reorg_type, cores, time, augs, 1, status);
  
  SDS_Object_finalize(x[0]);
  free(x);


}


