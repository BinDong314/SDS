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
 *
 * Author: Bin Dong <dbin at lbl.gov >
 * Copyright 2015 the Regents of the University of California
 * 
 */

#include "mp-fill.h"

//Fill SdsObject (in ../common/messsage.protoc) with SDS_Object (in ../client/sds-object.h)
void query_fill_object(SdsObject *new_obj, SDS_Object    *old_obj){
  if(old_obj->filename != NULL)
    new_obj->filename  = strdup(old_obj->filename);
  if(old_obj->group != NULL)
    new_obj->group = strdup(old_obj->group);
  if(old_obj->dsetname != NULL)
    new_obj->dsetname = strdup(old_obj->dsetname);   
  new_obj->has_file_type  = 1 ;
  new_obj->has_data_type  = 1 ;
  new_obj->file_type = old_obj->file_type;
  new_obj->data_type = old_obj->data_type;
}

//Fill SDS_Object (in ../client/sds-object.h) with SdsObject (in ../common/messsage) 
void query_fill_object_inverse(SDS_Object    *new_obj, SdsObject *old_obj){
  if(old_obj->filename != NULL)
    new_obj->filename  = strdup(old_obj->filename);
  if(old_obj->group != NULL)
    new_obj->group = strdup(old_obj->group);
  if(old_obj->dsetname != NULL)
    new_obj->dsetname = strdup(old_obj->dsetname);   
  if(old_obj->has_file_type == 1){
    new_obj->file_type = old_obj->file_type;
  }else{
    log_quit("Response back from server should have file type in query_fill_object_inverse()");
  }

  if(old_obj->has_data_type  == 1){
    new_obj->data_type = old_obj->data_type;
  }else{
    log_quit("Response back from server should have data type in query_fill_object_inverse()");
  }

}

//Fill SDS_Index_file (in ../client/sds-index-file.h) with SdsObject (in ../common/messsage) 
void query_fill_index(SDS_Index_file *new_index, SdsObject *old_obj){
  if(old_obj->filename != NULL)
    new_index->filename  = strdup(old_obj->filename);
  if(old_obj->group != NULL)
    new_index->group = strdup(old_obj->group);
  if(old_obj->dsetname != NULL)
    new_index->dsetname = strdup(old_obj->dsetname);   
  new_index->file_type = old_obj->file_type;
  new_index->data_type = old_obj->data_type;
}

//Fill SDS_Reorg_file (in ../client/sds-reorg-file.h) with SdsObject (in ../common/messsage) 
void query_fill_reorg(SDS_Reorg_file *new_reorg, SdsObject *old_obj){
  if(old_obj->filename != NULL)
    new_reorg->filename  = strdup(old_obj->filename);
  if(old_obj->group != NULL)
    new_reorg->group = strdup(old_obj->group);
  if(old_obj->dsetname != NULL)
    new_reorg->dsetname = strdup(old_obj->dsetname);   
  new_reorg->file_type = old_obj->file_type;
  new_reorg->data_type = old_obj->data_type;
}

//fill the cond_tree node defined in ../common/message
void  query_fill_cond_tree_node(CondTreeNode *new_node, SDS_Condition_tree    *old_node){
  new_node->type = old_node->type;
  if(old_node->type == LEAF){
    new_node->has_object_index = 1;
    new_node->object_index = old_node->object_index;
  }else{
    new_node->has_object_index = 0;
  }
  
  OperationM *op;
  op = malloc(sizeof(OperationM));
  operation_m__init(op);
  if(old_node->operation !=  NULL){
    op->op_n = old_node->operation->op_n;
    op->n_op_type    = op->op_n;
    op->n_op_operand = op->op_n;
    op->op_type     = malloc(sizeof(int)*op->op_n);
    op->op_operand  = malloc(sizeof(int)*op->op_n);
  }
  new_node->operation=op;
}


