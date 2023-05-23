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
#ifndef __SDS_QUERY_TREE_H__
#define __SDS_QUERY_TREE_H__

#include "sds-common.h"
#include "sds-object.h"
#include "sds-collection.h"


//SDS Query Tree a tree 
// Leaf node is Collection
// Non-leaf node is operator
typedef struct SDS_Query_tree{
  struct SDS_Query_tree             *left;              // NULL for leaf node
  struct SDS_Query_tree             *right;             // NULL for leaf node and non-join operator
  SDS_Tree_node_type                 type;              // 0: leaf node (collection);  1: non-leaf node (operator and operand)
  SDS_Collection_operation_type      operator;          // Type operator for left children
  void                              *operand;           // Operand for operator like condition string, or join predicate
  SDS_Collection                    *collection;        // Collection only existing at leaf node; NULL for non-leaf node
}SDS_Query_tree;

//Init a leaf node; Building a tree started with a simple leaf node
SDS_Query_tree    *SDS_Query_tree_init(SDS_Collection  *col);
//Free the memory
int                SDS_Query_tree_finalize(SDS_Query_tree      *root);
//Apply a operator onto the tree with the operator: project and filter
SDS_Query_tree    *SDS_Query_tree_apply(SDS_Query_tree *root,    SDS_Collection_operation_type     op_type,  void *operand);
//Combine two tree:  join
SDS_Query_tree    *SDS_Query_tree_combine(SDS_Query_tree *left,  SDS_Query_tree *right, SDS_Collection_operation_type  op_type,   void *operand);
//Serialize the tree into linear expression
void               SDS_Query_tree_serialize(SDS_Query_tree       *root,   SDS_Query_tree   **s_tree, int *n);
//De-Serialize the linear expression into tree: need test
SDS_Query_tree    *SDS_Query_tree_deserialize(SDS_Query_tree    **s_tree, int *n);

int                SDS_Query_tree_op_type(SDS_Query_tree      *root);


#endif

