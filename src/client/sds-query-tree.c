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
#include "sds-query-tree.h"

//Init a leaf node; Building a tree started with a simple leaf node
SDS_Query_tree    *SDS_Query_tree_init(SDS_Collection  *col){
  SDS_Query_tree    *new_tree;
  new_tree = malloc(sizeof(SDS_Query_tree));

  new_tree->collection = col;
  new_tree->left       = NULL;
  new_tree->right      = NULL;
  new_tree->type       = LEAF;
  new_tree->operator   = SDS_NONE_CO;
  new_tree->operand    = NULL;
  
  return new_tree;
}

//Free the memory
int                SDS_Query_tree_finalize(SDS_Query_tree      *root){
  if(root == NULL)
    log_quit("Query tree in empty in SDS_Query_tree_finalize()!");
  
  if(root->left != NULL)
    SDS_Query_tree_finalize(root->left);

  if(root->right != NULL)
    SDS_Query_tree_finalize(root->right);

  if(root->type == LEAF)
    SDS_Collection_finalize(root->collection);

  if(root->type == NONE_LEAF){
    switch(root->operator){
      case SDS_SELECT:
        //SDS_Condition_tree_finalize(root->operand);
        break;
      case SDS_PROJECT:
        //SDS_Project_operand_finalize(root->operand);
        break;
      case SDS_JOIN:
        break;
      case SDS_UDF:
        break;
      default:
        log_quit("Undefined operator!");
        break;
    }
  }
  
  free(root);
}

//Apply a operator onto the tree with the operator: project and filter
SDS_Query_tree    *SDS_Query_tree_apply(SDS_Query_tree *root,    SDS_Collection_operation_type    op_type,  void *operand){
  if(root == NULL)
    log_quit("Query tree in empty SDS_Query_tree_apply() !");
  
  SDS_Query_tree    *new_tree;
  new_tree = SDS_Query_tree_init(NULL);
  new_tree->collection = NULL;
  new_tree->left       = root;
  new_tree->right      = NULL;
  new_tree->type       = NONE_LEAF;
  new_tree->operator   = op_type;
  new_tree->operand    = operand;

  return new_tree;
}

//Combine two tree:  join
SDS_Query_tree    *SDS_Query_tree_combine(SDS_Query_tree *left,  SDS_Query_tree *right, SDS_Collection_operation_type     op_type,   void *operand){
  SDS_Query_tree    *new_tree;
  new_tree = SDS_Query_tree_init(NULL);

  new_tree->collection = NULL;
  new_tree->left       = left;
  new_tree->right      = right;
  new_tree->type       = NONE_LEAF;
  new_tree->type       = op_type;
  new_tree->operand    = operand;
 
}


int                SDS_Query_tree_op_type(SDS_Query_tree      *root){
  printf("Node type %d, op type %d \n ", root->type, root->operator);
}


//Serialize the tree into linear expression
void              SDS_Query_tree_serialize(SDS_Query_tree    *root, SDS_Query_tree    **s_tree, int *n){
  if(root == NULL){
    s_tree[*n] =  NULL;
    *n = *n + 1;
    return;
  }
  
  s_tree[*n] = root;
  *n = *n + 1;
  SDS_Query_tree_serialize(root->left,  s_tree, n);
  SDS_Query_tree_serialize(root->right, s_tree, n);
}

//De-Serialize the linear expression into tree: need test
SDS_Query_tree  *SDS_Query_tree_deserialize(SDS_Query_tree    **s_tree, int *n){
  if(s_tree[*n] == NULL){
    *n = *n + 1;
    return NULL;
  }

  SDS_Query_tree  *root=malloc(sizeof(SDS_Query_tree));
  root = s_tree[*n] ;
  *n = *n + 1;
  root->left = SDS_Query_tree_deserialize(s_tree, n);
  root->right = SDS_Query_tree_deserialize(s_tree, n);
  return root;
  
}

void SDS_Query_tree_size(SDS_Query_tree  *tree, int *size){
  if(tree == NULL){
    *size = *size + 1;
    return;
  }
  *size = *size + 1;
  SDS_Query_tree_size(tree->left, size);
  SDS_Query_tree_size(tree->right, size);
}
