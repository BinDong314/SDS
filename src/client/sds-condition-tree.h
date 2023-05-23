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
#ifndef __SDS_CONDITION_TREE_H__
#define __SDS_CONDITION_TREE_H__

#include "sds-common.h"

///op_type :    < , >, >=, <=, AND, OR
///op_n    :   the size of op_type and op_operand
///op_operand: value for corresponding type in op_type 
typedef struct SDS_Operation{
  SDS_Object_operation_type     op_type[SDS_OP_MAX_COUNT];          // operator type: <, >, =, AND, OR.
  int                           op_n;                              // Number of operator 
  SDS_Value_union               op_operand[SDS_OP_MAX_COUNT];      // As comments at the ahead. 
}SDS_Operation;

///SDS Condition Tree
///
///Todo: make it more reasonable
typedef struct SDS_Condition_tree{
  struct SDS_Condition_tree       *left;              // NULL for leaf node
  struct SDS_Condition_tree       *right;             // 
  SDS_Tree_node_type               type;              // 0: leaf node (sds object);  1: non-leaf node (operator and operand)
  char                            *object_name;       //The name of SDS Object. Right now, it is dataset (HDF5) name.
                                                      // ["object_name"."object_index"] is used as variable name in conditional string. 
  int                              object_index;      //Index of SDS Object in Collection (1, 2 , 3, ....)
  SDS_Operation                   *operation;           
}SDS_Condition_tree;

//Init leaf node with an object index in the collection you will applied to
SDS_Condition_tree      *SDS_Condition_tree_init(int object_index);
//Only apply to the leaf node (Error for non-leaf node)
void                     SDS_Condition_tree_apply(SDS_Condition_tree   *root,  SDS_Object_operation_type operator,  SDS_Value_union value);
//Merge two trees (or two leaf nodes)
SDS_Condition_tree      *SDS_Condition_tree_combine(SDS_Condition_tree *left,  SDS_Object_operation_type operator, SDS_Condition_tree *right);

//Finalize the query tree
int                      SDS_Condition_tree_finalize(SDS_Condition_tree      *root);

//Serilize the conditional tree into s_tree (an array) for communication
void                     SDS_Condition_tree_serialize(SDS_Condition_tree    *root, SDS_Condition_tree    **s_tree, int *n);

//De-serilize the conditional tree from s_tree (an array) after communication
SDS_Condition_tree      *SDS_Condition_tree_deserialize(SDS_Condition_tree    **s_tree, int *n);

int                      SDS_Condition_tree_object_covered(SDS_Condition_tree *root, SDS_Bool  *covered);

int                      SDS_Condition_tree_object_info(SDS_Condition_tree *root, int *object_index, SDS_Operation *object_operand, int *object_size, SDS_Object_operation_type *merge_operator, int *merge_size);
SDS_Bool SDS_Value_compare(SDS_Value_union data, SDS_Data_type data_type, SDS_Operation *object_operand);
SDS_Bool  SDS_Float_compare(float data, SDS_Operation *object_operand);
SDS_Bool  SDS_Double_compare(double data, SDS_Operation *object_operand);
SDS_Bool  SDS_Int_compare(int data, SDS_Operation *object_operand);


/*
 * The following functions are used by sds-parser to build condition tree 
 *
 */
//Allocate mem space for a tree node
static SDS_Condition_tree *allocatenode();

//Create a leaf node (sds-object) with a two side range conditional string
SDS_Condition_tree *create_leaf_between(char *name, double lv, int lt, double rv, int rt);

//Create a leaf node (sds-object) with a one side range conditional string
SDS_Condition_tree *create_leaf_oneside(char *name, double v, int t);


//Create a none leaf node (sds operation) defined in ../common/sds-common.h
SDS_Condition_tree *create_nonleaf(int type, SDS_Condition_tree *left, SDS_Condition_tree *right);

//free each node
void  free_node(SDS_Condition_tree *node);

//free the whole conditional tree
void delete_SDS_Condition_tree(SDS_Condition_tree *root);


#endif
