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
#include <sys/types.h>
#include <sys/stat.h>


typedef struct dataset_info{
  hid_t    open_file_id;    //file id returned by open
  char     name[244];       //dataset's name, which also contains the group path
  hid_t    open_dataset_id; //dataset id returned by open
  hid_t    open_space_id;   //space id 
  hsize_t  global_size;     //size of the dataset
  hsize_t  local_size;      //local size for the dataset 
  hsize_t  local_offset;    //offset 
  int      rank;
  float   *buf;
  float    local_min;
  float    local_max;
  float    global_min;
  float    global_max;
  float    chunk_size;       //size is equal to query size
  hsize_t  chunk_count;      //(max - min) / chunk_size
  float   *chunk_boundary;   //Array to store the chunk_boundary 
  int      mpi_rank;
  int      mpi_size;
  hid_t    shared_output_fileid;
  hid_t    shared_output_dsetid;
  hid_t    mapping_fileid;
  hid_t    mapping_dsetid;
  hid_t    mapping_attributeid;
  hid_t    hist_fileid;
  hid_t    hist_dsetid;
  hid_t    hist_attributeid;
}dataset_info_t;

#define MAX_DATSET_COUNT 10

dataset_info_t  data_info_array[MAX_DATSET_COUNT];
float           query_range[MAX_DATSET_COUNT] = {0.1, 10, 10, 10, 1, 2, 2, 2, 2, 2};
dataset_info_t  extra_data_info_array[MAX_DATSET_COUNT];



void  equal_partition(dataset_info_t *data_info_array){
  hid_t   dset_id;
  hsize_t my_size, my_offset, rest_size, dims_out[1];
  int     mpi_size, mpi_rank, rank;
  hid_t  dataspace;
  mpi_size = data_info_array->mpi_size;
  mpi_rank = data_info_array->mpi_rank;
  dset_id  = H5Dopen(data_info_array->open_file_id, data_info_array->name, H5P_DEFAULT);
  dataspace = H5Dget_space(dset_id);
  rank = H5Sget_simple_extent_ndims(dataspace);
  H5Sget_simple_extent_dims(dataspace, dims_out, NULL);

  dims_out[0] = dims_out[0];
  rest_size = dims_out[0] % mpi_size;
  if (mpi_rank ==  (mpi_size - 1)){
    my_size = dims_out[0]/mpi_size + rest_size;
  }else{
    my_size = dims_out[0]/mpi_size;
  }
  my_offset = mpi_rank * (dims_out[0]/mpi_size);
  
  data_info_array->open_dataset_id = dset_id;
  data_info_array->local_offset    = my_offset;
  data_info_array->global_size     = dims_out[0]; 
  data_info_array->local_size      = my_size;
  //printf("mpi rank %d , local_size %llu \n", mpi_rank, my_size);
  data_info_array->open_space_id   = dataspace;
  data_info_array->rank            = rank;

}



void read_segement(dataset_info_t * current_file_info){
  hid_t plist2_id, memspace;
  
  memspace =  H5Screate_simple(current_file_info->rank, &(current_file_info->local_size), NULL);

  plist2_id = H5Pcreate(H5P_DATASET_XFER);
  H5Pset_dxpl_mpio(plist2_id, H5FD_MPIO_COLLECTIVE);

  hsize_t my_offset, my_size;
  my_offset = current_file_info->local_offset;
  my_size   = current_file_info->local_size;
  H5Sselect_hyperslab(current_file_info->open_space_id, H5S_SELECT_SET, &my_offset, NULL, &my_size , NULL);
  
  H5Dread(current_file_info->open_dataset_id, H5T_NATIVE_FLOAT, memspace, current_file_info->open_space_id, plist2_id, current_file_info->buf);
  
  H5Pclose(plist2_id);
}


void find_min_and_max(dataset_info_t * current_file_info){
  float *buf, min, max;
  int   i;
  buf =  current_file_info->buf;
  
  min = buf[0];
  max = buf[0];
  
  for (i = 0 ; i < current_file_info->local_size; i++){
    if(min > buf[i]){
      min = buf[i];
    }
    
    if(max < buf[i]){
      max = buf[i];
    }
  }
  
  current_file_info->local_min = min;
  current_file_info->local_max = max;

}
  