//fill the query_tree_node (defined in ../common/message ) with
// SDS_Query_tree (defined in ../client/sds-query-tree.h)
void   query_fill_query_tree_node(QueryTreeNode *new_node, SDS_Query_tree *old_node){
  int i, n_obj;
  SdsObject *new_obj;
  new_node->type = old_node->type;
  if(old_node->type == LEAF){
    n_obj = old_node->collection->object_count;
    new_node->n_collection = n_obj;
    new_node->collection  = malloc(sizeof(SdsObject *) * n_obj);
    for(i = 0 ; i < n_obj; i++){
      new_obj = malloc(sizeof(SdsObject));
      query_fill_object(new_obj,  old_node->collection->object_array[i]);
      new_node->collection[i] = new_obj;
    }
    new_node->has_operator_ = 0;
    new_node->operand = NULL;
  }else{
    new_node->has_operator_ = 1;
    new_node->operator_ = old_node->operator;
    
    void                  *operand_buf;
    switch(old_node->operator){
      case SDS_SELECT:
        operand_buf = query_cond_tree_to_string(old_node->operand);
        new_node->operand = operand_buf;
        break;
      case SDS_PROJECT:
        operand_buf = project_index_to_string(old_node->operand);
        new_node->operand = operand_buf;
        break;
      case SDS_JOIN:
        operand_buf = join_index_to_string(old_node->operand);
        new_node->operand = operand_buf;
        break;
      case SDS_UDF:
        
        break;
      other:
        log_quit("Unkown operator in query_fill_tree_node");
        break;
    }
  }
}

//fill the query_tree_node (defined in ../common/message ) with
// SDS_Query_tree (defined in ../client/sds-query-tree.h)
void   query_fill_query_tree_node_reverse(SDS_Query_tree *new_node, QueryTreeNode *old_node){
  int i, n_obj;
  SdsObject  *old_obj;
  SDS_Object *new_obj;
  new_node->type = old_node->type;
  if(old_node->type == LEAF){
    n_obj                = old_node->n_collection;
    new_node->collection = SDS_Collection_init(n_obj); 
    for(i = 0 ; i < n_obj; i++){
      new_obj = malloc(sizeof(SDS_Object));
      query_fill_object_reverse(new_obj,  old_node->collection[i]);
      SDS_Collection_append(new_node->collection, new_obj);
    }
  }else{
    new_node->operator = old_node->operator_;
    void                  *operand_buf;
    switch(old_node->operator_){
      case SDS_SELECT:
        operand_buf = query_cond_tree_to_string_reverse(old_node->operand);
        new_node->operand = operand_buf;
        break;
      case SDS_PROJECT:
        operand_buf = project_index_to_string_reverse(old_node->operand);
        new_node->operand = operand_buf;
        break;
      case SDS_JOIN:
        operand_buf = join_index_to_string_reverse(old_node->operand);
        new_node->operand = operand_buf;
        break;
      case SDS_UDF:
        log_msg("Un-supported  UDF by in query_fill_tree_node !");
        break;
      other:
        log_quit("Unkown operator in query_fill_tree_node");
        break;
    }
  }
}

void query_fill_object_reverse(SDS_Object * new_obj, SdsObject  *old_node){

}


//flaten the cond_tree to a char * string
char *query_cond_tree_to_string(SDS_Condition_tree    *root){
  int i, n = 0;
  SDS_Condition_tree    **s_tree;
  char *tree_buf;
  int   tree_buf_size;

  SDS_Condition_tree_n_node(root, &n);
  s_tree = malloc(sizeof(SDS_Condition_tree *) * n);
  SDS_Condition_tree_serialize(root, s_tree, &n);

  CondTree cond_tree=COND_TREE__INIT;
  cond_tree.n_nodes  = n;
  
  CondTreeNode *new_node;
  for(i = 0 ; i < n; i++){
    new_node =  malloc(sizeof(CondTreeNode));
    query_fill_cond_tree_node(new_node, s_tree[i]);
    cond_tree.nodes[i] = new_node;
  }
  
  tree_buf_size = cond_tree__get_packed_size(&cond_tree);
  tree_buf = malloc(tree_buf_size*sizeof(char));
  cond_tree__pack(&cond_tree, tree_buf);

  for(i = 0 ; i < n; i++){
    free(cond_tree.nodes[i]);
  }  
  
  return tree_buf;
  
}

