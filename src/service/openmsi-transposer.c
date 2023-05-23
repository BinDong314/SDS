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
#include "stdlib.h"
#include "hdf5.h"
#include <unistd.h>
#include <string.h>

#ifdef SDS_HDF5_VOL
#include <sds-vol-external-native.h>
#endif

#define NAME_MAX 255

enum output_3D_type{
  XYZ=0,
  XZY=1,
  YXZ=2,
  YZX=3,
  ZXY=4,
  ZYX=5
};


hid_t create_group(hid_t file_id, char *group){
  char   *token;
  hid_t   group_id;
  
  token = strtok(group, "/");
  group_id = file_id;
  
  //Group can be multiple levels, return the id for last level 
  while (token != NULL)
  {
    group_id = H5Gcreate(group_id, token, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    token    = strtok(NULL, "/");
  }
  
  return group_id;
}

void exchange_data(int mpi_size, int mpi_rank, hsize_t dims_x, hsize_t dims_y, hsize_t dims_z, hsize_t my_x_size, hsize_t rest_x_size, int *x_data, int *final_buf){
  MPI_Status      Stat;
  int ii, jj, kk,ll, dest, source, t;
  hsize_t    my_z_size, temp_x_size,temp_z_size, z_start, z_end, sr_buff_size;
  int *send_buf, *recv_buf;
  hsize_t source_dims_y, source_dims_x, source_dims_z, final_x_start, final_z_start, dest_dims_x, dest_dims_y, dest_dims_z;
  
  if(mpi_rank != (mpi_size - 1)){
    my_z_size   = dims_z/mpi_size;
  }else{
    my_z_size   = dims_z/mpi_size + dims_z%mpi_size;
  }
  
  //Make sure it have enough buff for largest cube
  temp_z_size   = dims_z/mpi_size + dims_z%mpi_size;
  temp_x_size   = dims_x/mpi_size + dims_x%mpi_size;
  sr_buff_size  = temp_x_size * dims_y * temp_z_size;

  send_buf      = malloc(sr_buff_size*sizeof(int));
  recv_buf      = malloc(sr_buff_size*sizeof(int));

  //Source and dest used to control information exchange
  source = mpi_rank;
  dest = mpi_rank;
  for (ll = 0; ll < mpi_size; ll++){
    //We dvivide each data based on Z axis again
    z_start = dest * (dims_z / mpi_size);

    dest_dims_y = dims_y;
    if(dest != (mpi_size -1)){
      dest_dims_z = dims_z/mpi_size;
    }else{
      dest_dims_z = dims_z/mpi_size + dims_z%mpi_size;
    }
    
    if(mpi_rank != (mpi_size-1)){
      dest_dims_x = dims_x/mpi_size;
    }else{
      dest_dims_x = dims_x/mpi_size + dims_x%mpi_size;
    }
    

    for (ii = 0; ii < dest_dims_x; ii++){
      for(jj = 0; jj < dest_dims_y; jj++){
	for(kk =0; kk < dest_dims_z; kk++){
	  send_buf[kk+jj*dest_dims_z + ii*dest_dims_z*dest_dims_y] = x_data[z_start + kk + jj*dims_z + ii*dims_z*dims_y];
	}
      }
    }
    
    //MPI_Barrier(MPI_COMM_WORLD);
    if(mpi_rank != dest){
      MPI_Sendrecv(send_buf, sr_buff_size, MPI_INT, dest, 3, recv_buf, sr_buff_size, MPI_INT, source,3,MPI_COMM_WORLD,&Stat);
    }else{
      for (t=0; t<sr_buff_size; t++) {
	recv_buf[t] =send_buf[t];
      }
    }
    
    source_dims_y = dims_y;
    if(source != (mpi_size -1)){
      source_dims_x = dims_x/mpi_size;
    }else{
      source_dims_x = dims_x/mpi_size + dims_x%mpi_size;
    }
    if(mpi_rank != (mpi_size -1)){
      source_dims_z = dims_z/mpi_size;
    }else{
      source_dims_z = dims_z/mpi_size + dims_z%mpi_size;
    }

    final_x_start = source*(dims_x /mpi_size);
    for(ii=0; ii < source_dims_x; ii++){
      for(jj=0; jj < source_dims_y; jj++){
	for(kk=0; kk < source_dims_z;kk++){
	  final_buf[kk+jj*source_dims_z+(final_x_start+ii)*source_dims_z*source_dims_y] = recv_buf[kk+jj*source_dims_z+ii*source_dims_z*source_dims_y];
	}
      }
    }
    
    dest = (dest + 1)%mpi_size;
    if(source != 0){
      source =  source - 1;
    }else{
      source =  mpi_size -1;
    }
  }

  free(send_buf);
  free(recv_buf);
}

void  print_help(){
  char *msg="Usage: %s [OPTION] \n\
      	  -h help (--help)\n\
          -f name of the file (only HDF5 file in current version) \n\
          -g group path within HDF5 file to data set \n\
          -d name of the dataset to be reorganized \n  \
          -o output file name (Have same group and dataset as original one !)\n \
          -t type of reorganized file(0: XYZ,1: XZY, 2: YXZ,3: YZX, 4: ZXY, 5: ZYX) \n ";
   fprintf(stdout, msg, "reorganize");
}

int main(int argc, char **argv){
  char            filename[NAME_MAX], group[NAME_MAX], dataset[NAME_MAX], output_filename[NAME_MAX];
  int             oupt_3D_type = 5;
  hid_t           file_id, group_id, dset_id, dataspace_id, plist2_id, memspace_id;
  int             mpi_size, mpi_rank, rank;
  hsize_t         dims_out[3], dims_x, dims_y, dims_z; 
  MPI_Comm        comm = MPI_COMM_WORLD;
  MPI_Info        info = MPI_INFO_NULL;
  herr_t          status;
  int             *data;
  int             i, j, k, c;
  hsize_t         my_x_size, x_rest_size, my_hyperslab_offset[3], my_hyperslab_count[3],memspace_size[3], my_z_size;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(comm, &mpi_size);
  MPI_Comm_rank(comm, &mpi_rank);

  opterr = 0;
  while ((c = getopt (argc, argv, "f:g:d:o:t:h")) != -1)
    switch (c)
      {
      case 'f':
	strncpy(filename, optarg, NAME_MAX);
	break;
      case 'g':  
	strncpy(group, optarg, NAME_MAX);
	break;
      case 'd':  
	strncpy(dataset, optarg, NAME_MAX);
	break;
      case 'o':
	strncpy(output_filename, optarg, NAME_MAX);
	break;
      case 't':
        oupt_3D_type = atoi(optarg);
        break;
      case 'h':
        print_help();
	return 1;
      default:
	printf("Error option [%s]\n", optarg);
	exit(-1);
      }

  if(oupt_3D_type != 5){
    printf("Unsupported organization type of transpose ! \n ");
    exit(-1);
  }
  
 
  hid_t acc_tpl;
  acc_tpl = H5Pcreate(H5P_FILE_ACCESS);
#ifdef SDS_HDF5_VOL
  //Register SDS external native plugin
  hid_t under_dapl;
  hid_t vol_id, vol_id2;
  under_dapl = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fapl_mpio(under_dapl, comm, info);
  vol_id  = H5VLregister(&H5VL_sds_external_native_g);
  external_native_plugin_id = H5VLget_plugin_id("native");
  assert(external_native_plugin_id > 0);
  acc_tpl = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_vol(acc_tpl, vol_id, &under_dapl);
  //End of Register SDS external native plugin
#else
  if(mpi_rank == 0)
 	 printf("NO VOL !\n");
  H5Pset_fapl_mpio(acc_tpl, comm, info);
#endif

  //Open Original XYZ file
  file_id  = H5Fopen(filename, H5F_ACC_RDONLY, acc_tpl);
  group_id = H5Gopen(file_id,  group,  H5P_DEFAULT);
  dset_id  = H5Dopen(group_id,  dataset, H5P_DEFAULT);

  dataspace_id = H5Dget_space(dset_id);
  rank         = H5Sget_simple_extent_ndims(dataspace_id);
  status       = H5Sget_simple_extent_dims(dataspace_id, dims_out, NULL);

  //Only supported output file layout is ZYX now; Todo: more flexiable 
  dims_x  = dims_out[0];
  dims_y  = dims_out[1]; 
  dims_z  = dims_out[2];

  //Split the cuebe based on Z to perform (X, Y, Z) -> (Z, Y, X)
  //and let each process read its corresponding data
  x_rest_size = dims_x % mpi_size;
  if (mpi_rank != (mpi_size -1)){
    my_x_size  = dims_x / mpi_size;
    my_z_size  = dims_z / mpi_size;
  }else{
    my_x_size  = dims_x / mpi_size + x_rest_size;
    my_z_size  = dims_z / mpi_size + dims_z % mpi_size;
  }


  my_hyperslab_offset[0] = (dims_x/mpi_size)*mpi_rank;
  my_hyperslab_offset[1] = 0;
  my_hyperslab_offset[2] = 0;

  my_hyperslab_count[0]  = my_x_size;
  my_hyperslab_count[1]  = dims_y;
  my_hyperslab_count[2]  = dims_z;

  if(!mpi_rank){
	printf(" mpi_sze = %d \n ", mpi_size);
  }

  H5Sselect_hyperslab(dataspace_id, H5S_SELECT_SET, my_hyperslab_offset, NULL, my_hyperslab_count, NULL);

  plist2_id = H5Pcreate(H5P_DATASET_XFER);
  H5Pset_dxpl_mpio(plist2_id, H5FD_MPIO_COLLECTIVE);

  memspace_size[0] = my_x_size;
  memspace_size[1] = dims_y;
  memspace_size[2] = dims_z;

  data = malloc(my_x_size * dims_y * dims_z * sizeof(int)); 
  if(data == NULL){
    printf("Memory allocation fails for data (before read) at rank %d \n", mpi_rank);
    exit(-1);
  }
  memspace_id =  H5Screate_simple(rank, memspace_size, NULL);
  //Read data
  H5Dread(dset_id, H5T_NATIVE_INT, memspace_id, dataspace_id, plist2_id, data); 
  
  H5Sclose(dataspace_id);
  H5Sclose(memspace_id);
  H5Dclose(dset_id);
  H5Gclose(group_id);
  H5Fclose(file_id);

  //Exchange data among processes
  int *z_align_data;
  z_align_data    = malloc(dims_x * dims_y * my_z_size * sizeof(int));
  if(z_align_data == NULL){
    printf("Memory allocation fails for z_align_data (for exchange) at rank %d \n", mpi_rank);
    exit(-1);
  }
   if(!mpi_rank) printf("Exchange data ...");


  exchange_data(mpi_size, mpi_rank, dims_x, dims_y, dims_z, my_x_size,x_rest_size, data, z_align_data); 
  free(data);

  //Perform XyZ to ZyX transform
  int            *data_t;
  hsize_t         max_size;
  max_size = dims_x * dims_y * my_z_size;
  data_t   = malloc(max_size * sizeof(int)); 
  if(data_t == NULL){
    printf("Memory allocation fails for data_t (before transpose) at rank %d \n", mpi_rank);
    exit(-1);
  }
  if(!mpi_rank) printf("Transpose  data ... \n");
  for(i=0; i<dims_x; i++){
    for (j=0; j<dims_y; j++){
      for (k=0;k<my_z_size;k++){
	if( (i +  j*dims_x + k*dims_y*dims_x < max_size) && (k +  j*my_z_size + i*dims_y*my_z_size < max_size))
	  data_t[i +  j*dims_x + k*dims_y*dims_x] = z_align_data[k +  j*my_z_size + i*dims_y*my_z_size];
      }
    }
  }
  free(z_align_data);

  //Create new file and write result to this file
  hid_t   file_id2, group_id2, dset_id2, dataspace_id2,result_space, result_memspace_id;
  hsize_t dims2[3], result_offset[3], result_count[3], result_memspace_size[3];

    
  //file_id2 = H5Fcreate(output_filename,H5F_ACC_TRUNC, H5P_DEFAULT, plist_id3);
  file_id2 = H5Fcreate(output_filename,H5F_ACC_TRUNC, H5P_DEFAULT, acc_tpl);

  //group_id2_temp = H5Gcreate(file_id2, group, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  //group_id2 = H5Gcreate(group_id2_temp, "/entry_0/data_0", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  
  //Group can be multiple levels, return the id for last level 
  group_id2 = create_group(file_id2, group);
  
  dims2[0] = dims_z;
  dims2[1] = dims_y;
  dims2[2] = dims_x;
  dataspace_id2 = H5Screate_simple(3, dims2, NULL);
  dset_id2 = H5Dcreate(group_id2, dataset, H5T_STD_I32LE, dataspace_id2, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  result_offset[0] = (dims_z/mpi_size)*mpi_rank;
  result_offset[1] = 0;
  result_offset[2] = 0;

  result_count[0] = my_z_size;
  result_count[1] = dims_y;
  result_count[2] = dims_x;

  result_space = H5Dget_space(dset_id2);
  H5Sselect_hyperslab(result_space, H5S_SELECT_SET, result_offset, NULL, result_count, NULL);

  result_memspace_size[0] = my_z_size;
  result_memspace_size[1] = dims_y;
  result_memspace_size[2] = dims_x;
  result_memspace_id = H5Screate_simple(3, result_memspace_size, NULL);
  
  if(!mpi_rank) printf("Writing data ...");

  H5Dwrite(dset_id2, H5T_NATIVE_INT, result_memspace_id, result_space, plist2_id, data_t);
  free(data_t);

  H5Sclose(result_memspace_id);
  H5Sclose(dataspace_id2);
  H5Sclose(result_space);
  H5Dclose(dset_id2);
  H5Gclose(group_id2);
  H5Fclose(file_id2);
  H5Pclose(acc_tpl);
  H5Pclose(plist2_id);
#ifdef SDS_HDF5_VOL
  //Unregistered the plugin-id
  H5Pclose(under_dapl);
  H5VLunregister(vol_id);
  //End of Unregistered the plugin-id
#endif

  //MPI_Barrier(MPI_COMM_WORLD);
 
  MPI_Finalize();
  return  0;
}
