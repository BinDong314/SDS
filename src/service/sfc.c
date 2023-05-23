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

/*
 * This file contains the functions to reorganize the data file with Hilbert order
 * Author: Houjun Tang
 * Copyright 2016 the Regents of the University of California
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
#define DIM_MAX  10

uint Morton_2D_Encode_16bit( uint index2, uint index1 )
{   // pack 2 16-bit indices into a 32-bit Morton code
    index1 &= 0x0000ffff;
    index2 &= 0x0000ffff;
    index1 |= ( index1 << 8 );
    index2 |= ( index2 << 8 );
    index1 &= 0x00ff00ff;
    index2 &= 0x00ff00ff;
    index1 |= ( index1 << 4 );
    index2 |= ( index2 << 4 );
    index1 &= 0x0f0f0f0f;
    index2 &= 0x0f0f0f0f;
    index1 |= ( index1 << 2 );
    index2 |= ( index2 << 2 );
    index1 &= 0x33333333;
    index2 &= 0x33333333;
    index1 |= ( index1 << 1 );
    index2 |= ( index2 << 1 );
    index1 &= 0x55555555;
    index2 &= 0x55555555;
    return( index1 | ( index2 << 1 ) );
}

void Morton_2D_Decode_16bit( const uint morton, uint* index1, uint* index2 )
{   // unpack 2 16-bit indices from a 32-bit Morton code
    uint value1 = morton;
    uint value2 = ( value1 >> 1 );
    value1 &= 0x55555555;
    value2 &= 0x55555555;
    value1 |= ( value1 >> 1 );
    value2 |= ( value2 >> 1 );
    value1 &= 0x33333333;
    value2 &= 0x33333333;
    value1 |= ( value1 >> 2 );
    value2 |= ( value2 >> 2 );
    value1 &= 0x0f0f0f0f;
    value2 &= 0x0f0f0f0f;
    value1 |= ( value1 >> 4 );
    value2 |= ( value2 >> 4 );
    value1 &= 0x00ff00ff;
    value2 &= 0x00ff00ff;
    value1 |= ( value1 >> 8 );
    value2 |= ( value2 >> 8 );
    value1 &= 0x0000ffff;
    value2 &= 0x0000ffff;
    *index1 = value1;
    *index2 = value2;
}

uint Morton_3D_Encode_10bit( uint index3, uint index2, uint index1 )
{   // pack 3 10-bit indices into a 30-bit Morton code
    index1 &= 0x000003ff;
    index2 &= 0x000003ff;
    index3 &= 0x000003ff;
    index1 |= ( index1 << 16 );
    index2 |= ( index2 << 16 );
    index3 |= ( index3 << 16 );
    index1 &= 0x030000ff;
    index2 &= 0x030000ff;
    index3 &= 0x030000ff;
    index1 |= ( index1 << 8 );
    index2 |= ( index2 << 8 );
    index3 |= ( index3 << 8 );
    index1 &= 0x0300f00f;
    index2 &= 0x0300f00f;
    index3 &= 0x0300f00f;
    index1 |= ( index1 << 4 );
    index2 |= ( index2 << 4 );
    index3 |= ( index3 << 4 );
    index1 &= 0x030c30c3;
    index2 &= 0x030c30c3;
    index3 &= 0x030c30c3;
    index1 |= ( index1 << 2 );
    index2 |= ( index2 << 2 );
    index3 |= ( index3 << 2 );
    index1 &= 0x09249249;
    index2 &= 0x09249249;
    index3 &= 0x09249249;
    return( index1 | ( index2 << 1 ) | ( index3 << 2 ) );
}

void Morton_3D_Decode_10bit( const uint morton, uint* index3, uint* index2, uint* index1 )
{   // unpack 3 10-bit indices from a 30-bit Morton code
    uint value1 = morton;
    uint value2 = ( value1 >> 1 );
    uint value3 = ( value1 >> 2 );
    value1 &= 0x09249249;
    value2 &= 0x09249249;
    value3 &= 0x09249249;
    value1 |= ( value1 >> 2 );
    value2 |= ( value2 >> 2 );
    value3 |= ( value3 >> 2 );
    value1 &= 0x030c30c3;
    value2 &= 0x030c30c3;
    value3 &= 0x030c30c3;
    value1 |= ( value1 >> 4 );
    value2 |= ( value2 >> 4 );
    value3 |= ( value3 >> 4 );
    value1 &= 0x0300f00f;
    value2 &= 0x0300f00f;
    value3 &= 0x0300f00f;
    value1 |= ( value1 >> 8 );
    value2 |= ( value2 >> 8 );
    value3 |= ( value3 >> 8 );
    value1 &= 0x030000ff;
    value2 &= 0x030000ff;
    value3 &= 0x030000ff;
    value1 |= ( value1 >> 16 );
    value2 |= ( value2 >> 16 );
    value3 |= ( value3 >> 16 );
    value1 &= 0x000003ff;
    value2 &= 0x000003ff;
    value3 &= 0x000003ff;
    *index1 = value1;
    *index2 = value2;
    *index3 = value3;
}

uint Morton_to_Hilbert2D( const uint morton, const uint bits )
{
    uint hilbert = 0;
    uint remap = 0xb4;
    uint block = ( bits << 1 );
    while( block )
    {
        block -= 2;
        uint mcode = ( ( morton >> block ) & 3 );
        uint hcode = ( ( remap >> ( mcode << 1 ) ) & 3 );
        remap ^= ( 0x82000028 >> ( hcode << 3 ) );
        hilbert = ( ( hilbert << 2 ) + hcode );
    }
    return( hilbert );
}

uint Hilbert_to_Morton2D( const uint hilbert, const uint bits )
{
    uint morton = 0;
    uint remap = 0xb4;
    uint block = ( bits << 1 );
    while( block )
    {
        block -= 2;
        uint hcode = ( ( hilbert >> block ) & 3 );
        uint mcode = ( ( remap >> ( hcode << 1 ) ) & 3 );
        remap ^= ( 0x330000cc >> ( hcode << 3 ) );
        morton = ( ( morton << 2 ) + mcode );
    }
    return( morton );
}

uint Morton_to_Hilbert3D( const uint morton, const uint bits )
{
    uint hilbert = morton;
    if( bits > 1 )
    {
        uint block = ( ( bits * 3 ) - 3 );
        uint hcode = ( ( hilbert >> block ) & 7 );
        uint mcode, shift, signs;
        shift = signs = 0;
        while( block )
        {
            block -= 3;
            hcode <<= 2;
            mcode = ( ( 0x20212021 >> hcode ) & 3 );
            shift = ( ( 0x48 >> ( 7 - shift - mcode ) ) & 3 );
            signs = ( ( signs | ( signs << 3 ) ) >> mcode );
            signs = ( ( signs ^ ( 0x53560300 >> hcode ) ) & 7 );
            mcode = ( ( hilbert >> block ) & 7 );
            hcode = mcode;
            hcode = ( ( ( hcode | ( hcode << 3 ) ) >> shift ) & 7 );
            hcode ^= signs;
            hilbert ^= ( ( mcode ^ hcode ) << block );
        }
    }
    hilbert ^= ( ( hilbert >> 1 ) & 0x92492492 );
    hilbert ^= ( ( hilbert & 0x92492492 ) >> 1 );
    return( hilbert );
}

uint Hilbert_to_Morton3D( const uint hilbert, const uint bits )
{
    uint morton = hilbert;
    morton ^= ( ( morton & 0x92492492 ) >> 1 );
    morton ^= ( ( morton >> 1 ) & 0x92492492 );
    if( bits > 1 )
    {
        uint block = ( ( bits * 3 ) - 3 );
        uint hcode = ( ( morton >> block ) & 7 );
        uint mcode, shift, signs;
        shift = signs = 0;
        while( block )
        {
            block -= 3;
            hcode <<= 2;
            mcode = ( ( 0x20212021 >> hcode ) & 3 );
            shift = ( ( 0x48 >> ( 4 - shift + mcode ) ) & 3 );
            signs = ( ( signs | ( signs << 3 ) ) >> mcode );
            signs = ( ( signs ^ ( 0x53560300 >> hcode ) ) & 7 );
            hcode = ( ( morton >> block ) & 7 );
            mcode = hcode;
            mcode ^= signs;
            mcode = ( ( ( mcode | ( mcode << 3 ) ) >> shift ) & 7 );
            morton ^= ( ( hcode ^ mcode ) << block );
        }
    }
    return( morton );
}

// Above code copied and slightly modified

int is_power_of_2(int n)
{
    return n == 1 || (n & (n-1)) == 0;
}

int get_power_of_2(int n)
{
    int count = 0;
    while (n > 1) {
        n = n >> 1;
        count++;
    }
    return count;
}

int dim_check(int ndim, int *dims)
{
    // currently both z and hilbert convertion is restricted to equal dim size across all dims
    // and dim size must be power of 2
    int i;
    for (i = 0; i < ndim; i++) {
        // check if any is zero
        if (dims[i] == 0) {
            fprintf(stderr, "dim_check() error: dims[%d]=%d\n", i, dims[i]);
            return -1;
        }

        if (is_power_of_2(dims[i]) != 1) {
            fprintf(stderr, "dim_check() error: dims[%d]=%d is not power of 2\n", i, dims[i]);
            return -1;
        }

        // check if all dim size is equal
        if (i > 0) {
            if (dims[i] != dims[i-1]) {
                fprintf(stderr, "dim_check() error: dims[%d]=%d is not equal to dims[%d]=%d\n", i, dims[i], i-1, dims[i-1]);
                return -1;
            }
        }
    }

    return 1;
}

int coord_to_z(int ndim, int *dims, int* coord)
{
    if (dim_check(ndim, dims) == -1) {
        fprintf(stderr, "coord_to_z() error\n");
        return -1;
    }

    if (ndim == 2) {
        // check the coord are within 16bit
        if (coord[0] > 65536 || coord[1] > 65536) {
            fprintf(stderr, "coord2z(2D) error, current coord (%d, %d) exceeds supported 16-bit\n", coord[0], coord[1]);
            return -1;
        }
        else {
            return (int)Morton_2D_Encode_16bit((uint)coord[0], (uint)coord[1]);
        }

    }
    else if (ndim == 3) {
        // check the coord are within 16bit
        if (coord[0] > 1024 || coord[1] > 1024) {
            fprintf(stderr, "coord2z(3D) error, current coord (%d, %d, %d) exceeds supported 10-bit\n", coord[0], coord[1], coord[2]);
            return -1;
        }
        else {
            return (int)Morton_3D_Encode_10bit((uint)coord[0], (uint)coord[1], (uint)coord[2]);
        }
    }
    else {
        fprintf(stderr,"coord2z() error, current dim :%d is not supported\n", ndim);
        return -1;
    }

    return -1;
}

int coord_to_hilbert(int ndim, int *dims, int* coord)
{
    if (dim_check(ndim, dims) == -1) {
        fprintf(stderr, "coord_to_hilbert() error\n");
        return -1;
    }

    // get z order index first
    int zid;
    zid = coord_to_z(ndim, dims, coord);

    int size = get_power_of_2(dims[0]);

    if (ndim == 2) {
        return (int)Morton_to_Hilbert2D((uint)zid, (uint)size);
    }
    else if (ndim == 3) {
        return (int)Morton_to_Hilbert3D((uint)zid, (uint)size);
    }

    return -1;
}

int coord_to_idx(int ndim, int *dims, int* coord)
{
    // linearize the coord to pos
    int i, pos, mul;

    pos = 0;
    mul = 1;
    for (i = ndim - 1; i >= 0; i--) {
        pos += coord[i] * mul;
        mul *= dims[i];
    }

    return pos;
}

int idx_to_coord(int ndim, int *dims, int idx, int* coord)
{
    int i, mod;
    int *dimSize;

    dimSize = (int*)malloc(ndim*sizeof(int));
    dimSize[0] = dims[0];

    // total number of elements
    for (i = 1; i < ndim; i++) {
        dimSize[i] = dimSize[i-1] * dims[i];
    }


    mod = idx;
    for (i = 0; i < ndim-1; i++) {
        coord[i] = mod / dimSize[ndim-i-2];
        mod      = mod % dimSize[ndim-i-2];
    }
    coord[ndim-1] = mod % dimSize[ndim-i-1];

    free(dimSize);
}

void  print_help(){
  char *msg="Usage: %s [OPTION] \n\
          -h help (--help)\n\
          -f name of the file (only HDF5 file in current version) \n\
          -g group path within HDF5 file to data set \n\
          -d name of the dataset to be reorganized \n\
          -o output file name (Have same group and dataset as original one !) \n\
          -t type of reorganized file(0: Hilbert-SFC, 1: Morton(Z)-SFC) \n\
          -c chunk size (ndim, chunksize_0, chunksize_1, ..., chunksize_2) \n";
   fprintf(stdout, msg, "reorganize");
}


int main(int argc, const char *argv[])
{
    char            filename[NAME_MAX], group[NAME_MAX], dataset[NAME_MAX], output_filename[NAME_MAX];
    hid_t           file_id, group_id, dset_id, dataspace_id, plist2_id, memspace_id;
    int             mpi_size, mpi_rank, rank, ndim;
    int             sfc_type;
    hsize_t         chunk_size[DIM_MAX];
    hsize_t         dims_out[3], dims_x, dims_y, dims_z;
    MPI_Comm        comm = MPI_COMM_WORLD;
    MPI_Info        info = MPI_INFO_NULL;
    herr_t          status;
    int             *data;
    int             i, j, k, c;
    hsize_t         my_x_size, x_rest_size, my_hyperslab_offset[3], my_hyperslab_count[3],memspace_size[3], my_z_size;

/*     MPI_Init(&argc, &argv); */
/*     MPI_Comm_size(comm, &mpi_size); */
/*     MPI_Comm_rank(comm, &mpi_rank); */

    opterr = 0;
    while ((c = getopt (argc, argv, "f:g:d:o:t:c:h")) != -1) {
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
            sfc_type = atoi(optarg);
            break;
        case 'c':
            ndim = atoi(optarg);
            optarg++;
            for (i = 0; i < ndim; i++) {
                while(strcmp(optarg, "") == 0)
                    optarg++;
                chunk_size[i] = atoi(optarg);
                optarg++;
            }
            break;
        case 'h':
            print_help();
            return 1;
        default:
            printf("Error option [%s]\n", optarg);
            exit(-1);
        }
    }

    fprintf(stderr, "SFC type: %d\n", sfc_type);
    fprintf(stderr, "Chunk sizes: ");
    for (i = 0; i < ndim; i++) {
        fprintf(stderr, "%d, ", chunk_size[i]);
    }
    fprintf(stderr, "\n");


