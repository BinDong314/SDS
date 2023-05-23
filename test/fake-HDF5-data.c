#include "stdlib.h"
#include "hdf5.h"
#include "getopt.h"
#include <string.h>


#define NAME_MAX 255

void get_dimension(int dims, char *dims_str,  hsize_t *dims_size){
  int   i;
  char *pch;
  char  temp[255];
  
  if (dims == 1){
    dims_size[0] = atoi(dims_str);
  }else{
    strcpy(temp, dims_str);
    pch = strtok(temp, ",");
    
    i = 0;
    while(pch != NULL)
    {
      dims_size[i] = atoi(pch);
      pch=strtok(NULL, ",");
      i++;
    }
  }
  return;
}


void figure_out_o_c(int dims, hsize_t *dims_size, int mpi_rank, int mpi_size, hsize_t *offset, hsize_t *count){
  hsize_t  data_size;
  int i;
  //Partition along last dimension
  data_size =  dims_size[dims-1];
  if(mpi_rank != (mpi_size - 1)){
    count[dims-1]  = data_size / mpi_size;
  }else{
    count[dims-1]  = data_size / mpi_size + data_size % mpi_size;
  }
  offset[dims-1] = (data_size/mpi_size) * mpi_rank;

  for(i = 0; i < (dims-1); i++){
    count[i]  = dims_size[i];
    offset[i] = 0;
  }
}

void print_help(){
   char *msg="Usage: %s [OPTION] \n\
      	  -h help (--help)\n\
          -f name of the file (only HDF5 file in current version) \n\
          -g group path within HDF5 file to data set \n\
          -d name of the dataset \n\
          -n dimension of the dataset \n\
          -s size of each dimension (format 1D: n 2D: x,y 3D x,y,z, etc)  \n\
          -t type of the element(0: unsigned int, 1 : float (by default)), no other type supported \n\
          -c number of cores. \n\
          -m mode(1: array, 2: rational), no suport right now \n\
          Example:  mpirun (or no mpirun) -n 1 ./fake-hdf5 -f testf.h5p -g /testg -d /testg/testd -n 2 -s 100,100 -t 0 \n";

   fprintf(stdout, msg, "fake-hdf5");
}

//#define SDS_MPI

int main(int argc, char **argv){
  int      mpi_size, mpi_rank, dims, cores, multi_dataset, type = 0, c, i;
  hid_t    file_id, dset_id, group_id, plist_id, dataspace_id, write_space, memspace_id;
  hsize_t *offset, *count, *dims_size;
  char     dims_str[NAME_MAX], filename[NAME_MAX], groupname[NAME_MAX], dsetname[NAME_MAX];
  
#ifdef SDS_CLIENT_MPI
  MPI_Comm comm = MPI_COMM_WORLD;
  MPI_Info info = MPI_INFO_NULL;
#endif
  
  
     
  while ((c = getopt (argc, argv, "f:g:d:n:c:s:ht:")) != -1)
    switch (c)
    {
      case 'f':
        strncpy(filename, optarg, NAME_MAX);
        break;
      case 'g':
        strncpy(groupname, optarg, NAME_MAX);
        break;
      case 'd':
        strncpy(dsetname, optarg, NAME_MAX);
        break;
      case 'n':
        dims     = atoi(optarg);
        break;
      case 'c':
        cores    = atoi(optarg);
        break;
      case 's':
        //string formart is like X,Y,Z, which is seperated by commas  
        strcpy(dims_str, optarg);
        break;
      case 'm':
        multi_dataset = atoi(optarg);
        break;
      case 't':
        type=atoi(optarg);
        break;
      case 'h':
        print_help();
        return 1;
        break;
      default:
        break;
    }
  
#ifdef SDS_CLIENT_MPI
  MPI_Init(&argc, &argv);
  MPI_Comm_size(comm, &mpi_size);
  MPI_Comm_rank(comm, &mpi_rank);
  printf("Use MPI \n");
#else
  printf("NO MPI \n");
  mpi_size = 1;
  mpi_rank = 0;
#endif

  
  if(mpi_rank == 0)
    printf("Using %d cores to generate dataset [%s] of dimensions [%d: %s ] within file [%s]. \n", mpi_size, dsetname, dims, dims_str, filename);
  
  dims_size = malloc(dims * sizeof(hsize_t));
  get_dimension(dims, dims_str, dims_size);
  
  if(mpi_rank == 0){
    printf("Size : ");
    for(i = 0; i < dims; i++){
      printf( " %lld ", dims_size[i]);
    }
    printf(" \n");
  }

  //Create new file and write result to this file
#ifdef SDS_CLIENT_MPI
  plist_id = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fapl_mpio(plist_id, comm, info);
#else
  plist_id = H5P_DEFAULT;
#endif
    
  file_id = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, plist_id);
  H5Pclose(plist_id);

  group_id  = H5Gcreate(file_id, groupname, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  dataspace_id = H5Screate_simple(dims, dims_size, NULL);
  if(type == 0){ // type = 0 , data is int
    dset_id    = H5Dcreate(group_id, dsetname, H5T_STD_I32LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  }else{        //By default, data is float
    dset_id    = H5Dcreate(group_id, dsetname, H5T_IEEE_F32LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  }   //Other type to be added

  H5Sclose(dataspace_id);
 
  offset = malloc(dims * sizeof(hsize_t));
  count   = malloc(dims * sizeof(hsize_t));

  //printf("mpirank %d : offset ");
  figure_out_o_c(dims, dims_size, mpi_rank, mpi_size, offset, count);

  if(mpi_rank == 0){
    printf("Size : ");
    for(i = 0; i < dims; i++){
      printf( "%lld,  ", dims_size[i]);
    }
    printf(" \n");
  }

  write_space = H5Dget_space(dset_id);
  H5Sselect_hyperslab(write_space, H5S_SELECT_SET, offset, NULL, count, NULL);

  memspace_id = H5Screate_simple(dims, count, NULL);
  
  hsize_t data_size = 1;
  for(i=0; i < dims; i++){
    //data_size = data_size * dims_size[0];
    data_size = data_size * count[i];
  }
  
 
  float size_in_gb =  data_size/1024.0/1024.0/1024.0;
  
  int            *pi;
  float          *pf;
  if(type == 0){
    //pus = (int *)buf;
    pi = malloc(data_size * sizeof(int));
    if(pi == NULL){
      printf("Memory allocation fails (size=%f) \n", size_in_gb);
      exit(1);
    }

  }else{
    //pf  = (float *)buf; 
    pf  = malloc(data_size * sizeof(float));
    if(pf == NULL){
      printf("Memory allocation fails (size=%f) \n", size_in_gb);
      exit(1);
    }
  }

  for(i = 0; i < data_size; i++){
    if(type == 0){
      pi[i] = i;
    }else{
      pf[i]  = (float) i / (float) data_size + (float)i;
    } 
  }
 
  if(mpi_rank == 0){
    printf("Write data ... \n");
  }

  if(type == 0){
    H5Dwrite(dset_id, H5T_NATIVE_INT, memspace_id, write_space, H5P_DEFAULT, pi);
    free(pi);
  }else{
    H5Dwrite(dset_id, H5T_NATIVE_FLOAT,  memspace_id, write_space, H5P_DEFAULT, pf);
    free(pf);
  }

  H5Sclose(write_space);
  H5Sclose(memspace_id);
  H5Dclose(dset_id);
  H5Gclose(group_id);
  H5Fclose(file_id);
  
#ifdef SDS_CLIENT_MPI
  MPI_Finalize();
#endif
}
