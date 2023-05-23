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
#include "sds-hdf5.h"

//Defined in sds-query.c
extern char        client_sds_root_path[MAX_FILE_NAME_LENGTH];

void hdf5_append_all(SDS_Object  *obj, SDS_Bool merge, SDS_Query_comm comm, char *fname){
  hid_t    file_id, group_id, dset_id, plist_id, file_plist_id, plist2_id, dataspace_id;
  char     temp_dir_name[255];
  char     temp_file_name[255], file_name[255];
  char     dataset_name[255];
  hsize_t  my_size, my_offset, rest_size, dims_out[1];
  int      mpi_size, mpi_rank, rank, ret;
  hid_t    data_space, mem_space;
  hsize_t  i, j, *size_vector, my_data_size, file_size, my_data_offset;

#ifdef SDS_MPI
  MPI_Info info = MPI_INFO_NULL;  
  MPI_Comm_size(comm, &mpi_size);
  MPI_Comm_rank(comm, &mpi_rank);
#else
  mpi_size = 1;
  mpi_rank = 0;
#endif
  
  
  my_data_size = SDS_Object_get_data_size(obj);
  log_msg("Write data (size = %d )with HDF5 functioin to [%s]", my_data_size, fname);
  
#ifdef SDS_MPI
  file_plist_id      = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fapl_mpio(file_plist_id, comm, info); 
#else
  file_plist_id      = H5P_DEFAULT;
#endif


  //Check if the reult file existing.
  // If it does, open it and append data to it
  // If it doesn't, create a empty one and append data to it
  if(file_exist(fname)){
    log_msg("Open existing file to store results !");
    file_id  = H5Fopen(fname,     H5F_ACC_RDWR, file_plist_id);
    group_id = H5Gopen(file_id,   obj->group, H5P_DEFAULT);
    dset_id  = H5Dopen(group_id,  obj->dsetname, H5P_DEFAULT);
  }else{
    log_msg("Create a new file %s to store results !", fname);
    file_id  = H5Fcreate(fname, H5F_ACC_TRUNC, H5P_DEFAULT, file_plist_id);
    group_id = H5Gcreate(file_id,   obj->group, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT); 
    
    hsize_t dims[1]    = {0};
    hsize_t maxdims[1] = {H5S_UNLIMITED};
    
    // Modify dataset creation properties, i.e. enable chunking 
    // Set the chunk size to the data size of the first object
    hsize_t      chunk_dims[1] = {my_data_size};
    plist_id = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_chunk(plist_id, 1, chunk_dims);
    
    //Only support 1 dimensional dataset now
    dataspace_id = H5Screate_simple(1, dims, maxdims); 
    
    // Create a new dataset within the file using chunk creation properties.
    switch(obj->data_type){ 
      case SDS_INT:
        dset_id = H5Dcreate(group_id, obj->dsetname, H5T_STD_I32LE,  dataspace_id, H5P_DEFAULT, plist_id, H5P_DEFAULT);
        break;
      case SDS_FLOAT:
        dset_id = H5Dcreate(group_id, obj->dsetname, H5T_IEEE_F32LE, dataspace_id, H5P_DEFAULT, plist_id, H5P_DEFAULT);
        break;
      case SDS_DOUBLE:
        dset_id = H5Dcreate(group_id, obj->dsetname, H5T_IEEE_F64LE, dataspace_id, H5P_DEFAULT, plist_id, H5P_DEFAULT);
        break;
      default:
        log_quit("Unknown data type in hdf5_append_all()");
        break;
    }
    
    if(dset_id < 0){
      log_quit("Error in creating a  datasets in hdf5_append_all() \n");
    }
    H5Sclose (dataspace_id);
  }
  
  /* Select a hyperslab in extended portion of dataset  */
  data_space = H5Dget_space (dset_id);
  H5Sget_simple_extent_dims(data_space, dims_out, NULL);


  //Get the size of local data and global offset to write this time
  //1D dataset are supported new 
  //To do: support multi-dimensional dataset
  size_vector = malloc(mpi_size*sizeof(hsize_t));
#ifdef SDS_MPI
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Allgather(&my_data_size,  1, MPI_UNSIGNED_LONG_LONG, size_vector, 1, MPI_UNSIGNED_LONG_LONG, MPI_COMM_WORLD);
#else
  size_vector[0] = my_data_size;
#endif
  
  //Get the new data size
  file_size = dims_out[0]; 
  for (i = 0; i < mpi_size; i++) {
    file_size = file_size + size_vector[i];
  }

  //Set the extended file size
  H5Dset_extent(dset_id, &file_size);
  
  //Append the data to the end
  my_data_offset = dims_out[0]; 
  for (i = 0; i < mpi_rank; i++) {
    my_data_offset = my_data_offset + size_vector[i];
  }
  
  
  //Create dataspace_id for the whole file
  dataspace_id = H5Screate_simple(1, &file_size, NULL);
  //Create mem_space for the local memory 
  mem_space = H5Screate_simple(1, &my_data_size, NULL);

  //#ifdef SDS_MPI
  //Create the dataset and write the data
  //plist2_id = H5Pcreate(H5P_DATASET_XFER);
  //H5Pset_dxpl_mpio(plist2_id, H5FD_MPIO_COLLECTIVE);
  //#else
  //plist2_id = H5P_DEFAULT;
  //#endif
  
  void *temp_buf;
  double *tp;
  switch(obj->data_type){ 
    case SDS_INT :
      data_space = H5Dget_space(dset_id);
      H5Sselect_hyperslab(data_space, H5S_SELECT_SET, &my_data_offset, NULL, &my_data_size , NULL);
      temp_buf = malloc(sizeof(int) * my_data_size);
      SDS_Object_dup_data(obj, temp_buf);
      ret = H5Dwrite(dset_id, H5T_NATIVE_INT, mem_space, data_space, H5P_DEFAULT, temp_buf);
      break;
    case SDS_FLOAT:
      data_space = H5Dget_space(dset_id);
      H5Sselect_hyperslab(data_space, H5S_SELECT_SET, &my_data_offset, NULL, &my_data_size , NULL);
      temp_buf = malloc(sizeof(float) * my_data_size);
      SDS_Object_dup_data(obj, temp_buf);
      ret = H5Dwrite(dset_id, H5T_NATIVE_FLOAT, mem_space, data_space, H5P_DEFAULT, temp_buf);
      break;
    case SDS_DOUBLE:
      data_space = H5Dget_space(dset_id);
      H5Sselect_hyperslab(data_space, H5S_SELECT_SET, &my_data_offset, NULL, &my_data_size , NULL);
      temp_buf = malloc(sizeof(double) * my_data_size);
      SDS_Object_dup_data(obj, temp_buf);
      //tp=SDS_Object_dup_data(obj);
      //printf("%f, %f \n", ((double *)temp_buf)[0], ((double *)temp_buf)[1]);
      ret = H5Dwrite(dset_id, H5T_NATIVE_DOUBLE, mem_space, data_space, H5P_DEFAULT, temp_buf);
      //free(tp);
      break;
    default:
      log_quit("Unknown data type in hdf5_read_all()");
      break;
  }

  free(temp_buf);
  if(ret < 0){
    log_quit("Read error in hdf5_read_all");
  }

  H5Sclose(dataspace_id);
  H5Sclose(data_space);
  H5Sclose(mem_space);
#ifdef SDS_MPI
  H5Pclose(file_plist_id);
#endif
#ifdef SDS_MPI
  H5Pclose(plist_id);
#endif

  //#ifdef SDS_MPI
  //H5Pclose(plist2_id);
  //#endif
  H5Dclose(dset_id);
  H5Gclose(group_id);
  H5Fclose(file_id);
}