/*     hid_t acc_tpl; */
/*     acc_tpl = H5Pcreate(H5P_FILE_ACCESS); */
/* #ifdef SDS_HDF5_VOL */
/*     //Register SDS external native plugin */
/*     hid_t under_dapl; */
/*     hid_t vol_id, vol_id2; */
/*     under_dapl = H5Pcreate(H5P_FILE_ACCESS); */
/*     H5Pset_fapl_mpio(under_dapl, comm, info); */
/*     vol_id  = H5VLregister(&H5VL_sds_external_native_g); */
/*     external_native_plugin_id = H5VLget_plugin_id("native"); */
/*     assert(external_native_plugin_id > 0); */
/*     acc_tpl = H5Pcreate(H5P_FILE_ACCESS); */
/*     H5Pset_vol(acc_tpl, vol_id, &under_dapl); */
/*     //End of Register SDS external native plugin */
/* #else */
/*     H5Pset_fapl_mpio(acc_tpl, comm, info); */
/* #endif */

/*   //Open Original XYZ file */
/*   file_id  = H5Fopen(filename, H5F_ACC_RDONLY, acc_tpl); */
/*   group_id = H5Gopen(file_id,  group,  H5P_DEFAULT); */
/*   dset_id  = H5Dopen(group_id,  dataset, H5P_DEFAULT); */

/*   dataspace_id = H5Dget_space(dset_id); */
/*   rank         = H5Sget_simple_extent_ndims(dataspace_id); */
/*   status       = H5Sget_simple_extent_dims(dataspace_id, dims_out, NULL); */

    

/*   H5Pclose(plist2_id); */
/*   H5Sclose(dataspace_id); */
/*   H5Sclose(memspace_id); */
/*   H5Dclose(dset_id); */
/*   H5Gclose(group_id); */
/*   H5Fclose(file_id); */



/* #ifdef SDS_HDF5_VOL */
/*   //Unregistered the plugin-id */
/*   H5Pclose(under_dapl); */
/*   H5VLunregister(vol_id); */
/*   //End of Unregistered the plugin-id */
/* #endif */

  //MPI_Barrier(MPI_COMM_WORLD);

  /* MPI_Finalize(); */



    return 0;
}

