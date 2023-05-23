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


void hdf5_write_all(SDS_Object  *obj){
  
}

void hdf5_read_all(SDS_Object   *obj, SDS_Query_comm comm){
  hid_t    file_id, dset_id, plist_id, plist_id2;
  char     dataset_name[255];
  hsize_t  my_size, my_offset, rest_size, dims_out[1];
  int      mpi_size, mpi_rank, rank;
  hid_t    dataspace, memspace;

  MPI_Info info = MPI_INFO_NULL; 
  plist_id      = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fapl_mpio(plist_id, comm, info);

  sprintf(dataset_name, "%s/%s", obj->group, obj->dsetname);
  file_id = H5Fopen(obj->filename, H5F_ACC_RDONLY, plist_id);
  dset_id  = H5Dopen(file_id, dataset_name, H5P_DEFAULT);
  
  dataspace = H5Dget_space(dset_id);
  rank = H5Sget_simple_extent_ndims(dataspace);
  H5Sget_simple_extent_dims(dataspace, dims_out, NULL);

  MPI_Comm_size(comm, &mpi_size);
  MPI_Comm_rank(comm, &mpi_rank);
  
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
  
  memspace =  H5Screate_simple(current_file_info->rank, &(current_file_info->local_size), NULL);
  H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, &my_offset, NULL, &my_size , NULL);

  plist2_id = H5Pcreate(H5P_DATASET_XFER);
  H5Pset_dxpl_mpio(plist2_id, H5FD_MPIO_COLLECTIVE);
  switch(obj->data_type){ 
    case SDS_Int :
      obj->data_buffer = malloc(my_size * sizeof(int));
      ret = H5Dread(dset_id, H5T_NATIVE_INT, memspace, dataspace, plist2_id, obj->data_buffer);
      break;
    case SDS_Float:
      obj->data_buffer = malloc(my_size * sizeof(float));
      ret = H5Dread(dset_id, H5T_NATIVE_FLOAT, memspace, dataspace, plist2_id, obj->data_buffer);
      break;
    case SDS_Double:
      obj->data_buffer = malloc(my_size * sizeof(int));
      ret = H5Dread(dset_id, H5T_NATIVE_DOUBLE, memspace, dataspace, plist2_id, obj->data_buffer);
      break;
    default:
      log_quit("Unknown data type in hdf5_read_all()");
      break;
  }
  
  if(ret < 0){
    log_quit("Read error in hdf5_read_all");
  }

}

void hdf5_write_range(SDS_Object  *obj){

}

void hdf5_read_range(SDS_Object   *obj){

}

void hdf5_group_collection(SDS_Collection *coll, char *file_name, char *group_name){
  hid_t    file_id, group_id, dataset_id, type_id;
  hsize_t  num_obj;
  int      obj_class;
  char     obj_name[MAX_DATASET_NAME_LENGTH + 1];
  
  coll->object_count = 0;
  //Open HDF5 file
  file_id  = H5Fopen(file_name, H5F_ACC_RDONLY, H5P_DEFAULT);
  group_id = H5Gopen(file_id, group_name, H5P_DEFAULT);
  
  H5Gget_num_objs(group_id, &num_obj);
  for (i = 0; i < num_obj; i++)
  {
    obj_class = H5Gget_objtype_by_idx(group_id, i);
    H5Gget_objname_by_idx(group_id, i, obj_name, MAX_DATASET_NAME_LENGTH);
    /* Deal with object based on its obj_class. */
    switch(obj_class)
    {
      case H5G_DATASET:
        /* /\* Open the dataset. *\/ */
        /* strncpy(dname_array[dataset_num].dataset_name, obj_name, NAME_MAX); */
        /* /\* Open the dataset. *\/ */
        /* dataset_id = H5Dopen(gid, obj_name, H5P_DEFAULT); */
        /* typeid                              = H5Dget_type(dataset_id); */
        /* dname_array[dataset_num].type_size  = H5Tget_size(typeid); */
        /* if (max_type_size < dname_array[dataset_num].type_size){ */
        /*   max_type_size = dname_array[dataset_num].type_size; */
        /* } */
        /* dname_array[dataset_num].type_id    = getDataType(typeid); */
        /* if(dataset_num == 0){ */
        /*   dname_array[dataset_num].package_offset = 0; */
        /* }else{ */
        /*   dname_array[dataset_num].package_offset = dname_array[dataset_num-1].package_offset + dname_array[dataset_num].type_size; */
        /* } */
        /* dataset_num++; */
        break;
      default:
        printf("Unknown object class %d!", obj_class);
    }
  }

}
