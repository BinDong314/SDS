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
#ifndef __SDS_COMMON_H__
#define __SDS_COMMON_H__

#ifdef SDS_CLIENT_MPI
#include "mpi.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
 
#include <getopt.h>

#define SDS_REORGANIZATION_TYPE_SORT               1

/* Port to listen on. */
#define SERVER_ADDRESS          "128.55.44.227" //Edison MOM node 14 
#define SERVER_PORT              90001

#define MAX_FILE_NAME_LENGTH     256
#define MAX_DATASET_NAME_LENGTH  256
#define MAX_GROUP_NAME_LENGTH    256
#define MAX_LOCATION_NAME_LENGTH 256
#define MAX_MESSAGE_LENGTH       1000

typedef unsigned long long int SDS_Offset;
 
typedef enum SDS_Index_type{
  SORT_INDEX      = 0,
  BITMAP_INDEX    = 1,
  MDBIN_INDEX     = 2,
  USER_DEFINED_INDEX    = 3,
  NONE_INDEX     = 4, //increase it by one after adding new type
}SDS_Index_type;

typedef enum SDS_Reorg_type{
  SORT          = 0,
  TRANSFORM     = 1,
  MDBIN         = 2,
  SFC           = 3,
  CONCATENATION = 4,
  USER_DEFINED_REORG  = 5,
  NONE_REORG    = 6, //increase it by one after adding new type
}SDS_Reorg_type;


typedef int SDS_Bool;
#define SDS_TRUE  1
#define SDS_FALSE 0

//Based on MPI definiation http://exodus.physics.ucla.edu/appleseed/dev/mpi.h

#ifdef SDS_CLIENT_MPI
#define SDS_QUERY_WORLD     MPI_COMM_WORLD //#define MPI_COMM_WORLD     0
#define SDS_QUERY_SELF      MPI_COMM_SELF  //#define MPI_COMM_SELF      1
#else
#define SDS_QUERY_WORLD     0 //#define MPI_COMM_WORLD     0
#define SDS_QUERY_SELF      1 //#define MPI_COMM_SELF      1
#endif

typedef int SDS_Query_comm;  //typedef int MPI_Comm;


//Combine operator for two querys.  
typedef enum SDS_Collection_operation_type{
    SDS_SELECT    = 0,
    SDS_PROJECT   = 1,
    SDS_JOIN      = 2,
    SDS_HISTGRAM  = 3,
    SDS_UDF       = 4,
    SDS_NONE_CO   = 5, //No operator of collection operation
}SDS_Collection_operation_type;


typedef enum SDS_Object_operation_type{
  SDS_GT    = 0,
  SDS_GE    = 1,
  SDS_LT    = 2,
  SDS_LE    = 3,
  SDS_EQ    = 4,
  SDS_NE    = 5,
  SDS_AND   = 6,
  SDS_OR    = 7,
  SDS_NONE_OO = 8,   //No operator of object operation
}SDS_Object_operation_type;
#define SDS_OP_MAX_COUNT  6 // 2(GT, GE) + 2(LT, LE) + 2(EQ, NE), used by SDS_Condition_tree in sds-condition-tree.h


typedef enum SDS_Tree_node_type{
  LEAF     =0,
  NONE_LEAF=1,
}SDS_Tree_node_type;

typedef union SDS_Value_union{
  int    i;
  float  f;
  double d;
}SDS_Value_union;

//The data types SDS currently supported 
typedef enum SDS_Data_type {
  SDS_INT         = 0,
  SDS_FLOAT       = 1,
  SDS_DOUBLE      = 2,
  SDS_UNKNOWN_TYPE = 3, //Increase this by one when add new before.
}SDS_Data_type;

//The file type which the objects resides
typedef enum SDS_File_type{
  SDS_HDF5   =0,
  SDS_BINARY =1,
  SDS_NETCDF =2,
  SDS_MEMORY =3,
  SDS_UNKNOWD_FILE_TYPE=4,
}SDS_File_type;


//The file type which the objects resides
typedef enum SDS_Pattern_type{
  SDS_PAPPTERN_HYPERSLAB  =0,
  SDS_PAPPTERN_ELEMENT =1,
  SDS_PAPPTERN_UNKNOWN=4,
}SDS_Pattern_type;


/* Behaves similarly to fprintf(stderr, ...), but adds file, line, and function
 information. */
#define error_out(...) {\
    fprintf(stderr, "%s:%d: %s():\t", __FILE__, __LINE__, __FUNCTION__);\
    fprintf(stderr, __VA_ARGS__);\
}


//It will be delted later 
enum reorganization_type{
  //SORT          = 0,
  //  TRANSFORM     = 1,
  INDEX         = 2,
  USER_DEFINED  = 3
};

// default MPI subarray size in parallel mode
#define FASTQUERY_DEFAULT_MPI_LEN 10000000
// default dimension rank to split data into subarrays in parallel mode
#define FASTQUERY_DEFAULT_MPI_DIM 0


//Does the dir path have a "/" as the last character
SDS_Bool dir_with_last_slash(char *dir);
//Does the file exist.
SDS_Bool file_exist(char *filename);
//User must allocate memory for this
//http://linux.die.net/man/3/basename
int      split_path(char  *path, char *dir, char *filename);
#endif