//flaten the cond_tree to a char * string
SDS_Condition_tree    *query_cond_tree_to_string_reverse(char *buf){
  SDS_Condition_tree    *root;
  SDS_Condition_tree    **s_tree;
  int                   i, n, buf_size;
  //char *tree_buf;
  //int   tree_buf_size
  CondTree             *cond_tree;

  buf_size = sizeof(buf)/sizeof(buf[0]);
  cond_tree = cond_tree__unpack(NULL, buf_size, buf);
  n = cond_tree->n_nodes;
  s_tree = malloc(n * sizeof(SDS_Condition_tree *));
  SDS_Condition_tree    *new_node;
  for (i = 0 ; i < n; i++){
    new_node = malloc(sizeof(SDS_Condition_tree));
    query_fill_cond_tree_node_reverse(new_node, cond_tree->nodes[i]);
    s_tree[i] = new_node;
  }

  n = 0;
  root = SDS_Condition_tree_deserialize(s_tree, &n);

  return root;

}

void query_fill_cond_tree_node_reverse(SDS_Condition_tree    *new_node, CondTreeNode    * node){
  
}

//flaten project_index (defined in ../common/message) to char * string
char *project_index_to_string(int *selected_obj_indexs){
  ProjectIndex indexs = PROJECT_INDEX__INIT;
  int  i, n;
  n = sizeof(selected_obj_indexs)/sizeof(selected_obj_indexs[0]);
  indexs.n_selected_obj_indexs =  n;
  indexs.selected_obj_indexs = malloc(sizeof(int) * n);
  for(i = 0; i < n ; i++){
    indexs.selected_obj_indexs[i] = selected_obj_indexs[i];
  } 

  char *buf;
  int   buf_size;
  buf_size = project_index__get_packed_size(&indexs);
  buf = malloc(buf_size * sizeof(char));
  project_index__pack(&indexs, buf);

  free(indexs.selected_obj_indexs);

  return buf;
}


//flaten project_index (defined in ../common/message) to char * string
int *project_index_to_string_reverse(char *buf){
  ProjectIndex *index;
  int  i, buf_size, *selected_indexs, selected_indexs_n;
  buf_size = sizeof(buf)/sizeof(buf[0]); 
  
  index             = project_index__unpack(NULL, buf_size, buf);
  selected_indexs_n = index->n_selected_obj_indexs; 
  selected_indexs   = malloc(sizeof(int) * selected_indexs_n);

  for(i = 0; i < selected_indexs_n; i++ ){
    selected_indexs[i] = index->selected_obj_indexs[i];
  }

  return selected_indexs;
}


//flaten join_index (defined in ../common/message) to a char * string
char *join_index_to_string(int *index){
  JoinIndex joinindex = JOIN_INDEX__INIT;
  joinindex.key_obj_index1 = index[0];
  joinindex.key_obj_index2 = index[1];

  char *buf;
  int   buf_size;
  buf_size = join_index__get_packed_size(&joinindex);
  buf = malloc(buf_size * sizeof(char));
  join_index__pack(&joinindex, buf);

  return buf;
}

//flaten join_index (defined in ../common/message) to a char * string
int *join_index_to_string_reverse(char *buf){
  int       *index;
  int        buf_size;
  JoinIndex *join_index;

  buf_size = sizeof(buf_size) / sizeof(buf[0]);
  
  join_index = join_index__unpack(NULL, buf_size, buf);
  
  index =  malloc(2 * sizeof(int));
  
  index[0] = join_index->key_obj_index1;
  index[1] = join_index->key_obj_index2;

  return index;
}

