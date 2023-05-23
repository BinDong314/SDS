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
 * This file defines the function to map the SDS structures 
 *  and the structures  defined in message.protoc 
 * This file is shared by SDS Client and SDS Server
 *
 * Author: Bin Dong <dbin at lbl.gov >
 * Copyright 2015 the Regents of the University of California
 * 
 */

#ifndef __MP_FILL_H__
#define __MP_FILL_H__

#include "sds-common.h"
#include "message.protoc.pb-c.h"
#include "sds-error.h"
#include "sds-object.h"
#include "sds-collection.h"
#include "sds-query-tree.h"
#include "sds-condition-tree.h" 


//Fill SdsObject (in ../common/messsage.protoc) with SDS_Object (in ../client/sds-object.h)
void query_fill_object(SdsObject *new_obj, SDS_Object    *old_obj);

//fill the cond_tree node defined in ../common/message
void  query_fill_cond_tree_node(CondTreeNode *new_node, SDS_Condition_tree    *old_node);
//flaten the cond_tree to a char * string
char *query_cond_tree_to_string(SDS_Condition_tree    *root);
//flaten the cond_tree to a char * string
SDS_Condition_tree    *query_cond_tree_to_string_reverse(char *buf);
//flaten project_index (defined in ../common/message) to char * string
char *project_index_to_string(int *selected_obj_indexs);
//flaten project_index (defined in ../common/message) to char * string
int *project_index_to_string_reverse(char *buf);
//flaten join_index (defined in ../common/message) to a char * string
char *join_index_to_string(int *index);
//flaten join_index (defined in ../common/message) to a char * string
int *join_index_to_string_reverse(char *buf);

//fill the query_tree_node (defined in ../common/message ) with SDS_Query_tree (defined in ../client/sds-query-tree.h)
void  query_fill_query_tree_node(QueryTreeNode *new_node, SDS_Query_tree *old_node);
//Fill SDS_Object (in ../client/sds-object.h) with SdsObject (in ../common/messsage) 
void query_fill_object_inverse(SDS_Object    *new_obj, SdsObject *old_obj);
//Fill SDS_Index_file (in ../client/sds-index-file.h) with SdsObject (in ../common/messsage) 
void query_fill_index(SDS_Index_file *new_index, SdsObject *old_obj);
//Fill  SdsObject (in ../common/messsage) with SDS_Index_file (in ../client/sds-index-file.h)
void query_fill_index_inverse(SDS_Index_file *new_index, SdsObject *old_obj);
//Fill SDS_Reorg_file (in ../client/sds-reorg-file.h) with SdsObject (in ../common/messsage) 
void query_fill_reorg(SDS_Reorg_file *new_reorg, SdsObject *old_obj);
//Fill SdsObject (in ../common/messsage)  with SDS_Reorg_file (in ../client/sds-reorg-file.h) 
void query_fill_reorg_inverse(SDS_Reorg_file *new_reorg, SdsObject *old_obj);

void query_fill_object_reverse(SDS_Object * new_obj, SdsObject  *old_node);

void query_fill_cond_tree_node_reverse(SDS_Condition_tree    *new_node, CondTreeNode    * node);

//Free the ClientRequest (defined in message.protoc)
void ClientRequest_free(ClientRequest *cr);

//Free the ClientObject (defined in message.protoc)
void SdsObject_free(SdsObject *object);

//Free the ClientQueryData (defined in message.protoc)
void RequestQueryData_free(RequestQueryData *query_data);

//Free the RequestReorgData (defined in message.protoc)
void RequestReorgData_free(RequestReorgData *reorg_data);

//Free the RequestAdminData (defined in message.protoc)
void RequestAdminData_free(RequestAdminData *admin_data);

//Free the RequestAnalyData (defined in message.protoc)
void RequestAnalyData_free(RequestAnalyData *analy_data);

//Free the QueryTreeNode (defined in message.protoc)
void QueryTreeNode_free(QueryTreeNode *query_tree_node);



#endif