void  find_global_min_and_max(dataset_info_t * current_file_info){
  double  global_min, global_max;
  double local_min,  local_max;
  
  local_min  =  current_file_info->local_min;
  local_max  =  current_file_info->local_max;
  
  MPI_Allreduce(&local_max, &global_max, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
  MPI_Allreduce(&local_min, &global_min, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);

  current_file_info->global_min = global_min;
  current_file_info->global_max = global_max;
 
}

void compute_chunk_boundary(dataset_info_t *current_file_info, int total_chunk_count){
  int i;
  current_file_info->chunk_boundary = malloc(sizeof(float) * (total_chunk_count-1));
  
  //Set the last one to the maximum value
  for (i = 1; i < total_chunk_count; i++){
    current_file_info->chunk_boundary[i-1] = current_file_info->global_min + current_file_info->chunk_size * (float) i;
  }
}


hsize_t compute_chunk_id(float *tem_buf, int buf_size){
  hsize_t id = 0;
  hsize_t i, j, k;
  float value;
  dataset_info_t *current_file_info;
  for (i = 0;  i < buf_size; i++){
    value = tem_buf[i];
    current_file_info = &(data_info_array[i]);
    j = 0;
    while( (value > current_file_info->chunk_boundary[j] &&  j < (current_file_info->chunk_count - 1))){
      j++;
    }
    id = j + current_file_info->chunk_count * id ; // To be tested !
  }
  
  return id;
}


void   filter_local(hsize_t local_size, hsize_t *local_chunk_id, hsize_t chunk_count, hsize_t *local_chunk_hist){
  hsize_t i, j;
  for(j = 0; j < local_size; j++){
    i = local_chunk_id[j];
    local_chunk_hist[i] = local_chunk_hist[i] + 1;
  }
}

void find_local_buffer_offset(hsize_t *buffet_offset, hsize_t chunk_count, hsize_t *local_histgram){
  int i;
  hsize_t offset = 0;
  
  buffet_offset[0] = 0;
  for (i = 1; i < chunk_count; i++){
    buffet_offset[i] = buffet_offset[i-1] + local_histgram[i-1];
  }
}


void create_output_file(char *outputdir, char *groupname, int file_count, int extra_file_count, int mpi_rank, int mpi_size, hsize_t  total_chunk_count, hsize_t *local_histgram){
  hid_t file_id, group_id, plist_id, *dset_id, dataspace_id, mapping_fileid, mapping_dsetid;
  dataset_info_t *current_file_info;
  hsize_t dset_size;
  char    temp_dir[255], file_name_with_rank[255], mapping_file_name_with_rank[255],  hist_file_name_with_rank[255];
  float   t1, t2;

  //Create sorted file and dataset
  sprintf(temp_dir, "%s-sorted-dir", outputdir);
  /* if(stat(temp_dir, &st) != 0){ */
  /*   //mkdir(fnm.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); */
  /*   mkdir(temp_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); */
  /* } */
  //mkpath(temp_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

  MPI_Barrier(MPI_COMM_WORLD);
  t1 =  MPI_Wtime();
  struct stat st;
  if(mpi_rank == 0){
    if (stat(temp_dir, &st) != 0) {
      printf("Create temp_dir %s ! \n ", temp_dir);
      if(mkdir(outputdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0){
        printf("Cann't create dir %s \n ", temp_dir);
        exit(-1);
      }
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);

  //sprintf(file_name_with_rank, "%s/sd_%d.h5p", temp_dir, mpi_rank);
  
  sprintf(file_name_with_rank, "%s/all2.h5p", temp_dir);
  
  
  plist_id = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fapl_mpio(plist_id, MPI_COMM_WORLD, MPI_INFO_NULL);

  file_id  = H5Fcreate(file_name_with_rank, H5F_ACC_TRUNC, H5P_DEFAULT, plist_id);
  group_id = H5Gcreate(file_id,  groupname, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  int i;
  current_file_info = &(data_info_array[0]);
  dset_size         = current_file_info->local_size;
  dataspace_id      = H5Screate_simple(1, &dset_size, NULL);

  //Create mapping file and dataset
  sprintf(mapping_file_name_with_rank, "%s/m_%d.h5p", temp_dir, mpi_rank);
  //mapping_fileid   = H5Fcreate(mapping_file_name_with_rank, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  //mapping_dsetid   = H5Dcreate(mapping_fileid, "/md", H5T_NATIVE_ULLONG, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    
  hid_t hist_dataspace_id, hist_fileid, hist_dsetid, size_dsetid;
  hist_dataspace_id  = H5Screate_simple(1, &total_chunk_count, NULL);
  sprintf(hist_file_name_with_rank, "%s/hist_%d.h5p", temp_dir, mpi_rank);
  hist_fileid   = H5Fcreate(hist_file_name_with_rank, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  hist_dsetid   = H5Dcreate(hist_fileid, "/offset",     H5T_NATIVE_ULLONG, hist_dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  size_dsetid   = H5Dcreate(hist_fileid, "/size",     H5T_NATIVE_ULLONG, hist_dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
 
  current_file_info = &(data_info_array[0]); 
  current_file_info = &(data_info_array[0]);

  hsize_t *local_offset_table, *local_size_table;
  local_offset_table = malloc(sizeof(hsize_t)*total_chunk_count);
  local_size_table   = malloc(sizeof(hsize_t)*total_chunk_count);
  local_offset_table[0] = current_file_info->local_offset;
  local_size_table[0]   = local_histgram[0];

  //printf("rank %d , offset %llu, size %llu, local_hist %llu \n ", mpi_rank, local_offset_table[0], local_size_table[0], local_histgram[0]);
  
  for(i = 1 ; i < total_chunk_count; i++){
    local_offset_table[i] = local_histgram[i-1] + local_offset_table[i-1];
    local_size_table[i]   = local_histgram[i]; 
    if(mpi_rank == 0 && i < 20){
      printf("%d , offset %llu, size %llu, local_hist %llu \n ", i, local_offset_table[i], local_size_table[i], local_histgram[i]);
    }
  }
  H5Dwrite(hist_dsetid, H5T_NATIVE_ULLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, local_offset_table);
  H5Dwrite(size_dsetid, H5T_NATIVE_ULLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, local_size_table);
 
  hsize_t dims[2];
  hid_t   asid, minmaxtable_attributeid;
  float   min_max_table[4][3];
  dims[0] = 4;
  dims[1] = 3;
  asid = H5Screate_simple(2, dims, NULL);
  minmaxtable_attributeid = H5Acreate(hist_dsetid, "minmaxtable", H5T_NATIVE_FLOAT, asid, H5P_DEFAULT, H5P_DEFAULT);
  for(i = 0 ; i < 4; i++){
    current_file_info = &(data_info_array[i]);
    min_max_table[i][0] = current_file_info->global_min;
    min_max_table[i][1] = current_file_info->global_max;
    min_max_table[i][2] = current_file_info->chunk_size;
  }
  
  H5Awrite(minmaxtable_attributeid, H5T_NATIVE_FLOAT, min_max_table);
  H5Aclose(minmaxtable_attributeid);
  H5Sclose(asid);

  H5Fclose(hist_fileid);
  H5Dclose(size_dsetid);
  H5Dclose(hist_dsetid);
  H5Sclose(hist_dataspace_id);

  hsize_t global_dset_size         = current_file_info->global_size;
  hid_t   global_dataspace_id      = H5Screate_simple(1, &global_dset_size, NULL);


  //hsize_t dims, local_offset;
  //hid_t  mapping_attributeid, asid;
  //dims = 1;
  //asid = H5Screate_simple(1, &dims, NULL);
  //mapping_attributeid = H5Acreate(mapping_dsetid, "offset", H5T_NATIVE_ULLONG, asid, H5P_DEFAULT, H5P_DEFAULT);
  //local_offset = current_file_info->local_offset;
  //H5Awrite(mapping_attributeid, H5T_NATIVE_ULLONG, &local_offset);
  //H5Aclose(mapping_attributeid);
  //H5Sclose(asid);
      
  for (i = 0; i < file_count; i++){
    current_file_info = &(data_info_array[i]);
    //printf("%s \n", current_file_info->name);
    //MPI_Barrier(MPI_COMM_WORLD);
    current_file_info->shared_output_fileid = file_id;
    current_file_info->shared_output_dsetid = H5Dcreate(file_id, current_file_info->name, H5T_IEEE_F32LE, global_dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    //if(current_file_info->shared_output_dsetid <  0){
    //  printf("Create output dataset failed ! \n");
    //  exit(-1);
    //}
    //current_file_info->mapping_fileid = mapping_fileid;
    //current_file_info->mapping_dsetid = mapping_dsetid;
  }

  for (i = 0; i < extra_file_count; i++){
    current_file_info = &(extra_data_info_array[i]);
    if(mpi_rank == 0)
      printf("i %d : date %s \n", i, current_file_info->name);
    current_file_info->shared_output_fileid = file_id;
    current_file_info->shared_output_dsetid = H5Dcreate(file_id, current_file_info->name, H5T_IEEE_F32LE, global_dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    //if(current_file_info->shared_output_dsetid <  0){
    //  printf("Create output dataset failed ! \n");
    //  exit(-1);
    // }
    current_file_info->mapping_fileid = mapping_fileid;
    current_file_info->mapping_dsetid = mapping_dsetid;
  }

  H5Sclose(global_dataspace_id);
  H5Sclose(dataspace_id);
  H5Pclose(plist_id);
  H5Gclose(group_id);

  MPI_Barrier(MPI_COMM_WORLD);
  t2 =  MPI_Wtime();
  
  if(mpi_rank == 0){
    printf("Time to create directory and (%d) files is %f \n ", mpi_size, t2 - t1);
  }
  //H5Fclose(file_id);
  return;
}


void dump_to_file_multiple(dataset_info_t *current_file_info, float *temp_data, int mpi_rank){
  hid_t plist_id, file_id, group_id, dset_id, plist_id2, dataspace_id, filespace, memspace;
  hsize_t  dset_size, offset_size, dump_size;
  //current_file_info = &(data_info_array[j]);
    
  dset_id     = current_file_info->shared_output_dsetid;
  dump_size   = current_file_info->local_size;
  offset_size = current_file_info->local_offset;
  filespace = H5Dget_space(dset_id);
  H5Sselect_hyperslab(filespace, H5S_SELECT_SET, &offset_size, NULL, &dump_size, NULL);
  
  //Write data
  memspace  = H5Screate_simple(1, &dump_size, NULL);
  plist_id2 = H5Pcreate(H5P_DATASET_XFER);
  H5Pset_dxpl_mpio(plist_id2, H5FD_MPIO_COLLECTIVE);
  H5Dwrite(dset_id, H5T_NATIVE_FLOAT, memspace, filespace, plist_id2, temp_data);
  //H5Dwrite(dset_id, H5T_NATIVE_FLOAT, memspace, filespace, H5P_DEFAULT, temp_data);

  //H5Dwrite(dset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, temp_data);

  H5Pclose(plist_id2);
  H5Sclose(filespace);
  H5Sclose(memspace);
  
  return;
}


void  dump_local_mapping(hsize_t *local_mapping){
  dataset_info_t *current_file_info;
  hid_t     dset_id, filespace, memspace;
  hsize_t   offset_size, dump_size;
  
  current_file_info = &(data_info_array[0]);
    
  dset_id     = current_file_info->mapping_dsetid;
  dump_size   = current_file_info->local_size;
  offset_size = 0;
  filespace = H5Dget_space(dset_id);
  H5Sselect_hyperslab(filespace, H5S_SELECT_SET, &offset_size, NULL, &dump_size, NULL);

  //Write data
  memspace  = H5Screate_simple(1, &dump_size, NULL);
  H5Dwrite(dset_id, H5T_NATIVE_ULLONG, memspace, filespace, H5P_DEFAULT, local_mapping);

  H5Sclose(filespace);
  H5Sclose(memspace);
  
  return;
}



void  write_result(int mpi_size, int mpi_rank, hsize_t local_size, int file_count, int extra_file_count, hsize_t *local_chunk_id, hsize_t chunk_count, hsize_t *global_histgram, hsize_t *local_histgram, hsize_t *local_mapping){
  hsize_t  i, j, *point_array, buffer_offset, chunk_id, *chunk_offset_of_sortedfile, *sorted_buffer_offset;
  float   **temp_data, **extra_temp_data;
  dataset_info_t *current_file_info, *extra_current_file_info;
  float t2, t1, t3;

  MPI_Barrier(MPI_COMM_WORLD);
  t1 =  MPI_Wtime();

  if(mpi_rank == 0)
    printf("[Rank %d] write sorted results ... ", mpi_rank);

  //chunk_offset_of_sortedfile   = malloc(sizeof(hsize_t) * chunk_count);
  //if(chunk_offset_of_sortedfile ==  NULL){
  //  printf("Allocate memory fails in write_result !\n ");
  //  exit(-1);
  //}
  sorted_buffer_offset = malloc(sizeof(hsize_t) * chunk_count);
  if(sorted_buffer_offset ==  NULL){
    printf("Allocate memory fails in write_result !\n ");
    exit(-1);
  }

  //Get the offset of each chunk based mpi_rank
  //obtain_chunk_offset_in_file(chunk_offset_of_sortedfile, chunk_count, mpi_size, mpi_rank, local_histgram, global_histgram);

  //Figure out the offset for each chunk in sorted local buffer
  find_local_buffer_offset(sorted_buffer_offset, chunk_count, local_histgram);

  //Buffer for sorted data
  temp_data = (float **)malloc(sizeof(float *)*file_count);
  for(i = 0; i < file_count; i++){
    temp_data[i]  = malloc(sizeof(float)*local_size);
    if(temp_data[i] == NULL){
      printf("Allocate memory fails in write_result !\n ");
      exit(-1);
    }
  }

 //Buffer for sorted data
  extra_temp_data = (float **)malloc(sizeof(float *)*extra_file_count);
  for(i = 0; i < extra_file_count; i++){
    extra_temp_data[i]  = malloc(sizeof(float)*local_size);
    if(extra_temp_data[i] == NULL){
      printf("Allocate memory fails in write_result !\n ");
      exit(-1);
    }
  }

  //point_array maitains current postition to store the sorted data in different chunk
  point_array = malloc(sizeof(hsize_t) * chunk_count);
  if(point_array == NULL){
    printf("Allocate memory fails in write_result !\n ");
    exit(-1);
  }
  memset(point_array, 0, sizeof(hsize_t) * chunk_count);

  //Iterate through all data
  for (i = 0; i < local_size; i++){
    //Find the data within one chunk
    chunk_id = local_chunk_id[i];
    //buffer_offset = find_local_buffer_offset(chunk_id, point_array[chunk_id], local_histgram);
    buffer_offset = sorted_buffer_offset[chunk_id] + point_array[chunk_id];
    //Iterate all files
    for (j = 0; j < file_count; j++){
      current_file_info           = &(data_info_array[j]);
      temp_data[j][buffer_offset] = current_file_info->buf[i];
    }

    for (j = 0; j < extra_file_count; j++){
      extra_current_file_info           = &(extra_data_info_array[j]);
      extra_temp_data[j][buffer_offset] = extra_current_file_info->buf[i];
    }
    
    //Update pointer
    point_array[chunk_id] = point_array[chunk_id] + 1;
    //update local_mapping
    //local_mapping[i] = find_global_chunk_offset(mpi_rank, mpi_size, chunk_id, point_array[chunk_id], global_histgram, local_histgram);
    //local_mapping[i] = chunk_offset_of_sortedfile[chunk_id] + point_array[chunk_id];
    local_mapping[i] = buffer_offset;
    if(mpi_rank == 0)
      if(i % 1000000 == 0 )
        printf("i /  local_size = %llu, %llu, %f % \n", i, local_size, (float)i/(float)local_size * 100);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  t2 =  MPI_Wtime();
  if(mpi_rank == 0 ){
    printf("Gather data %f \n", t2 - t1);
  }

  for (j = 0; j < file_count; j++){
    current_file_info  = &(data_info_array[j]);
    dump_to_file_multiple(current_file_info, temp_data[j], mpi_rank);
  }

  //for (j = 0; j < extra_file_count; j++){
  //  extra_current_file_info  = &(extra_data_info_array[j]);
  //  dump_to_file_multiple(extra_current_file_info, extra_temp_data[j], mpi_rank);
  //}
  dump_local_mapping(local_mapping);  

  MPI_Barrier(MPI_COMM_WORLD);
  t3 =  MPI_Wtime();
  if(mpi_rank == 0 ){
    printf("inside Writing time %f \n", t3 - t2);
  }

  for(i = 0 ; i < file_count; i++){
    //memcpy(current_file_info->buf, temp_data[i], sizeof(float)*local_size);
    free(temp_data[i]);
  }

  for(i = 0 ; i < extra_file_count; i++){
    free(extra_temp_data[i]);
  }

  free(temp_data);
  free(extra_temp_data);

  free(point_array);
  free(sorted_buffer_offset);

  //  if(mpi_rank == 0)
  // printf(" done ! \n ");
}


void  chunk_sort(int file_count, hsize_t total_chunk_count, hsize_t *local_chunk_id, hsize_t *local_histgram){
  int i, j;
  dataset_info_t *current_file_info =  &(data_info_array[0]);
  hsize_t         local_size        = current_file_info->local_size;
  int             local_rank        = current_file_info->mpi_rank; 
  float *temp_buf;
  
  if(local_rank == 0)
    printf("[Rank %d] Chunk sort ....", local_rank);

  temp_buf = malloc(sizeof(float) * (file_count));
  if(temp_buf == NULL){
    printf("Mememory allocation fail in chunk_sort !\n");
    exit(-1);
  }
  
  for(i=0; i < local_size; i++){
    for (j  = 0; j < file_count; j++){
      current_file_info = &(data_info_array[j]);
      temp_buf[j] = current_file_info->buf[i];
    }
    local_chunk_id[i] = compute_chunk_id(temp_buf, file_count);
    
    if(local_rank == 0 && i<10 )
      printf("(e, x, y, z)(%f, %f, %f, %f) = id (%lld)\n", temp_buf[0], temp_buf[1], temp_buf[2], temp_buf[3], local_chunk_id[i]);
  }

  //Obtain the local_histgram
  filter_local(local_size, local_chunk_id, total_chunk_count, local_histgram);
  
  free(temp_buf);
  if(local_rank == 0){
    printf("done. \n");
    fflush(stdout);
  }
}

void  print_help(){
  char *msg="Usage: %s [OPTION] \n\
      	  -h help (--help)\n\
          -f name of the file (only HDF5 file in current version) \n\
          -g group path within HDF5 file to data set \n\
          -d name of the dataset to be reorganized \n  \
          -q query range \n \
          -o output dir to store sorted files and mapping files \n \
             Files are named as sd_(mpi_rank).h5, and mapping file is named as m_(mpi_rank).h5p \n \
             > sd_(mpi_rank).h5 has same group/dataset structure as original file. \n \
             > m_(mpi_rank).h5 has one-dimension dataset (named md) under root group \n ";
   fprintf(stdout, msg, "reorganize");
}

int main(int argc, char *argv[]){
  char   filename[244], outputdir[244], group[244];
  int    i, file_count = 0, extra_file_count = 0, mpi_size, mpi_rank, query_count = 0;
  hid_t  file_id, plist_id;
  MPI_Comm comm = MPI_COMM_WORLD;
  MPI_Info info = MPI_INFO_NULL;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(comm, &mpi_size);
  MPI_Comm_rank(comm, &mpi_rank);

  static const char *options="f:g:d:o:q:e:h";
  extern char *optarg;
  int c;
  while ((c = getopt(argc, argv, options)) != -1) {
    switch (c) {
      case 'f':
      case 'F': 
        strcpy(filename, optarg); 
        break;
      case 'g':
      case 'G':
        strcpy(group, optarg); 
        break;
      case 'd':
      case 'D': 
        strcpy(data_info_array[file_count].name, optarg); 
        data_info_array[file_count].mpi_rank = mpi_rank;
        data_info_array[file_count].mpi_size = mpi_size;
        file_count = file_count + 1;
        break;
      case 'q':
      case 'Q':
        query_range[query_count]=atof(optarg);
        query_count = query_count + 1;
        break;
      case 'e':
      case 'E':
        strcpy(extra_data_info_array[extra_file_count].name, optarg); 
        extra_data_info_array[extra_file_count].mpi_rank = mpi_rank;
        extra_data_info_array[extra_file_count].mpi_size = mpi_size;
        extra_file_count = extra_file_count + 1;
        break;
      case 'o':
      case 'O':
        strcpy(outputdir, optarg);
        break;
      case 'h':
      case 'H':
        print_help();
        return 1;
      default: 
        break;
    } // switch
  } // while


  double t1, t2, t3, t4, t5, t6, t7, t8;
  MPI_Barrier(MPI_COMM_WORLD);
  t1 =  MPI_Wtime();
  
  plist_id = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fapl_mpio(plist_id, comm, info);
  file_id = H5Fopen(filename, H5F_ACC_RDONLY,plist_id);

  //Open one file and dataset
  dataset_info_t *current_file_info;
  hsize_t         total_chunk_count = 1;
  hsize_t         local_size, global_size;
  for(i = 0; i < file_count; i++){
    current_file_info = &(data_info_array[i]);
    current_file_info->open_file_id = file_id;

    //Partition into segements
    equal_partition(current_file_info);
    current_file_info->buf = malloc(sizeof(float)*current_file_info->local_size);
    if(current_file_info->buf == NULL){
      printf("Memory allocation for reading data fails ! \n");
      exit(-1);
    }
    local_size  =current_file_info->local_size;
    global_size =current_file_info->global_size;

    //printf("rank %d file_count %d local_size %llu\n", mpi_rank, i, local_size);
    
    //Parallel read
    read_segement(current_file_info);
    
    //Find   local min & max values
    find_min_and_max(current_file_info);
    
    //Find  global min & max  values
    find_global_min_and_max(current_file_info);
    

    //Determine chunk size
    current_file_info->chunk_size  = query_range[i];
    current_file_info->chunk_count =(int) ((current_file_info->global_max - current_file_info->global_min)/query_range[i]) + 1;
    total_chunk_count = current_file_info->chunk_count * total_chunk_count;
    
    //Compute chunk boundary 
    compute_chunk_boundary(current_file_info, current_file_info->chunk_count);

    if (mpi_rank == 0){
      printf("%s:  max %f, min %f, chunk count %llu, local_size %llu, query size %f \n", current_file_info->name, current_file_info->global_max, current_file_info->global_min, current_file_info->chunk_count, current_file_info->local_size, current_file_info->chunk_size);
      int k;
      for(k = 0; k< (current_file_info->chunk_count -1); k++){
        printf("%f(%d), ", current_file_info->chunk_boundary[k], k);
      }
      printf("\n");
    }
  }

  //Read extra datasets, which will not used as sorting candidates
  dataset_info_t *extra_current_file_info;
  for(i = 0; i < extra_file_count; i++){
    extra_current_file_info = &(extra_data_info_array[i]);
    extra_current_file_info->open_file_id = file_id;
    
    //Partition into segements
    equal_partition(extra_current_file_info);
    extra_current_file_info->buf = malloc(sizeof(float)*extra_current_file_info->local_size);
    if(extra_current_file_info->buf == NULL){
      printf("Memory allocation for reading extra data fails ! \n");
      exit(-1);
    }
    //Parallel read
    read_segement(extra_current_file_info);
  }
  //printf("mpi rank %d (%d) , offset %llu, size %llu \n", current_file_info->rank, mpi_rank,  current_file_info->local_offset, current_file_info->local_size);

  if(mpi_rank == 0){
    printf("Total chunk count %llu \n ", total_chunk_count);
  }

  H5Pclose(plist_id);

  MPI_Barrier(MPI_COMM_WORLD);
  t2 =  MPI_Wtime();
  
  if(mpi_rank == 0 ){
    printf("Reading time %f \n", t2 - t1);
  }
  hsize_t *local_histgram, *global_histgram, *local_mapping, *local_chunk_id;
  //Local and global hist for different chunks
  local_histgram       = malloc(sizeof(hsize_t) * total_chunk_count);
  global_histgram      = malloc(sizeof(hsize_t) * total_chunk_count);
  //local chunk id record the id (0, 1, ... total_chunk_count) for each chunk
  local_chunk_id       = malloc(sizeof(hsize_t) * local_size);
  //mapping records new file offset sorted data. It is a 1D array. 
  local_mapping        = malloc(sizeof(hsize_t) * local_size);
  if(local_histgram ==  NULL || global_histgram == NULL || local_chunk_id == NULL || local_mapping == NULL ){
    printf("Allocate memory fails for local_histgram, global_histgram, etc !\n");
    exit(-1);
  }
  
  memset(local_histgram,  0, sizeof(hsize_t) * total_chunk_count);
  memset(global_histgram, 0, sizeof(hsize_t) * total_chunk_count);
  
  //Fill local chunk id and local_histgram 
  chunk_sort(file_count, total_chunk_count, local_chunk_id, local_histgram);
  
  //Gather count of particles in each chunk
  MPI_Allreduce(local_histgram, global_histgram, total_chunk_count, MPI_UNSIGNED_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);

  MPI_Barrier(MPI_COMM_WORLD);
  t3 =  MPI_Wtime();

  if(mpi_rank == -1 ){
    printf("Building histgram time %f \n", t3 - t2);
    printf("first 10 hist at rank 0:   \n");
    for(i = 0 ; i < 10 ; i++){
      printf(" , %llu ", local_histgram[i]);
    }
    printf(" \n global hist  of first 10 chunk : ");
    for(i = 0 ; i < 10 ; i++){
      printf(" , %llu ", global_histgram[i]);
    }
    printf("\n");
  }


  int      j;
  hsize_t  size_check = 0;
  for (i = 0; i < total_chunk_count; i++){
    size_check = size_check + local_histgram[i];
  }
  if(size_check != local_size){
    printf("No, histgram size (%llu) is not equal to local size (%llu)! \n", size_check, local_size);
    exit(-1);
  }
  
  //printf("rank %d, local hist size %llu, local_size %llu \n", mpi_rank, size_check, local_size);
  //MPI_Barrier(MPI_COMM_WORLD);


  size_check = 0;
  for (i = 0; i < total_chunk_count; i++){
    size_check = size_check + global_histgram[i];
  }
  if(size_check != global_size){
    printf("No, histgram size (%llu) is not equal to global size (%llu)! \n", size_check, global_size);
    exit(-1);
  }
  
  create_output_file(outputdir, group, file_count, extra_file_count,  mpi_rank, mpi_size, total_chunk_count, local_histgram);
     
  //Write result to 
  write_result(mpi_size, mpi_rank, local_size, file_count, extra_file_count, local_chunk_id, total_chunk_count, global_histgram, local_histgram, local_mapping);

  MPI_Barrier(MPI_COMM_WORLD);
  t4 =  MPI_Wtime();
  
  if(mpi_rank == 0 ){
    printf("Writing time %f \n", t4 - t3);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  t5 =  MPI_Wtime();
  
  if(mpi_rank == 0 ){
    printf("Overall time time %f \n", t5 - t1);
  }


  free(local_histgram);
  free(global_histgram);
  free(local_chunk_id);
  free(local_mapping);
  for(i = 0; i < file_count; i++){
    current_file_info = &(data_info_array[i]);
    free(current_file_info->buf);
    H5Dclose(current_file_info->shared_output_dsetid);
    H5Sclose(current_file_info->open_space_id);
    H5Dclose(current_file_info->open_dataset_id);
  }

  for(i = 0; i < extra_file_count; i++){
    current_file_info = &(extra_data_info_array[i]);
    free(current_file_info->buf);
    H5Dclose(current_file_info->shared_output_dsetid);
    //H5Sclose(current_file_info->open_space_id);
    H5Dclose(current_file_info->open_dataset_id);
  }

  //H5Dclose(data_info_array[0].mapping_dsetid);
  //H5Fclose(data_info_array[0].mapping_fileid);
  H5Fclose(data_info_array[0].shared_output_fileid);
  H5Fclose(file_id);


  //MPI_Info_free(&info);
  MPI_Finalize();

  return 0;
}