//Free the ClientRequest (defined in message.protoc)
void ClientRequest_free(ClientRequest *cr){
  if(cr != NULL){
    if(cr->query_data != NULL){
      RequestQueryData_free(cr->query_data);
      free(cr->query_data);
    }

    if(cr->reorg_data != NULL){
      RequestReorgData_free(cr->reorg_data);
      free(cr->reorg_data);
    }
    
    if(cr->admin_data != NULL){
      RequestAdminData_free(cr->admin_data);
      free(cr->admin_data);
    }
    
    if(cr->analy_data != NULL){
      RequestAnalyData_free(cr->analy_data);
      free(cr->analy_data);
    }
  }
}

//Free the ClientObject (defined in message.protoc)
void SdsObject_free(SdsObject *object){
  if(object->filename != NULL){
    free(object->filename);
  }
  
  if(object->group != NULL){
    free(object->group);
  }

  if(object->dsetname != NULL){
    free(object->dsetname);
  }
}

//Free the ClientQueryData (defined in message.protoc)
void RequestQueryData_free(RequestQueryData *query_data){
  int i;
  if(query_data->objects != NULL){
    for(i = 0 ;  i < query_data->n_objects; i++){
      if(query_data->objects[i] != NULL)
        SdsObject_free(query_data->objects[i]);
    }
    free(query_data->objects);
  }
  
  //query_data->query_tree is not used 
}

//Free the RequestReorgData (defined in message.protoc)
void RequestReorgData_free(RequestReorgData *reorg_data){
  int i;
  if(reorg_data->objects != NULL){
    for (i = 0 ; i < reorg_data->n_objects; i++){
      SdsObject_free(reorg_data->objects[i]);
    }
    free(reorg_data->objects);
  }

  //if(reorg_data->index_type != NULL){
  //  free(reorg_data->index_type);
  //}

  //if(reorg_data->reorg_type != NULL){
  //  free(reorg_data->reorg_type);
  // }

  //if(reorg_data->index_cores != NULL){
  //  free(reorg_data->index_cores);
  // }

  //if(reorg_data->reorg_cores != NULL){
  //  free(reorg_data->reorg_cores);
  //}

  //if(reorg_data->reorg_time_secs != NULL){
  //  free(reorg_data->reorg_time_secs);
  //}

  //if(reorg_data->index_parameters != NULL){
  // for(i = 0; reorg_data->n_index_parameters; i++ )
  //    if(reorg_data->index_parameters[i] != NULL)
  //      free(reorg_data->index_parameters);
  //  free(reorg_data->index_parameters);
  //}

  //if(reorg_data->reorg_parameters != NULL){
  //  for(i = 0; reorg_data->n_reorg_parameters; i++ )
  //    if(reorg_data->reorg_parameters[i] != NULL)
  //      free(reorg_data->reorg_parameters);
  //  free(reorg_data->reorg_parameters);
  //}
}

//Free the RequestAdminData (defined in message.protoc)
void RequestAdminData_free(RequestAdminData *admin_data){
  if(admin_data->object != NULL)
    SdsObject_free(admin_data->object);
}

//Free the RequestAnalyData (defined in message.protoc)
void RequestAnalyData_free(RequestAnalyData *analy_data){
  int i;
  if(analy_data->query_tree != NULL){
    for(i = 0; i < analy_data->n_query_tree; i++){
      QueryTreeNode_free(analy_data->query_tree[i]);
    }
    free(analy_data->query_tree);
  }
}

//Free the QueryTreeNode (defined in message.protoc)
void QueryTreeNode_free(QueryTreeNode *query_tree_node){
  int i;
  if(query_tree_node->type == LEAF){
    if(query_tree_node->collection != NULL){
      for(i = 0; i < query_tree_node->n_collection; i++ ){
        SdsObject_free(query_tree_node->collection[i]);
      }
      free(query_tree_node->collection);
    }
  }else{
    if(query_tree_node->operand != NULL)
      free(query_tree_node->operand);
  }
}