void hdf5_write_all(SDS_Object  *obj, SDS_Bool merge, SDS_Query_comm comm){
  hid_t    file_id, group_id, dset_id, plist_id, dataspace_id;
  char     temp_dir_name[255];
  char     temp_file_name[255], file_name[255];
  char     dataset_name[255];
  hsize_t  my_size, my_offset, rest_size, dims_out[1];
  int      mpi_size, mpi_rank, rank, ret;
  hid_t    data_space, mem_space;
  hsize_t  i, j, *size_vector, my_data_size, file_size, my_data_offset;

#ifdef SDS_MPI
  MPI_Info info = MPI_INFO_NULL; 
  log_msg("Write data with HDF5 functioin to SDS_ROOT_PATH");
  MPI_Comm_size(comm, &mpi_size);
  MPI_Comm_rank(comm, &mpi_rank);
#else
  mpi_size = 1;
  mpi_rank = 0;
#endif
  //Get the size of local data and global offset to write data
  //1D dataset are supported new 
  //To do: support multi-dimensional dataset
  my_data_size = SDS_Object_get_data_size(obj);
  size_vector = malloc(mpi_size*sizeof(hsize_t));
#ifdef SDS_MPI
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Allgather(&my_data_size,  1, MPI_UNSIGNED_LONG_LONG, size_vector, 1, MPI_UNSIGNED_LONG_LONG, MPI_COMM_WORLD);
#else
  size_vector[0] = my_data_size;
#endif
  
  file_size = 0;
  for (i = 0; i < mpi_size; i++) {
    file_size = file_size + size_vector[i];
  }
  my_data_offset = 0;
  for (i = 0; i < mpi_rank; i++) {
    my_data_offset = my_data_offset + size_vector[i];
  }


  //Create file and group here. Create dataset later 
  //Todo: group might be multiple levels 
#ifdef SDS_MPI
  plist_id      = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fapl_mpio(plist_id, comm, info); 
#else
  plist_id = H5P_DEFAULT;
#endif

  split_path(obj->filename, temp_dir_name, temp_file_name); //Write data under SDS_ROOT_PATH as cache 
  if(dir_with_last_slash(client_sds_root_path) == SDS_TRUE){
    sprintf(file_name, "%s%s", client_sds_root_path, temp_file_name);
  }else{
    sprintf(file_name, "%s/%s", client_sds_root_path, temp_file_name);
  }
  file_id      = H5Fcreate(file_name, H5F_ACC_TRUNC, H5P_DEFAULT, plist_id); //Todo: check file existing 
  group_id     = H5Gcreate(file_id,   obj->group, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT); 
  if(dir_with_last_slash(obj->group) == SDS_TRUE){
    sprintf(dataset_name, "%s%s", obj->group, obj->dsetname);      
  }else{
    sprintf(dataset_name, "%s/%s", obj->group, obj->dsetname);
  }
  
  

  //Create dataspace_id for the whole file
  dataspace_id = H5Screate_simple(1, &file_size, NULL);
  //Create mem_space for the local memory 
  mem_space = H5Screate_simple(1, &my_data_size, NULL);

  //Create the dataset and write the data
  //plist2_id = H5Pcreate(H5P_DATASET_XFER);
  //H5Pset_dxpl_mpio(plist2_id, H5FD_MPIO_COLLECTIVE);
  //plist2_id = H5P_DEFAULT;
  void *temp_buf;
  switch(obj->data_type){ 
    case SDS_INT :
      dset_id   = H5Dcreate(group_id,  obj->dsetname, H5T_STD_I32LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
      data_space = H5Dget_space(dset_id);
      H5Sselect_hyperslab(data_space, H5S_SELECT_SET, &my_data_offset, NULL, &my_data_size , NULL);
      temp_buf = malloc(sizeof(int) * my_data_size);
      SDS_Object_dup_data(obj, temp_buf);
      //temp_buf = SDS_Object_dup_data(obj);
      ret = H5Dwrite(dset_id, H5T_NATIVE_INT, mem_space, data_space,H5P_DEFAULT, temp_buf);
      break;
    case SDS_FLOAT:
      dset_id   = H5Dcreate(group_id,  obj->dsetname, H5T_IEEE_F32LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
      data_space = H5Dget_space(dset_id);
      H5Sselect_hyperslab(data_space, H5S_SELECT_SET, &my_data_offset, NULL, &my_data_size , NULL);
      temp_buf = malloc(sizeof(float) * my_data_size);
      SDS_Object_dup_data(obj, temp_buf);
      //temp_buf = SDS_Object_dup_data(obj);
      ret = H5Dwrite(dset_id, H5T_NATIVE_FLOAT, mem_space, data_space, H5P_DEFAULT, temp_buf);
      break;
    case SDS_DOUBLE:
      dset_id   = H5Dcreate(group_id,  obj->dsetname, H5T_IEEE_F64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
      data_space = H5Dget_space(dset_id);
      H5Sselect_hyperslab(data_space, H5S_SELECT_SET, &my_data_offset, NULL, &my_data_size , NULL);
      temp_buf = malloc(sizeof(double) * my_data_size);
      SDS_Object_dup_data(obj, temp_buf);
      //temp_buf =  SDS_Object_dup_data(obj);
      ret = H5Dwrite(dset_id, H5T_NATIVE_DOUBLE, mem_space, data_space, H5P_DEFAULT, temp_buf);
      break;
    default:
      log_quit("Unknown data type in hdf5_read_all()");
      break;
  }

  free(temp_buf);
  if(ret < 0){
    log_quit("Read error in hdf5_read_all");
  }

  H5Sclose(dataspace_id);
  H5Sclose(data_space);
  H5Sclose(mem_space);
  H5Pclose(plist_id);
  //H5Pclose(plist2_id);
  H5Dclose(dset_id);
  H5Gclose(group_id);
  H5Fclose(file_id);

}

void hdf5_read_all(SDS_Object   *obj, SDS_Query_comm comm){
  hid_t    file_id, dset_id, plist_id, plist2_id;
  char     dataset_name[255];
  hsize_t  my_size, my_offset, rest_size, dims_out[1], i;
  int      mpi_size, mpi_rank, rank, ret;
  hid_t    dataspace, memspace;
  
#ifdef SDS_MPI
  MPI_Info info = MPI_INFO_NULL; 
  plist_id      = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fapl_mpio(plist_id, comm, info);
#else
  plist_id      = H5P_DEFAULT;
#endif
  
  log_msg("Read data with HDF5 functioin: ");
  //Open file and dataset
  file_id = H5Fopen(obj->filename, H5F_ACC_RDONLY, plist_id);
  if(dir_with_last_slash(obj->group) == SDS_TRUE){
    sprintf(dataset_name, "%s%s", obj->group, obj->dsetname);
  }else{
    sprintf(dataset_name, "%s/%s", obj->group, obj->dsetname);
  }
  dset_id  = H5Dopen(file_id, dataset_name, H5P_DEFAULT);
  
  //Get rank and size of each dimension
  dataspace = H5Dget_space(dset_id);
  rank = H5Sget_simple_extent_ndims(dataspace);
  H5Sget_simple_extent_dims(dataspace, dims_out, NULL);

  obj->array_attribute.dims=rank;
  for(i = 0; i < rank ; i++)
    obj->array_attribute.length[i] = dims_out[i];

#ifdef SDS_MPI
  MPI_Comm_size(comm, &mpi_size);
  MPI_Comm_rank(comm, &mpi_rank);
#else
  mpi_size = 1;
  mpi_rank = 0;
#endif  

  //Partition the dataset
  //only support 1D dataset now 
  //Todo: add more support to n-D
  dims_out[0] = dims_out[0];
  rest_size = dims_out[0] % mpi_size;
  if (mpi_rank ==  (mpi_size - 1)){
    my_size = dims_out[0]/mpi_size + rest_size;
  }else{
    my_size = dims_out[0]/mpi_size;
  }

  my_offset = mpi_rank * (dims_out[0]/mpi_size);
  
  obj->my_size = my_size;
  obj->my_offset = my_offset;

  memspace =  H5Screate_simple(rank, &my_size, NULL);
  H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, &my_offset, NULL, &my_size , NULL);

  //Read the data
  //#ifdef SDS_MPI
  //plist2_id = H5Pcreate(H5P_DATASET_XFER);
  //H5Pset_dxpl_mpio(plist2_id, H5FD_MPIO_COLLECTIVE);
  //#else
  //plist2_id = H5P_DEFAULT;
  //#endif

  void *temp_buf;
  SDS_Value_union *result_buf;
  obj->data_buffer = malloc(my_size * sizeof(SDS_Value_union));
  result_buf = obj->data_buffer;
  switch(obj->data_type){ 
    case SDS_INT :
      temp_buf = malloc(my_size * sizeof(int));
      ret = H5Dread(dset_id, H5T_NATIVE_INT, memspace, dataspace, H5P_DEFAULT, temp_buf);
      for(i = 0 ; i < my_size; i++)
        result_buf[i].i = ((int *)temp_buf)[i];
      break;
    case SDS_FLOAT:
      temp_buf = malloc(my_size * sizeof(float));
      ret = H5Dread(dset_id, H5T_NATIVE_FLOAT, memspace, dataspace, H5P_DEFAULT, temp_buf);
      for(i = 0 ; i < my_size; i++){
        result_buf[i].f = ((float *)temp_buf)[i];
        // printf("Float %f ", result_buf[i].f);
      }
      break;
    case SDS_DOUBLE:
      temp_buf = malloc(my_size * sizeof(double));
      ret = H5Dread(dset_id, H5T_NATIVE_DOUBLE, memspace, dataspace, H5P_DEFAULT, temp_buf);
      for(i = 0 ; i < my_size; i++)
        result_buf[i].d = ((double *)temp_buf)[i];
      break;
    default:
      log_quit("Unknown data type in hdf5_read_all()");
      break;
  }

  free(temp_buf);
  if(ret < 0){
    log_quit("Read error in hdf5_read_all");
  }

  H5Sclose(dataspace);
  H5Sclose(memspace);
  H5Pclose(plist_id);
  //H5Pclose(plist2_id);
  H5Dclose(dset_id);
  H5Fclose(file_id);

}

void hdf5_write_range(SDS_Object  *obj){

}

void hdf5_read_range(SDS_Object   *obj){

} 


int hdf5_dataset_datatype(char *file, char *group, char *dset){
  log_msg("Open existing file to find its data type!");
  hid_t    file_id, group_id, dset_id;
  H5T_class_t class;                 /* datatype class */
  hid_t       datatype;

  file_id  = H5Fopen(file,     H5F_ACC_RDWR, H5P_DEFAULT);
  group_id = H5Gopen(file_id,   group, H5P_DEFAULT);
  dset_id  = H5Dopen(group_id,  dset,  H5P_DEFAULT);


  datatype  = H5Dget_type(dset_id);     /* datatype handle */ 
  class     = H5Tget_class(datatype);  
  
  switch(class){
    case H5T_INTEGER:
      return SDS_INT;
    case H5T_FLOAT:
      printf("File is in float format !\n");
      return SDS_FLOAT;
      //case H5T_DOUBLE:
      //return SDS_DOUBLE;
    default:
      return SDS_UNKNOWN_TYPE;
  }

  H5Dclose(dset_id);
  H5Gclose(group_id);  
  H5Fclose(file_id);

  return SDS_UNKNOWN_TYPE;
}
