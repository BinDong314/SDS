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
 * This file defines the SDS Query interface to init/run/finalize a query tree
 * Author: Bin Dong <dbin at lbl.gov >
 * Copyright 2015 the Regents of the University of California
 * 
 */

#ifndef __SDS_QUERY_H__
#define __SDS_QUERY_H__

//#include "message.protoc.pb-c.h" 
#include "sds-common.h"
#include "sds-error.h"
#include "sds-object.h"
#include "sds-collection.h"
#include "sds-query-tree.h"
#include "sds-condition-tree.h" 

//
typedef struct SDS_Query_handle{
  SDS_Query_tree         *root;
  SDS_Collection         *result;
  SDS_Query_comm          comm; //For portability on single node, it is defiend as int (not using MPI_Comm)
}SDS_Query_handle;

//Create a query_handle based on a pre-built query tree 
//The comm is like mpi comm:  local and global 
SDS_Query_handle   *SDS_Query_init(SDS_Query_tree   *root, SDS_Query_comm comm);

//Run a run at client side
int                 SDS_Query_run(SDS_Query_handle             *handle);

//This function is called by SDS_Query_run to evaluate the tree 
//from bottom-up (post) order.
SDS_Collection     *SDS_Query_run_evaluate_tree(SDS_Query_tree *root, SDS_Query_comm comm);

//Run the query_tree on a remote server (SDS Server)
//Rightnow, it supports a single node
//To do: run the query_tree on a remmote cluster.
//       and let SDS Server to start a batch job to run it  
int                 SDS_Query_rrun(SDS_Query_handle   *handle);

//Get the result collection 
SDS_Collection     *SDS_Query_get_result(SDS_Query_handle     *handle);

//Release the query handle 
int                 SDS_Query_finalize(SDS_Query_handle       *handle);

#endif
