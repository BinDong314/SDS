#include "hdf5.h"
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define NAME_MAX 255


int getvalue(char *query, hsize_t *offset, hsize_t *size){
  char    *token;
  hsize_t  v1, v2; 
  int i;
  
  //":"  no specifed range
  if(strlen(query) == 1){
    return -1;
  }
  
  //"v1:v2": 
  token = strtok(query, ":");
  v1 = atoi(token);

  token = strtok(NULL, ":");
  v2 = atoi(token);


  
  if(v2 < v1 ){
    printf("The query string has error ! \n"); 
    exit(-1);
  }

  //printf("%d, %d \n", v1, v2);
  *offset = v1;
  *size   = v2 - v1;
  
  //Todo: other query condition needs to be handled
  return 0;
}

void  compute_offset_size(char *query, hsize_t *offset, hsize_t *size, hsize_t  *dims_out, int rank){
  int i, j, d;
  char   *token, *temp;
  hsize_t  offset_t, size_t;


  token = strtok(query, ",");
  i = 0;
  while(token != NULL){
    d=strlen(token);
    temp  = malloc(d + 1);
    strcpy(temp, token);
    //printf("%s \n", temp);
    if(getvalue(temp, &offset_t, &size_t) != -1){
      offset[i] = offset_t;
      size[i]   = size_t;
    }else{
      offset[i] = 0;
      size[i]   = dims_out[i];
    }
    free(temp);
    i++;
    token = strtok(NULL, ",");
  }
}



void  print_help(){
   char *msg="Usage: %s [OPTION] \n \
      	  -h help (--help)\n \
	  -p path to the file  \n \
          -f name of the file (only HDF5 file in current version) \n\
          -g group path within HDF5 file to data set \n \
          -d name of the dataset to be reorganized \n \
          -q query on the 3D dataset (format is x1:x2,y1:y2,z1:z2, where x1~z2 can be empty to indicate first or last value) \n \
          Example: ./read-test-internal -f /scratch3/scratchdirs/dbin/testf.h5   -d /testg/testd -q :,:,1:10 \n ";
   fprintf(stdout, msg, "read");
}


int main(int argc, char **argv){
  char            path[255]={'\0'}, filename[255]={'\0'}, group[255]={'\0'}, dataset[255]={'\0'}, query[255]={'\0'};
  hid_t           file_id,  dataset_id, dataspace_id,  memspace_id, vol_id, under_dapl, dacc_tpl;
  unsigned short *data;
  int             rank, c, i;
  hsize_t         dims_out[3], offset[3], size[3];
  static time_t t1, t2;


  opterr = 0;
  while ((c = getopt (argc, argv, "p:f:g:d:q:h")) != -1)
    switch (c)
    {
      case 'p':
	strncpy(path,     optarg, NAME_MAX);
	break;
      case 'f':
	strncpy(filename, optarg, NAME_MAX);
	break;
      case 'g':
	strncpy(group,    optarg, NAME_MAX);
	break;
      case 'd':
	strncpy(dataset,  optarg, NAME_MAX);
	break;
      case 'q':
	strncpy(query,    optarg, NAME_MAX);
	break;
      case 'h':
        print_help();
	return 1;
      default:
	printf("Error option [%s]\n", optarg);
	exit(-1);
    }

  clock_t start, finish;
  double  duration;

  start = clock();

  // Open file and Dataset  
  file_id          = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
  dataset_id       = H5Dopen(file_id,  dataset,        H5P_DEFAULT);
  

  /* Get dataspace for each dataset */
  dataspace_id        = H5Dget_space(dataset_id);
  rank                = H5Sget_simple_extent_ndims(dataspace_id);
  H5Sget_simple_extent_dims(dataspace_id, dims_out, NULL);

  compute_offset_size(query, offset, size, dims_out, rank);
  //printf("dims: %d(%d, %d),   %d(%d, %d),  %d(%d, %d) \n", dims_out[0], offset[0], size[0], dims_out[1],offset[1], size[1], dims_out[2], offset[2], size[2]);

  H5Sselect_hyperslab(dataspace_id, H5S_SELECT_SET, offset, NULL, size, NULL);
  data    = (unsigned short *)malloc(sizeof(unsigned short) * size[0] * size[1] * size[2]);
  memspace_id =  H5Screate_simple (rank, size, NULL);   
  
  //Read data
  H5Dread(dataset_id, H5T_NATIVE_USHORT, memspace_id, dataspace_id, H5P_DEFAULT, data);
  finish = clock();
  duration = (double)(finish-start); // CLOCKS_PER_SEC;
  //printf("%f seconds\n",duration);
 
  printf("Reading data takes [%f]s \n", duration/CLOCKS_PER_SEC);
  
  /* int data_size = size[0] * size[1] * size[2]; */
  /* for(i = 0 ; i < 10; i++ ){ */
  /*   printf("%hu, ", data[i]); */
  /* } */
  /* printf("\n"); */
  free(data);
  H5Sclose(memspace_id);  
  H5Sclose(dataspace_id);  
  H5Dclose(dataset_id);
  H5Fclose(file_id);
}
