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

#define H5F_FRIEND

#include "sds-external-vol.h"
#include "sds-client-communicator.h"
#include "sds-config-client.h"
#include "sds-object.h"
#include <libgen.h>
#include <time.h>
#include <sys/time.h>
#include </project/projectdirs/m1248/sds-depends-packages/src/hdf5-vol-r25561/src/H5Fpkg.h>

static herr_t H5VL_sds_external_init(hid_t vipl_id);
static herr_t H5VL_sds_external_term(hid_t vtpl_id);

/* Datatype callbacks */
static void *H5VL_sds_external_datatype_commit(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t type_id, hid_t lcpl_id, hid_t tcpl_id, hid_t tapl_id, hid_t dxpl_id, void **req);
static void *H5VL_sds_external_datatype_open(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t tapl_id, hid_t dxpl_id, void **req);
static herr_t H5VL_sds_external_datatype_get(void *dt, H5VL_datatype_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
static herr_t H5VL_sds_external_datatype_close(void *dt, hid_t dxpl_id, void **req);

/* Dataset callbacks */
static void *H5VL_sds_external_dataset_create(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t dcpl_id, hid_t dapl_id, hid_t dxpl_id, void **req);
static void *H5VL_sds_external_dataset_open(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t dapl_id, hid_t dxpl_id, void **req);
static herr_t H5VL_sds_external_dataset_read(void *dset, hid_t mem_type_id, hid_t mem_space_id,
                                    hid_t file_space_id, hid_t plist_id, void *buf, void **req);
static herr_t H5VL_sds_external_dataset_write(void *dset, hid_t mem_type_id, hid_t mem_space_id,
                                     hid_t file_space_id, hid_t plist_id, const void *buf, void **req);
static herr_t H5VL_sds_external_dataset_get(void *dset, H5VL_dataset_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
static herr_t H5VL_sds_external_dataset_close(void *dset, hid_t dxpl_id, void **req);

/* File callbacks */
static void *H5VL_sds_external_file_create(const char *name, unsigned flags, hid_t fcpl_id, hid_t fapl_id, hid_t dxpl_id, void **req);
static void *H5VL_sds_external_file_open(const char *name, unsigned flags, hid_t fapl_id, hid_t dxpl_id, void **req);
static herr_t H5VL_sds_external_file_get(void *file, H5VL_file_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
static herr_t H5VL_sds_external_file_close(void *file, hid_t dxpl_id, void **req);

/* Group callbacks */
static void *H5VL_sds_external_group_create(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t gcpl_id, hid_t gapl_id, hid_t dxpl_id, void **req);
static herr_t H5VL_sds_external_group_close(void *grp, hid_t dxpl_id, void **req);

/* Link callbacks */

/* Object callbacks */
static void *H5VL_sds_external_object_open(void *obj, H5VL_loc_params_t loc_params, H5I_type_t *opened_type, hid_t dxpl_id, void **req);
static herr_t H5VL_sds_external_object_specific(void *obj, H5VL_loc_params_t loc_params, H5VL_object_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments);

hid_t native_plugin_id = -1;
// Tang: record file open time
// NOTE: assuming only one file is opened at a time.
struct timeval h5open_time;

const H5VL_class_t H5VL_sds_external_g = {
    SDS_EXTERNAL,
   "SDSEXTERNALNL",			/* name */
    H5VL_sds_external_init,                              /* initialize */
    H5VL_sds_external_term,                              /* terminate */
    sizeof(hid_t),
    NULL,
    NULL,
    {                                           /* attribute_cls */
        NULL, //H5VL_sds_external_attr_create,                /* create */
        NULL, //H5VL_sds_external_attr_open,                  /* open */
        NULL, //H5VL_sds_external_attr_read,                  /* read */
        NULL, //H5VL_sds_external_attr_write,                 /* write */
        NULL, //H5VL_sds_external_attr_get,                   /* get */
        NULL, //H5VL_sds_external_attr_specific,              /* specific */
        NULL, //H5VL_sds_external_attr_optional,              /* optional */
        NULL  //H5VL_sds_external_attr_close                  /* close */
    },
    {                                           /* dataset_cls */
        H5VL_sds_external_dataset_create,                    /* create */
        H5VL_sds_external_dataset_open,                      /* open */
        H5VL_sds_external_dataset_read,                      /* read */
        H5VL_sds_external_dataset_write,                     /* write */
        H5VL_sds_external_dataset_get,                       /* get */
        NULL, //H5VL_sds_external_dataset_specific,          /* specific */
        NULL, //H5VL_sds_external_dataset_optional,          /* optional */
        H5VL_sds_external_dataset_close                      /* close */
    },
    {                                               /* datatype_cls */
        H5VL_sds_external_datatype_commit,                   /* commit */
        H5VL_sds_external_datatype_open,                     /* open */
        H5VL_sds_external_datatype_get,                      /* get_size */
        NULL, //H5VL_sds_external_datatype_specific,         /* specific */
        NULL, //H5VL_sds_external_datatype_optional,         /* optional */
        H5VL_sds_external_datatype_close                     /* close */
    },
    {                                           /* file_cls */
        H5VL_sds_external_file_create,                      /* create */
        H5VL_sds_external_file_open,                        /* open */
        H5VL_sds_external_file_get,                         /* get */
        NULL, //H5VL_sds_external_file_specific,            /* specific */
        NULL, //H5VL_sds_external_file_optional,            /* optional */
        H5VL_sds_external_file_close                        /* close */
    },
    {                                           /* group_cls */
        H5VL_sds_external_group_create,                     /* create */
        NULL, //H5VL_sds_external_group_open,               /* open */
        NULL, //H5VL_sds_external_group_get,                /* get */
        NULL, //H5VL_sds_external_group_specific,           /* specific */
        NULL, //H5VL_sds_external_group_optional,           /* optional */
        H5VL_sds_external_group_close                       /* close */
    },
    {                                           /* link_cls */
        NULL, //H5VL_sds_external_link_create,                /* create */
        NULL, //H5VL_sds_external_link_copy,                  /* copy */
        NULL, //H5VL_sds_external_link_move,                  /* move */
        NULL, //H5VL_sds_external_link_get,                   /* get */
        NULL, //H5VL_sds_external_link_specific,              /* specific */
        NULL, //H5VL_sds_external_link_optional,              /* optional */
    },
    {                                           /* object_cls */
        H5VL_sds_external_object_open,                        /* open */
        NULL, //H5VL_sds_external_object_copy,                /* copy */
        NULL, //H5VL_sds_external_object_get,                 /* get */
        H5VL_sds_external_object_specific,                    /* specific */
        NULL, //H5VL_sds_external_object_optional,            /* optional */
    },
    {
        NULL,
        NULL,
        NULL
    },
    NULL
};


static herr_t H5VL_sds_external_init(hid_t vipl_id)
{
    return 0;
}

static herr_t H5VL_sds_external_term(hid_t vtpl_id)
{
    return 0;
}

static void *
H5VL_sds_external_file_create(const char *name, unsigned flags, hid_t fcpl_id, hid_t fapl_id, hid_t dxpl_id, void **req)
{
    hid_t under_fapl;
    H5VL_sds_external_t *file;

    file = (H5VL_sds_external_t *)calloc(1, sizeof(H5VL_sds_external_t));

    under_fapl = *((hid_t *)H5Pget_vol_info(fapl_id));
    file->under_object = H5VLfile_create(name, flags, fcpl_id, under_fapl, dxpl_id, req);

    return (void *)file;
}

static void *
H5VL_sds_external_file_open(const char *name, unsigned flags, hid_t fapl_id, hid_t dxpl_id, void **req)
{
    hid_t under_fapl;
    H5VL_sds_external_t *file;

    file = (H5VL_sds_external_t *)calloc(1, sizeof(H5VL_sds_external_t));

    under_fapl = *((hid_t *)H5Pget_vol_info(fapl_id));
    file->under_object = H5VLfile_open(name, flags, under_fapl, dxpl_id, req);
    
    //printf("%s\n", name);
    //char* ts1 = strdup(name);
    //char* ts2 = strdup(name);
    //file->dir_name  = dirname(ts1);
    file->file_name = strdup(name);
    file->fapl_id   = fapl_id;
    file->fxpl_id   = dxpl_id;
    //printf("file name: %s and dir %s \n", file->file_name, file->dir_name);
    
    // Tang: get file open time
    gettimeofday(&h5open_time, NULL);
    
    return (void *)file;
}

static herr_t 
H5VL_sds_external_file_get(void *file, H5VL_file_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
    H5VL_sds_external_t *f = (H5VL_sds_external_t *)file;

    H5VLfile_get(f->under_object, native_plugin_id, get_type, dxpl_id, req, arguments);
    return 1;
}
static herr_t 
H5VL_sds_external_file_close(void *file, hid_t dxpl_id, void **req)
{
    H5VL_sds_external_t *f = (H5VL_sds_external_t *)file;

    H5VLfile_close(f->under_object, native_plugin_id, dxpl_id, req);
    free(f);

    return 1;
}

static void *
H5VL_sds_external_group_create(void *obj, H5VL_loc_params_t loc_params, const char *name, 
                      hid_t gcpl_id, hid_t gapl_id, hid_t dxpl_id, void **req)
{
    H5VL_sds_external_t *group;
    H5VL_sds_external_t *o = (H5VL_sds_external_t *)obj;

    group = (H5VL_sds_external_t *)calloc(1, sizeof(H5VL_sds_external_t));

    group->under_object = H5VLgroup_create(o->under_object, loc_params, native_plugin_id, name, gcpl_id,  gapl_id, dxpl_id, req);

    return (void *)group;
}


static herr_t 
H5VL_sds_external_group_close(void *grp, hid_t dxpl_id, void **req)
{
    H5VL_sds_external_t *g = (H5VL_sds_external_t *)grp;

    H5VLgroup_close(g->under_object, native_plugin_id, dxpl_id, req);
    free(g);

    return 1;
}

static void *
H5VL_sds_external_datatype_commit(void *obj, H5VL_loc_params_t loc_params, const char *name, 
                         hid_t type_id, hid_t lcpl_id, hid_t tcpl_id, hid_t tapl_id, hid_t dxpl_id, void **req)
{
    H5VL_sds_external_t *dt;
    H5VL_sds_external_t *o = (H5VL_sds_external_t *)obj;

    dt = (H5VL_sds_external_t *)calloc(1, sizeof(H5VL_sds_external_t));

    dt->under_object = H5VLdatatype_commit(o->under_object, loc_params, native_plugin_id, name, 
                                           type_id, lcpl_id, tcpl_id, tapl_id, dxpl_id, req);

    return dt;
}
static void *
H5VL_sds_external_datatype_open(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t tapl_id, hid_t dxpl_id, void **req)
{
    H5VL_sds_external_t *dt;
    H5VL_sds_external_t *o = (H5VL_sds_external_t *)obj;  

    dt = (H5VL_sds_external_t *)calloc(1, sizeof(H5VL_sds_external_t));

    dt->under_object = H5VLdatatype_open(o->under_object, loc_params, native_plugin_id, name, tapl_id, dxpl_id, req);

    return (void *)dt;
}

static herr_t 
H5VL_sds_external_datatype_get(void *dt, H5VL_datatype_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
    H5VL_sds_external_t *o = (H5VL_sds_external_t *)dt;
    herr_t ret_value;

    ret_value = H5VLdatatype_get(o->under_object, native_plugin_id, get_type, dxpl_id, req, arguments);

    return ret_value;
}

static herr_t 
H5VL_sds_external_datatype_close(void *dt, hid_t dxpl_id, void **req)
{
    H5VL_sds_external_t *type = (H5VL_sds_external_t *)dt;

    assert(type->under_object);

    H5VLdatatype_close(type->under_object, native_plugin_id, dxpl_id, req);
    free(type);
    return 1;
}

static void *
H5VL_sds_external_object_open(void *obj, H5VL_loc_params_t loc_params, H5I_type_t *opened_type, hid_t dxpl_id, void **req)
{
    H5VL_sds_external_t *new_obj;
    H5VL_sds_external_t *o = (H5VL_sds_external_t *)obj;

    new_obj = (H5VL_sds_external_t *)calloc(1, sizeof(H5VL_sds_external_t));
    
    new_obj->under_object = H5VLobject_open(o->under_object, loc_params, native_plugin_id, opened_type, dxpl_id, req);

    return (void *)new_obj;
}

static herr_t 
H5VL_sds_external_object_specific(void *obj, H5VL_loc_params_t loc_params, H5VL_object_specific_t specific_type, 
                         hid_t dxpl_id, void **req, va_list arguments)
{
    H5VL_sds_external_t *o = (H5VL_sds_external_t *)obj;

    H5VLobject_specific(o->under_object, loc_params, native_plugin_id, specific_type, dxpl_id, req, arguments);

    return 1;
}

static void *
H5VL_sds_external_dataset_create(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t dcpl_id, hid_t dapl_id, hid_t dxpl_id, void **req) 
{
    H5VL_sds_external_t *dset;
    H5VL_sds_external_t *o = (H5VL_sds_external_t *)obj;

    dset = (H5VL_sds_external_t *)calloc(1, sizeof(H5VL_sds_external_t));

    dset->under_object = H5VLdataset_create(o->under_object, loc_params, native_plugin_id, name, dcpl_id,  dapl_id, dxpl_id, req);

    return (void *)dset;
}

static void *
H5VL_sds_external_dataset_open(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t dapl_id, hid_t dxpl_id, void **req)
{
    H5VL_sds_external_t *dset;
    H5VL_sds_external_t *o = (H5VL_sds_external_t *)obj;

    dset = (H5VL_sds_external_t *)calloc(1, sizeof(H5VL_sds_external_t));

    dset->under_object = H5VLdataset_open(o->under_object, loc_params, native_plugin_id, name, dapl_id, dxpl_id, req);
    char* ts1 = strdup(name);
    char* ts2 = strdup(name);
    dset->group_name   = dirname(ts1);
    dset->dataset_name = basename(ts2);
    
    dset->file_name    = o->file_name;
    //dset->dir_name     = o->dir_name;
   
    dset->fapl_id      = o->fapl_id;
    dset->fxpl_id      = o->fxpl_id;
    dset->dapl_id      = dapl_id;
    dset->dxpl_id      = dxpl_id;
    dset->loc_params   = loc_params;

    return (void *)dset;
}

int SwitchBackToMemoryMatrix(void *buf, hsize_t *my_xyz_count, hsize_t *my_xyz_offset){
  hsize_t x_range = my_xyz_count[2];
  hsize_t y_range = my_xyz_count[1];
  hsize_t z_range = my_xyz_count[0];
  hsize_t range_max;
  int i,j,k;

  unsigned short *temp_buf; 
  unsigned short *temp_buf_target; 

  temp_buf_target = buf;
  range_max = x_range*y_range*z_range;
  temp_buf = (unsigned short *)malloc(range_max*sizeof(unsigned short));
  memcpy(temp_buf, buf, range_max*sizeof(unsigned short));
  
  for(i=0; i<x_range; i++){
    for (j=0; j<y_range; j++){
      for (k=0; k<z_range; k++){
	if( ((i +  j*z_range + k*z_range*y_range) <  range_max) && ((k +  j*x_range + i*x_range*y_range) < range_max) ){
	  temp_buf_target[i +  j*z_range + k*z_range*y_range] = temp_buf[k +  j*x_range + i*x_range*y_range];
	}
      }
    }
  }
  
  free(temp_buf);
  return 1;
}

// 
int Read_SDS_ht_reorg_file_external(H5VL_sds_external_t *d, ResponseTraceData* reorg_layout, hid_t mem_type_id, hid_t mem_space_id, hid_t read_plist_id, void *buf, H5VL_loc_params_t loc_params){
  /* Open SDS file and dataset*/
  H5VL_sds_external_t       *file, *group, *dset;
  hid_t                      under_dapl,sds_fs_id, sds_mem_id;

  char                      *sds_file_name, *sds_group_name, *sds_dataset_name;
  int                        i, j;
  //H5VL_loc_params_t          loc_params;
  //

  sds_file_name     = reorg_layout->opt_file->filename;
  sds_group_name    = reorg_layout->opt_file->group;
  sds_dataset_name  = reorg_layout->opt_file->dsetname; 

  //printf("Open the SDS file [%s] and dataset [%s] \n", sds_file_name, sds_dataset_name);
  file = (H5VL_sds_external_t *)calloc(1, sizeof(H5VL_sds_external_t));
  group= (H5VL_sds_external_t *)calloc(1, sizeof(H5VL_sds_external_t));
  dset = (H5VL_sds_external_t *)calloc(1, sizeof(H5VL_sds_external_t));

  //under_dapl           = H5Pcreate(H5P_FILE_ACCESS);
  //H5Pset_fapl_mpio(under_dapl, MPI_COMM_WORLD, MPI_INFO_NULL);

  printf("Before file %s ! \n", sds_file_name);
  //file->under_object   = H5VLfile_open(&(file->under_plugin), sds_file_name, H5F_ACC_RDONLY, d->fapl_id, H5_REQUEST_NULL);
  file->under_object = H5VLfile_open(sds_file_name, H5F_ACC_RDONLY, d->fapl_id, d->dxpl_id, H5_REQUEST_NULL);
  
  // Open group
  /* group->under_object = H5VLgroup_open(file->under_object, loc_params, file->under_plugin, sds_group_name, d->dapl_id, d->dxpl_id, H5_REQUEST_NULL); */ 
      
  /* H5I_dec_app_ref(under_dapl); */
  
  loc_params.type      = H5VL_OBJECT_BY_SELF;
  loc_params.obj_type  = 1;

  // Open dataset
  //dset->under_object = H5VLdataset_open(file->under_object, loc_params, file->under_plugin, sds_dataset_name, d->dapl_id, H5_REQUEST_NULL);
  /* dset->under_object = H5VLdataset_open(group->under_object, loc_params, native_plugin_id, sds_dataset_name,  d->dapl_id, d->dxpl_id, H5_REQUEST_NULL); */
  log_msg("[C] Before open dataset  %s ! \n", sds_dataset_name);
  dset->under_object = H5VLdataset_open(file->under_object, loc_params, native_plugin_id, sds_dataset_name,  d->dapl_id, d->dxpl_id, H5_REQUEST_NULL);
  log_msg("[C] Open dataset successed!");

  dset->under_plugin   = file->under_plugin;

  //Get dataspace ID and apply hypersalb to the space
  H5VL_dataset_get(dset->under_object, dset->under_plugin, H5VL_DATASET_GET_SPACE, H5_REQUEST_NULL, &sds_fs_id);

  // Select the hyperslab using reorganized layout metadata 
  int nblock; 
  int ndim;
  hsize_t *start, *stride, *count, *block;

  nblock = reorg_layout->n_opt_hyperslab;
  if (nblock <= 0) {
      log_msg("[C] reorg_layout->n_opt_hyperslab <= 0, read failed!\n[C]exiting...");
      return -1;
  }
  ndim   = reorg_layout->opt_hyperslab[0]->n_start;
  for (i = 0; i < nblock; i++) {
      start = (hsize_t*)reorg_layout->opt_hyperslab[i]->start;
      stride= (hsize_t*)reorg_layout->opt_hyperslab[i]->stride;
      count = (hsize_t*)reorg_layout->opt_hyperslab[i]->count;
      block = (hsize_t*)reorg_layout->opt_hyperslab[i]->block;
      if (i == 0) 
          H5Sselect_hyperslab(sds_fs_id, H5S_SELECT_SET, start, stride, count, block);
      else
          H5Sselect_hyperslab(sds_fs_id, H5S_SELECT_OR, start, stride, count, block);
  }

  /* sds_mem_id = H5Screate_simple(ndim, my_xyz_count, NULL); */

  //Read the file H5P_DATASET_XFER_DEFAULT
  H5VLdataset_read(dset->under_object, file->under_plugin, mem_type_id, mem_space_id, sds_fs_id, read_plist_id, buf, H5_REQUEST_NULL);

  log_msg("Read successed!");
  
  H5I_dec_app_ref(sds_fs_id);
  /* H5I_dec_app_ref(sds_mem_id); */ 

  H5VLdataset_close(dset->under_object, native_plugin_id, d->fxpl_id, H5_REQUEST_NULL);
  /* H5VLgroup_close( group->under_object, native_plugin_id, d->fxpl_id, H5_REQUEST_NULL); */
  H5VLfile_close(   file->under_object, native_plugin_id, d->dxpl_id, H5_REQUEST_NULL);
  
  free(file);
  free(group);
  free(dset);

  return 1;
}


//Read SDS file instead of original file
int Read_SDS_File_external(H5VL_sds_external_t *d, char *sds_file_name, char * sds_dataset_name, hid_t mem_type_id, hid_t read_plist_id, void *buf, hsize_t *my_xyz_count, hsize_t *my_xyz_offset, H5VL_loc_params_t loc_params){
  /* Open SDS file and dataset*/
  H5VL_sds_external_t       *file, *dset;
  hid_t                      under_dapl,sds_fs_id, sds_mem_id;
  //H5VL_loc_params_t          loc_params;

  //printf("Open the SDS file [%s] and dataset [%s] \n", sds_file_name, sds_dataset_name);
  file = (H5VL_sds_external_t *)calloc(1, sizeof(H5VL_sds_external_t));
  dset = (H5VL_sds_external_t *)calloc(1, sizeof(H5VL_sds_external_t));

  //under_dapl           = H5Pcreate(H5P_FILE_ACCESS);
  //H5Pset_fapl_mpio(under_dapl, MPI_COMM_WORLD, MPI_INFO_NULL);
  printf("Before file %s ! \n", sds_file_name);

  //file->under_object   = H5VLfile_open(&(file->under_plugin), sds_file_name, H5F_ACC_RDONLY, d->fapl_id, H5_REQUEST_NULL);
  file->under_object = H5VLfile_open(sds_file_name, H5F_ACC_RDONLY, d->fapl_id, d->dxpl_id, H5_REQUEST_NULL);
  printf("Before open dataset  %s  ! \n", sds_dataset_name);
  if(file->under_object == NULL){
	printf("file->under_object is NULL\n");	
  }else{
	printf("file->under_object is not NULL\n");
  }
  //H5F_t *ft;
  //ft =  (H5F_t *)file->under_object;
  //if(ft->shared == NULL)
//	 printf("file->under_object is NULL");

  //H5I_dec_app_ref(under_dapl);
  
  //loc_params.type      = H5VL_OBJECT_BY_SELF;
  //loc_params.obj_type  = 1;

  ////dset->under_object   =  H5VLdataset_open(file->under_object, loc_params, file->under_plugin, sds_dataset_name, d->dapl_id, H5_REQUEST_NULL);
  dset->under_object = H5VLdataset_open(file->under_object, loc_params, native_plugin_id, sds_dataset_name,  d->dapl_id, d->dxpl_id, H5_REQUEST_NULL);

  printf("Open dataset successed !");

  dset->under_plugin   = file->under_plugin;

  //Get dataspace ID and apply hypersalb to the space
  /*H5VL_dataset_get(dset->under_object, dset->under_plugin, H5VL_DATASET_GET_SPACE, H5_REQUEST_NULL, &sds_fs_id);

  H5Sselect_hyperslab(sds_fs_id, H5S_SELECT_SET, my_xyz_offset, NULL, my_xyz_count, NULL);
  sds_mem_id = H5Screate_simple(3, my_xyz_count, NULL);

  //printf("offset (%llu, %llu, %llu) -> count (%llu, %llu, %llu) \n", my_xyz_offset[0],my_xyz_offset[1],my_xyz_offset[2],my_xyz_count[0],my_xyz_count[1],my_xyz_count[2]);
  //Read the file H5P_DATASET_XFER_DEFAULT
  //H5VLdataset_read(dset->under_object, file->under_plugin, mem_type_id, sds_mem_id, sds_fs_id, read_plist_id, buf, H5_REQUEST_NULL);
  //printf("Read successed !");
  */
  
  H5I_dec_app_ref(sds_fs_id);
  H5I_dec_app_ref(sds_mem_id); 
  //H5VLdataset_close(dset->under_object, native_plugin_id, d->fxpl_id, H5_REQUEST_NULL);
  
  H5VLfile_close(file->under_object, native_plugin_id, d->dxpl_id, H5_REQUEST_NULL);
  
  free(file);
  free(dset);

  return 1;
}

// Tang
// Trace the data selection of H5Dread using file_space_id
static int SDS_trace_read(hid_t file_space_id, H5VL_sds_external_t* sds_info, SDS_Object *obj)
{
    hsize_t nblock, nbuf;
    hsize_t *buf;
    int ndim;
    hsize_t *dims;
    double read_time;
    SDS_Pattern_type pattern_type;

    // Get ndim and dims
    ndim = H5Sget_simple_extent_ndims(file_space_id);
    dims = (hsize_t*)malloc(sizeof(hsize_t) * ndim);
    H5Sget_simple_extent_dims(file_space_id, dims, NULL);

    if (H5Sget_select_type(file_space_id) == H5S_SEL_HYPERSLABS) {
        // Hyperslab selection
        nblock    = H5Sget_select_hyper_nblocks(file_space_id);
        nbuf      = nblock* ndim * 2;
        buf       = (hsize_t*)malloc(nbuf * sizeof(hsize_t));
        H5Sget_select_hyper_blocklist(file_space_id, 0, nblock, buf );
        pattern_type  = SDS_PAPPTERN_HYPERSLAB;
    }
    else if (H5Sget_select_type(file_space_id) == H5S_SEL_POINTS) {
        // Element selection
        nblock    = H5Sget_select_elem_npoints(file_space_id);
        nbuf      = nblock * ndim;
        buf       = (hsize_t*)malloc(nbuf * sizeof(hsize_t));
        H5Sget_select_elem_pointlist(file_space_id, 0, nblock, buf );
        pattern_type  = SDS_PAPPTERN_ELEMENT;
    }
    else if (H5Sget_select_type(file_space_id) == H5S_SEL_ALL) {
        // Entire dataset
        nbuf      = 2 * ndim;
        buf       = (hsize_t*)malloc(nbuf * sizeof(hsize_t));
        int i;
        for (i = 0; i < ndim; i++) {
            buf[i*ndim]   = 0;
            buf[i*ndim+1] = dims[0];
        }
        pattern_type  = SDS_PAPPTERN_HYPERSLAB;
    }
    else if (H5Sget_select_type(file_space_id) == H5S_SEL_NONE) {
        // No selection at all 
        nbuf      = 2 * ndim;
        buf       = (hsize_t*)malloc(nbuf * sizeof(hsize_t));
        int i;
        for (i = 0; i < ndim; i++) {
            buf[i*ndim]   = 0;
            buf[i*ndim+1] = 0;
        }
        pattern_type  = SDS_PAPPTERN_HYPERSLAB;
    }
    else {
        // Error
        log_msg("[C] SDS_trace_read(): Selection Invalid!\n");
        return -1;
    }

    // Pattern includes selection_type, file_path, group_name, dset_name, hyperslab/element parameters, time
    // Hyperslab parameters: nbuf, (start[ndim], stride[ndim], count[ndim], block[ndim]), ..., (repeated nbuf times)
    // Element   parameters: "RANDOM" message for now

    // sds_info contains the following information:
    /* sds_info->file_name; */
    /* sds_info->group_name; */
    /* sds_info->dataset_name; */

    /* fprintf(stderr, "Fname: %s\n Gname:%s\n Dname:%s\n", sds_info->file_name, sds_info->group_name, sds_info->dataset_name); */

    fprintf(stderr, "\n[C]\n==== TRACE ====\n");
    int i;
    for (i = 0; i < nbuf; i++) {
        fprintf(stderr, "%llu ", buf[i]);
    }
    fprintf(stderr, "\n==== END TRACE ====\n[C]\n");


    // buf contains the selection information, nbuf is the number of elements in buf
    SDS_send_pattern_to_server(sds_info, obj, pattern_type, nbuf, buf, ndim);


    // Cleanup
    free(dims);
    free(buf);
    return 1;
}

// Tang
// Send pattern to server
int SDS_send_pattern_to_server(H5VL_sds_external_t *sds_info, SDS_Object *obj, int ptn_type, hsize_t nbuf, hsize_t *buf, int ndim)
{

    int mpi_size = 1, mpi_rank = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    int                    i, j, sockfd;
    ClientRequest          query    = CLIENT_REQUEST__INIT;
    ClientResponse        *response;
    uint64_t               q_len,   r_len;
    void                  *q_buf,  *r_buf;
  
    parse_client_file(ini_file);
  
    //Start a socket
    sockfd = SDS_Socket_start();
    if(sockfd < 0){
      return -1;
    }
  
    //Fill the query
    /* query.type                     =  MESSAGE_TYPE__QUERY; */
    query.type                     =  MESSAGE_TYPE__TRACE;
    query.trace_data               =  malloc(sizeof(RequestTraceData));
    request_trace_data__init(query.trace_data);
    query.trace_data->object       = malloc(sizeof(SdsObject));
    sds_object__init(query.trace_data->object);
    query_fill_object(query.trace_data->object, obj);

    RequestTraceData *trace_ptr = query.trace_data;

    // Share properties for both hyperslab and element selection
    trace_ptr->mpi_size     = mpi_size;
    trace_ptr->mpi_rank     = mpi_rank;
    trace_ptr->pattern_type = ptn_type;
    trace_ptr->ndim         = ndim;
    trace_ptr->time         = h5open_time.tv_sec * 1000000 + h5open_time.tv_usec;
    
    if (ptn_type == SDS_PAPPTERN_HYPERSLAB) {

        trace_ptr->n_h_pattern   = nbuf;
        /* log_msg("[C] n_h_patter: %d", nbuf); */
        trace_ptr->h_pattern     = malloc(nbuf * sizeof(Hdf5Hyperslab*));

        // Allocate space to store hyperslab parameters
        // Tang TODO: make sure the buffers below are freed
        for (i = 0; i < nbuf; i++) {
            trace_ptr->h_pattern[i] = malloc(sizeof(Hdf5Hyperslab));
            hdf5_hyperslab__init(trace_ptr->h_pattern[i]);
            trace_ptr->h_pattern[i]->start    = malloc(ndim * sizeof(uint64_t));
            trace_ptr->h_pattern[i]->stride   = malloc(ndim * sizeof(uint64_t));
            trace_ptr->h_pattern[i]->count    = malloc(ndim * sizeof(uint64_t));   // count is actually the lower right corner
            trace_ptr->h_pattern[i]->block    = malloc(ndim * sizeof(uint64_t));
        }

        // Convert hyperslab parameters from trace 
        int buf_iter = 0;
        for (i = 0; i < nbuf; i++) {
            trace_ptr->h_pattern[i]->n_start       = ndim;
            for (j = 0; j < ndim; j++) 
                trace_ptr->h_pattern[i]->start[j]  = buf[buf_iter++];

            trace_ptr->h_pattern[i]->n_stride      = ndim;
            for (j = 0; j < ndim; j++) 
                trace_ptr->h_pattern[i]->stride[j] = 1;

            trace_ptr->h_pattern[i]->n_count       = ndim;
            for (j = 0; j < ndim; j++) 
                trace_ptr->h_pattern[i]->count[j]  = buf[buf_iter++];

            trace_ptr->h_pattern[i]->n_block       = ndim;
            for (j = 0; j < ndim; j++) 
                trace_ptr->h_pattern[i]->block[j]  = 1;
        }

    }
    else if(ptn_type == SDS_PAPPTERN_ELEMENT) {

        /* fprintf(stderr, "[C] Ptn_type=%d!\n", ptn_type); */
        trace_ptr->e_pattern = malloc(sizeof(Hdf5Eselection));
        hdf5_eselection__init(trace_ptr->e_pattern);

        // Allocate space to store element selection coordinates 
        trace_ptr->e_pattern->coordination = malloc(nbuf * sizeof(uint64_t));
        
        trace_ptr->e_pattern->n_coordination = nbuf;
        for (i = 0; i < nbuf; i++) {
            trace_ptr->e_pattern->coordination[i] = buf[i];
        }

    }
    
  q_len = client_request__get_packed_size(&query);

  log_msg("[C] Trace total size: %llu", q_len);

  q_buf = malloc(q_len);

  client_request__pack(&query, q_buf);
  //Send to SDS Server
  SDS_Socket_sent(sockfd, q_buf, q_len);

  r_buf=  malloc(MAX_MESSAGE_LENGTH);
  memset(r_buf, 0, MAX_MESSAGE_LENGTH);

  log_msg("[C] Pattern sent to server");

  //Receive from SDS Server
  r_len = SDS_Socket_recv(sockfd, r_buf, MAX_MESSAGE_LENGTH);
  response = client_response__unpack(NULL, r_len, r_buf);

  if(response->error_code != QUERY_STATUS__SUCCESSFUL){
    log_quit("Response received at SDS Client with error for tracing!");
    exit(-1);
  }

  if (response != NULL && response->trace_data != NULL) {
      // Received target layout successfully, use it for redirection.
      // TODO: change obj->reorg_files (type is SDS_Reorg_file)
      /* log_msg("[C] Layout has %d blocks", response->trace_data->n_opt_hyperslab); */
      obj->ht_reorg_file = response->trace_data;
      log_msg("[C] Received layout from server, with %d slabs", response->trace_data->n_opt_hyperslab);
  }
  else 
      log_msg("[C] Received response has NULL trace_data");

  //Stop the socket
  SDS_Socket_stop(sockfd);

  //Release all memory
  ClientRequest_free(&query);
  free(q_buf);
  free(r_buf);

  return 1;
}

static herr_t 
H5VL_sds_external_dataset_read(void *dset, hid_t mem_type_id, hid_t mem_space_id,
                      hid_t file_space_id, hid_t plist_id, void *buf, void **req)
{
    H5VL_sds_external_t  *d = (H5VL_sds_external_t *)dset;
    hsize_t               count[3], offset[3], space_start[3], space_end[3];
    char                  reorganized_file[512], reorganized_dataset[512];
    //printf("dir: %s, filename: %s, group name: %s, dset name %s \n", d->dir_name, d->file_name, d->group_name, d->dataset_name);
    //Send a request to SDS Server to check wether we have a reorganized file 
    //sds_file_info_t                     reor_file_info; 
    //reor_file_info.sds_file_location    = malloc(sizeof(char)*255);
    //reor_file_info.sds_file_name        = malloc(sizeof(char)*255); 
    //reor_file_info.sds_group            = malloc(sizeof(char)*255); 
    //reor_file_info.sds_dataset_name     = malloc(sizeof(char)*255); 
    //sds_client_query(d->dir_name, d->file_name, d->group_name, d->dataset_name, &reor_file_info);
    
    SDS_Object    **obj_array;
    obj_array = malloc(sizeof(SDS_Object *));
    obj_array[0] = SDS_Object_init(d->file_name, d->group_name, d->dataset_name, SDS_HDF5, SDS_UNKNOWN_TYPE);
    SDS_read_collection_metadata(obj_array, 1);


/* #ifdef SDS_TRACE */
/*     // Tang: timing the read */ 
/*     struct timeval ht_read_start; */
/*     struct timeval ht_read_end; */
/*     long long      ht_read_elapsed; */
/*     time_t current_time; */
/*     current_time = time(NULL); */
/*     d->date_str= ctime(&current_time); */
/*     d->date_str[strlen(d->date_str)-1] = 0; // remove the trailing "\n" */
/*     gettimeofday(&ht_read_start, 0); */
/* #endif */
 printf("REC BACK !");
#ifdef SDS_TRACE
    // Record read data selection
    SDS_trace_read(file_space_id, d, obj_array[0]);     // Calls trace_log_analysis() at the end;



#endif

 
    /* if(SDS_Object_SDSfile_exist(obj_array[0]) == SDS_TRUE){ */
    if (obj_array[0]->ht_reorg_file != NULL) {
        SdsFile1 * opt_file = obj_array[0]->ht_reorg_file->opt_file;
        log_msg("[C] Target layout: file name[%s] group[%s] dset[%s], with [%d] blocks", 
             opt_file->filename, opt_file->group, opt_file->dsetname, obj_array[0]->ht_reorg_file->n_opt_hyperslab);
        
        Read_SDS_ht_reorg_file_external(dset, obj_array[0]->ht_reorg_file, mem_type_id, mem_space_id, plist_id, buf, d->loc_params);
        // original read
        /* log_msg("[C] Read from original file"); */
        /* H5VLdataset_read(d->under_object, native_plugin_id, mem_type_id, mem_space_id, file_space_id, plist_id, buf, req); */
    }
    else if(SDS_Object_SDSfile_exist(obj_array[0]) == SDS_TRUE){
      // Bin's old code to read from transposed layout only.
      printf("Use reorganized file %s (group %s, dataset %s) \n", 
              obj_array[0]->reorg_files->filename, obj_array[0]->reorg_files->group, obj_array[0]->reorg_files->dsetname);       
      
      //Current only supported reorganization is 3D matrix transpose
      H5Sget_select_bounds(file_space_id, space_start, space_end);
      
      //Change dimension
      offset[0] = space_start[2];
      offset[1] = space_start[1];
      offset[2] = space_start[0];
      count[0]  = space_end[2] - space_start[2] + 1;
      count[1]  = space_end[1] - space_start[1] + 1;
      count[2]  = space_end[0] - space_start[0] + 1;
      
      sprintf(reorganized_file,    "%s",          obj_array[0]->reorg_files->filename);
      sprintf(reorganized_dataset, "%s/%s",       obj_array[0]->reorg_files->group, obj_array[0]->reorg_files->dsetname );
      
      //sprintf(reorganized_file,    "%s/%s", reor_file_info.sds_file_location, reor_file_info.sds_file_name);
      //sprintf(reorganized_dataset, "%s/%s", reor_file_info.sds_group, reor_file_info.sds_dataset_name);
      //Read_SDS_File_external(H5VL_sds_external_t *d, char *sds_file_name, char * sds_dataset_name, hid_t mem_type_id, hid_t read_plist_id, void *buf, hid_t req, hsize_t *my_xyz_count, hsize_t *my_xyz_offset)
      Read_SDS_File_external(dset, reorganized_file, reorganized_dataset, mem_type_id, plist_id, buf, count, offset, d->loc_params);
      SwitchBackToMemoryMatrix(buf, count, offset);

    }else{
        //No optimization existing, tell read function to read whole data
        printf("Found NO SDS file for (dir %s, file %s, group %s, dataset %s), read original file. \n", d->dir_name, d->file_name, d->group_name, d->dataset_name);       
        // Actual read call
        H5VLdataset_read(d->under_object, native_plugin_id, mem_type_id, mem_space_id, file_space_id, plist_id, buf, req);
    }

    // Tang: record total elapsed time of read
    /* gettimeofday(&ht_read_end, 0); */
    /* ht_read_elapsed     = (ht_read_end.tv_sec-ht_read_start.tv_sec)*1000000LL + ht_read_end.tv_usec-ht_read_start.tv_usec; */
    /* /1* d->read_time = ht_read_elapsed / 1000000.0; *1/ */

   
    return 1;
}
static herr_t 
H5VL_sds_external_dataset_write(void *dset, hid_t mem_type_id, hid_t mem_space_id,
                       hid_t file_space_id, hid_t plist_id, const void *buf, void **req)
{
    H5VL_sds_external_t *d = (H5VL_sds_external_t *)dset;

    H5VLdataset_write(d->under_object, native_plugin_id, mem_type_id, mem_space_id, file_space_id, 
                     plist_id, buf, req);
    return 1;
}

static herr_t 
H5VL_sds_external_dataset_get(void *dset, H5VL_dataset_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
  H5VL_sds_external_t *d = (H5VL_sds_external_t *)dset;
  
  H5VLdataset_get(d->under_object, native_plugin_id, get_type,  dxpl_id, req, arguments);
  
  return 1;
}

static herr_t 
H5VL_sds_external_dataset_close(void *dset, hid_t dxpl_id, void **req)
{
    H5VL_sds_external_t *d = (H5VL_sds_external_t *)dset;

    H5VLdataset_close(d->under_object, native_plugin_id, dxpl_id, req);
    free(d);
    return 1;
}

