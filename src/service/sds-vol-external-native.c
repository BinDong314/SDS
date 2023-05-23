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
#include "sds-vol-external-native.h"

static herr_t H5VL_sds_external_native_init(hid_t vipl_id);
static herr_t H5VL_sds_external_native_term(hid_t vtpl_id);

/* Datatype callbacks */
static void *H5VL_sds_external_native_datatype_commit(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t type_id, hid_t lcpl_id, hid_t tcpl_id, hid_t tapl_id, hid_t dxpl_id, void **req);
static void *H5VL_sds_external_native_datatype_open(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t tapl_id, hid_t dxpl_id, void **req);
static herr_t H5VL_sds_external_native_datatype_get(void *dt, H5VL_datatype_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
static herr_t H5VL_sds_external_native_datatype_close(void *dt, hid_t dxpl_id, void **req);

/* Dataset callbacks */
static void *H5VL_sds_external_native_dataset_create(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t dcpl_id, hid_t dapl_id, hid_t dxpl_id, void **req);
static void *H5VL_sds_external_native_dataset_open(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t dapl_id, hid_t dxpl_id, void **req);
static herr_t H5VL_sds_external_native_dataset_read(void *dset, hid_t mem_type_id, hid_t mem_space_id,
                                    hid_t file_space_id, hid_t plist_id, void *buf, void **req);
static herr_t H5VL_sds_external_native_dataset_write(void *dset, hid_t mem_type_id, hid_t mem_space_id,
                                     hid_t file_space_id, hid_t plist_id, const void *buf, void **req);
static herr_t H5VL_sds_external_native_dataset_get(void *dset, H5VL_dataset_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
static herr_t H5VL_sds_external_native_dataset_close(void *dset, hid_t dxpl_id, void **req);

/* File callbacks */
static void *H5VL_sds_external_native_file_create(const char *name, unsigned flags, hid_t fcpl_id, hid_t fapl_id, hid_t dxpl_id, void **req);
static void *H5VL_sds_external_native_file_open(const char *name, unsigned flags, hid_t fapl_id, hid_t dxpl_id, void **req);
static herr_t H5VL_sds_external_native_file_get(void *file, H5VL_file_get_t get_type, hid_t dxpl_id, void **req, va_list arguments);
static herr_t H5VL_sds_external_native_file_close(void *file, hid_t dxpl_id, void **req);

/* Group callbacks */
static void *H5VL_sds_external_native_group_create(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t gcpl_id, hid_t gapl_id, hid_t dxpl_id, void **req);
static herr_t H5VL_sds_external_native_group_close(void *grp, hid_t dxpl_id, void **req);

/* Link callbacks */

/* Object callbacks */
static void *H5VL_sds_external_native_object_open(void *obj, H5VL_loc_params_t loc_params, H5I_type_t *opened_type, hid_t dxpl_id, void **req);
static herr_t H5VL_sds_external_native_object_specific(void *obj, H5VL_loc_params_t loc_params, H5VL_object_specific_t specific_type, hid_t dxpl_id, void **req, va_list arguments);

hid_t external_native_plugin_id = -1;

const H5VL_class_t H5VL_sds_external_native_g = {
    SDS_EXTERNAL_NATIVE,
   "SDSEXTERNALNLNATIVE",			/* name */
    H5VL_sds_external_native_init,                              /* initialize */
    H5VL_sds_external_native_term,                              /* terminate */
    sizeof(hid_t),
    NULL,
    NULL,
    {                                           /* attribute_cls */
        NULL, //H5VL_sds_external_native_attr_create,                /* create */
        NULL, //H5VL_sds_external_native_attr_open,                  /* open */
        NULL, //H5VL_sds_external_native_attr_read,                  /* read */
        NULL, //H5VL_sds_external_native_attr_write,                 /* write */
        NULL, //H5VL_sds_external_native_attr_get,                   /* get */
        NULL, //H5VL_sds_external_native_attr_specific,              /* specific */
        NULL, //H5VL_sds_external_native_attr_optional,              /* optional */
        NULL  //H5VL_sds_external_native_attr_close                  /* close */
    },
    {                                           /* dataset_cls */
        H5VL_sds_external_native_dataset_create,                    /* create */
        H5VL_sds_external_native_dataset_open,                      /* open */
        H5VL_sds_external_native_dataset_read,                      /* read */
        H5VL_sds_external_native_dataset_write,                     /* write */
        H5VL_sds_external_native_dataset_get,                       /* get */
        NULL, //H5VL_sds_external_native_dataset_specific,          /* specific */
        NULL, //H5VL_sds_external_native_dataset_optional,          /* optional */
        H5VL_sds_external_native_dataset_close                      /* close */
    },
    {                                               /* datatype_cls */
        H5VL_sds_external_native_datatype_commit,                   /* commit */
        H5VL_sds_external_native_datatype_open,                     /* open */
        H5VL_sds_external_native_datatype_get,                      /* get_size */
        NULL, //H5VL_sds_external_native_datatype_specific,         /* specific */
        NULL, //H5VL_sds_external_native_datatype_optional,         /* optional */
        H5VL_sds_external_native_datatype_close                     /* close */
    },
    {                                           /* file_cls */
        H5VL_sds_external_native_file_create,                      /* create */
        H5VL_sds_external_native_file_open,                        /* open */
        H5VL_sds_external_native_file_get,                         /* get */
        NULL, //H5VL_sds_external_native_file_specific,            /* specific */
        NULL, //H5VL_sds_external_native_file_optional,            /* optional */
        H5VL_sds_external_native_file_close                        /* close */
    },
    {                                           /* group_cls */
        H5VL_sds_external_native_group_create,                     /* create */
        NULL, //H5VL_sds_external_native_group_open,               /* open */
        NULL, //H5VL_sds_external_native_group_get,                /* get */
        NULL, //H5VL_sds_external_native_group_specific,           /* specific */
        NULL, //H5VL_sds_external_native_group_optional,           /* optional */
        H5VL_sds_external_native_group_close                       /* close */
    },
    {                                           /* link_cls */
        NULL, //H5VL_sds_external_native_link_create,                /* create */
        NULL, //H5VL_sds_external_native_link_copy,                  /* copy */
        NULL, //H5VL_sds_external_native_link_move,                  /* move */
        NULL, //H5VL_sds_external_native_link_get,                   /* get */
        NULL, //H5VL_sds_external_native_link_specific,              /* specific */
        NULL, //H5VL_sds_external_native_link_optional,              /* optional */
    },
    {                                           /* object_cls */
        H5VL_sds_external_native_object_open,                        /* open */
        NULL, //H5VL_sds_external_native_object_copy,                /* copy */
        NULL, //H5VL_sds_external_native_object_get,                 /* get */
        H5VL_sds_external_native_object_specific,                    /* specific */
        NULL, //H5VL_sds_external_native_object_optional,            /* optional */
    },
    {
        NULL,
        NULL,
        NULL
    },
    NULL
};


static herr_t H5VL_sds_external_native_init(hid_t vipl_id)
{
    return 0;
}

static herr_t H5VL_sds_external_native_term(hid_t vtpl_id)
{
    return 0;
}

static void *
H5VL_sds_external_native_file_create(const char *name, unsigned flags, hid_t fcpl_id, hid_t fapl_id, hid_t dxpl_id, void **req)
{
    hid_t under_fapl;
    H5VL_sds_external_native_t *file;

    file = (H5VL_sds_external_native_t *)calloc(1, sizeof(H5VL_sds_external_native_t));

    under_fapl = *((hid_t *)H5Pget_vol_info(fapl_id));
    file->under_object = H5VLfile_create(name, flags, fcpl_id, under_fapl, dxpl_id, req);

    return (void *)file;
}

static void *
H5VL_sds_external_native_file_open(const char *name, unsigned flags, hid_t fapl_id, hid_t dxpl_id, void **req)
{
    hid_t under_fapl;
    H5VL_sds_external_native_t *file;

    file = (H5VL_sds_external_native_t *)calloc(1, sizeof(H5VL_sds_external_native_t));

    under_fapl = *((hid_t *)H5Pget_vol_info(fapl_id));
    file->under_object = H5VLfile_open(name, flags, under_fapl, dxpl_id, req);
    printf("This is external plugin for open ! \n");
    
    return (void *)file;
}

static herr_t 
H5VL_sds_external_native_file_get(void *file, H5VL_file_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
    H5VL_sds_external_native_t *f = (H5VL_sds_external_native_t *)file;

    H5VLfile_get(f->under_object, external_native_plugin_id, get_type, dxpl_id, req, arguments);
    return 1;
}
static herr_t 
H5VL_sds_external_native_file_close(void *file, hid_t dxpl_id, void **req)
{
    H5VL_sds_external_native_t *f = (H5VL_sds_external_native_t *)file;

    H5VLfile_close(f->under_object, external_native_plugin_id, dxpl_id, req);
    free(f);

    return 1;
}

static void *
H5VL_sds_external_native_group_create(void *obj, H5VL_loc_params_t loc_params, const char *name, 
                      hid_t gcpl_id, hid_t gapl_id, hid_t dxpl_id, void **req)
{
    H5VL_sds_external_native_t *group;
    H5VL_sds_external_native_t *o = (H5VL_sds_external_native_t *)obj;

    group = (H5VL_sds_external_native_t *)calloc(1, sizeof(H5VL_sds_external_native_t));

    group->under_object = H5VLgroup_create(o->under_object, loc_params, external_native_plugin_id, name, gcpl_id,  gapl_id, dxpl_id, req);

    return (void *)group;
}

static herr_t 
H5VL_sds_external_native_group_close(void *grp, hid_t dxpl_id, void **req)
{
    H5VL_sds_external_native_t *g = (H5VL_sds_external_native_t *)grp;

    H5VLgroup_close(g->under_object, external_native_plugin_id, dxpl_id, req);
    free(g);

    return 1;
}

static void *
H5VL_sds_external_native_datatype_commit(void *obj, H5VL_loc_params_t loc_params, const char *name, 
                         hid_t type_id, hid_t lcpl_id, hid_t tcpl_id, hid_t tapl_id, hid_t dxpl_id, void **req)
{
    H5VL_sds_external_native_t *dt;
    H5VL_sds_external_native_t *o = (H5VL_sds_external_native_t *)obj;

    dt = (H5VL_sds_external_native_t *)calloc(1, sizeof(H5VL_sds_external_native_t));

    dt->under_object = H5VLdatatype_commit(o->under_object, loc_params, external_native_plugin_id, name, 
                                           type_id, lcpl_id, tcpl_id, tapl_id, dxpl_id, req);

    return dt;
}
static void *
H5VL_sds_external_native_datatype_open(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t tapl_id, hid_t dxpl_id, void **req)
{
    H5VL_sds_external_native_t *dt;
    H5VL_sds_external_native_t *o = (H5VL_sds_external_native_t *)obj;  

    dt = (H5VL_sds_external_native_t *)calloc(1, sizeof(H5VL_sds_external_native_t));

    dt->under_object = H5VLdatatype_open(o->under_object, loc_params, external_native_plugin_id, name, tapl_id, dxpl_id, req);

    return (void *)dt;
}

static herr_t 
H5VL_sds_external_native_datatype_get(void *dt, H5VL_datatype_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
    H5VL_sds_external_native_t *o = (H5VL_sds_external_native_t *)dt;
    herr_t ret_value;

    ret_value = H5VLdatatype_get(o->under_object, external_native_plugin_id, get_type, dxpl_id, req, arguments);

    return ret_value;
}

static herr_t 
H5VL_sds_external_native_datatype_close(void *dt, hid_t dxpl_id, void **req)
{
    H5VL_sds_external_native_t *type = (H5VL_sds_external_native_t *)dt;

    assert(type->under_object);

    H5VLdatatype_close(type->under_object, external_native_plugin_id, dxpl_id, req);
    free(type);
    return 1;
}

static void *
H5VL_sds_external_native_object_open(void *obj, H5VL_loc_params_t loc_params, H5I_type_t *opened_type, hid_t dxpl_id, void **req)
{
    H5VL_sds_external_native_t *new_obj;
    H5VL_sds_external_native_t *o = (H5VL_sds_external_native_t *)obj;

    new_obj = (H5VL_sds_external_native_t *)calloc(1, sizeof(H5VL_sds_external_native_t));
    
    new_obj->under_object = H5VLobject_open(o->under_object, loc_params, external_native_plugin_id, opened_type, dxpl_id, req);

    return (void *)new_obj;
}

static herr_t 
H5VL_sds_external_native_object_specific(void *obj, H5VL_loc_params_t loc_params, H5VL_object_specific_t specific_type, 
                         hid_t dxpl_id, void **req, va_list arguments)
{
    H5VL_sds_external_native_t *o = (H5VL_sds_external_native_t *)obj;

    H5VLobject_specific(o->under_object, loc_params, external_native_plugin_id, specific_type, dxpl_id, req, arguments);

    return 1;
}

static void *
H5VL_sds_external_native_dataset_create(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t dcpl_id, hid_t dapl_id, hid_t dxpl_id, void **req) 
{
    H5VL_sds_external_native_t *dset;
    H5VL_sds_external_native_t *o = (H5VL_sds_external_native_t *)obj;

    dset = (H5VL_sds_external_native_t *)calloc(1, sizeof(H5VL_sds_external_native_t));

    dset->under_object = H5VLdataset_create(o->under_object, loc_params, external_native_plugin_id, name, dcpl_id,  dapl_id, dxpl_id, req);

    return (void *)dset;
}

static void *
H5VL_sds_external_native_dataset_open(void *obj, H5VL_loc_params_t loc_params, const char *name, hid_t dapl_id, hid_t dxpl_id, void **req)
{
    H5VL_sds_external_native_t *dset;
    H5VL_sds_external_native_t *o = (H5VL_sds_external_native_t *)obj;

    dset = (H5VL_sds_external_native_t *)calloc(1, sizeof(H5VL_sds_external_native_t));

    dset->under_object = H5VLdataset_open(o->under_object, loc_params, external_native_plugin_id, name, dapl_id, dxpl_id, req);

    return (void *)dset;
}

static herr_t 
H5VL_sds_external_native_dataset_read(void *dset, hid_t mem_type_id, hid_t mem_space_id,
                      hid_t file_space_id, hid_t plist_id, void *buf, void **req)
{
    H5VL_sds_external_native_t *d = (H5VL_sds_external_native_t *)dset;

    H5VLdataset_read(d->under_object, external_native_plugin_id, mem_type_id, mem_space_id, file_space_id, 
                     plist_id, buf, req);
    return 1;
}
static herr_t 
H5VL_sds_external_native_dataset_write(void *dset, hid_t mem_type_id, hid_t mem_space_id,
                       hid_t file_space_id, hid_t plist_id, const void *buf, void **req)
{
    H5VL_sds_external_native_t *d = (H5VL_sds_external_native_t *)dset;

    H5VLdataset_write(d->under_object, external_native_plugin_id, mem_type_id, mem_space_id, file_space_id, 
                     plist_id, buf, req);
    return 1;
}

static herr_t 
H5VL_sds_external_native_dataset_get(void *dset, H5VL_dataset_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
  H5VL_sds_external_native_t *d = (H5VL_sds_external_native_t *)dset;
  
  H5VLdataset_get(d->under_object, external_native_plugin_id, get_type,  dxpl_id, req, arguments);
  
  return 1;
}

static herr_t 
H5VL_sds_external_native_dataset_close(void *dset, hid_t dxpl_id, void **req)
{
    H5VL_sds_external_native_t *d = (H5VL_sds_external_native_t *)dset;

    H5VLdataset_close(d->under_object, external_native_plugin_id, dxpl_id, req);
    free(d);
    return 1;
}

