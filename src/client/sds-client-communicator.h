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
 * This file defines the communication interface from client to server
 * Author: Bin Dong <dbin at lbl.gov >
 * Copyright 2015 the Regents of the University of California
 * 
 */

#ifndef __SDS_CLIENT_COMMUNICATOR_H__
#define __SDS_CLIENT_COMMUNICATOR_H__

#include "sds-common.h"
#include "sds-error.h"
#include "sds-object.h"
#include "sds-collection.h"
#include "sds-query-tree.h"
#include "sds-condition-tree.h" 

#include "message.protoc.pb-c.h"
#include "mp-fill.h"


//Start a socket with SERVER_ADDRESS and SERVER_PORT defined in ../common/sds-common.h
int SDS_Socket_start();
//Receive data from socket
int SDS_Socket_recv(int sockfd, void *buf, int length);
//Send data to socket
int SDS_Socket_sent(int sockfd, void *buf, int length);
//Stop the socket
int SDS_Socket_stop(int sockfd);

//Start a analysis job on server for the pre-built query_tree
SDS_Collection *SDS_Remote_analyze(SDS_Query_tree *query_tree);

//Read metadata for the collection from SDS Server with one communication 
int  SDS_read_collection_metadata(SDS_Object    **obj_array, int size);
//Start reorganization jobs on objections in a collection
int  SDS_start_collection_reorg(SDS_Object      **obj_array, int *index_type, int *reorg_type, int *cores, int *time, char **parameters, int size, int *reorg_status);
//Start the trace log analysis at server
int SDS_start_trace_analysis(SDS_Object    *obj, int mpi_rank, char *dir_name, char *app_name, int dim_rank);
#endif

