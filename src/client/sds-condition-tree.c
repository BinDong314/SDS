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
#include "sds-condition-tree.h"
//Init leaf node with an object index in the collection you will applied to
SDS_Condition_tree      *SDS_Condition_tree_init(int object_index){
  SDS_Condition_tree *new_tree;
  new_tree = malloc(sizeof(SDS_Condition_tree));
  
  new_tree->object_name  = NULL;
  new_tree->object_index = object_index;
  new_tree->left  = NULL;
  new_tree->right = NULL;
  new_tree->type  = LEAF;
  new_tree->operation = malloc(sizeof(SDS_Operation));
  new_tree->operation->op_n = 0;

  return new_tree;
}

int SDS_Condition_tree_finalize(SDS_Condition_tree      *root){
  if(root == NULL)
    return -1;
  
  SDS_Condition_tree_finalize(root->left);
  SDS_Condition_tree_finalize(root->right);
  
  if(root->operation != NULL)
    free(root->operation);
  
  free(root);
}




//Only apply to the leaf node (Error for non-leaf node)
void SDS_Condition_tree_apply(SDS_Condition_tree   *root,  SDS_Object_operation_type operator,  SDS_Value_union value){
  int                              i;
  //SDS_Operation                   *operation;

  if(root == NULL)
    log_quit("root is NULL in SDS_Condition_tree_apply()!");
  
  //operation = root->operation;
  //Todo: check value at the same time 
  //Todo: permit one operator have different values
  //Todo: merge and evaluate check at certain time
  //for (i = 0 ;  i< operation->op_n; i++){
  //  if(operation->op_type[i] == operator){
  //    operation->op_operand[i] = value;
  //    return;
  //  }
  //}
  
  switch(operator){
    case SDS_GT:
    case SDS_GE:
    case SDS_LT:
    case SDS_LE:
    case SDS_EQ:
    case SDS_NE:
      //if(root->op_n > SDS_OP_MAX_COUNT ): no need to check should be less than three
      root->operation->op_operand[root->operation->op_n] = value;
      root->operation->op_n = root->operation->op_n + 1;
      break;
    case SDS_AND:
    case SDS_OR:
      break;
      log_quit("SDS_AND and SDS_OR are only applied to non-leaf node! ");
    default:
      log_quit("Unknown operator in SDS_Condition_tree_apply()! ");
  }
  
}
//Merge two trees (or two leaf nodes)
SDS_Condition_tree      *SDS_Condition_tree_combine(SDS_Condition_tree *left,  SDS_Object_operation_type operator, SDS_Condition_tree *right){
  SDS_Condition_tree *new_tree = SDS_Condition_tree_init(0); //0 is useless 
  if(left == NULL || right == NULL)
    log_quit("Left or right are NULL in SDS_Condition_tree_combine()!");
  
  if((operator != SDS_AND) || (operator != SDS_OR) ){
    log_quit("operator should be SDS_AND, SDS_OR in SDS_Condition_tree_combine()!");
  }
  new_tree->type  = NONE_LEAF;
  new_tree->left  = left;
  new_tree->right = right;
  //new_tree->operation = malloc(sizeof(SDS_Operation));
  new_tree->operation->op_type[0] = operator;
  new_tree->operation->op_n = 1;
}


/// Folloging two functions:
///  -- First, use SDS_Condition_tree_object_covered to find the the index of objects covered in condition tree
///     (input covered is allocated based on the total number of objects in collection)
/// -- Then, use the number of covered objects to allocate memory for input of  SDS_Condition_tree_object_info
int  SDS_Condition_tree_object_covered(SDS_Condition_tree *root, SDS_Bool  *covered){
  if(root == NULL)
    return 0;
  SDS_Condition_tree_object_covered(root->left, covered);
  if(root->type == LEAF){
    covered[root->object_index] = SDS_TRUE;
  }
  SDS_Condition_tree_object_covered(root->right, covered);
  return 0;
}
int  SDS_Condition_tree_object_info(SDS_Condition_tree *root, int *object_index, SDS_Operation *object_operand, int *object_size, SDS_Object_operation_type *merge_operator, int *merge_size){
  if(root == NULL)
    return -1;
  int i;
  SDS_Condition_tree_object_info(root->left, object_index, object_operand, object_size, merge_operator, merge_size);
  if(root->type == LEAF){
    object_index[*object_size]              = root->object_index;
    object_operand[*object_size].op_n       = root->operation->op_n;
    for (i = 0 ; i < object_operand[*object_size].op_n; i++){
      object_operand[*object_size].op_type[i]    = root->operation->op_type[i];
      object_operand[*object_size].op_operand[i] = root->operation->op_operand[i];
    }
    *object_size = *object_size + 1;
  }
  
  if(root->type == NONE_LEAF){
    merge_operator[*merge_size] = root->operation->op_type[0];
    *merge_size = *merge_size + 1;
  }
  
  SDS_Condition_tree_object_info(root->right, object_index, object_operand, object_size, merge_operator, merge_size);
}

SDS_Bool SDS_Value_compare(SDS_Value_union data, SDS_Data_type data_type, SDS_Operation *object_operand){
  SDS_Bool ret = SDS_FALSE;
  switch(data_type){
    case SDS_INT:
      ret = SDS_Int_compare(data.i,   object_operand);
      break;
    case SDS_FLOAT:
      ret = SDS_Float_compare(data.f, object_operand);
      break;
    case SDS_DOUBLE:
      ret = SDS_Double_compare(data.d, object_operand);
      break;
    default:     
      log_quit("Unknown data type in SDS_Value_compare(); !");
      break;
  }
  
  
  return ret;
 
}


SDS_Bool  SDS_Int_compare(int data, SDS_Operation *object_operand){
  SDS_Bool                      ret = SDS_TRUE, c_ret;
  int                           compare_value;
  SDS_Object_operation_type     op_type;
  int i;
  for(i = 0; i < object_operand->op_n; i++){
    op_type = object_operand->op_type[i];
    compare_value = object_operand->op_operand[i].d;
    switch(op_type){
      case SDS_GT:
        if(data > compare_value){
          c_ret = SDS_TRUE;
        }else{
          c_ret = SDS_FALSE;
        }
        break;
      case SDS_GE:
        if(data >= compare_value){
          c_ret = SDS_TRUE;
        }else{
          c_ret = SDS_FALSE;
        }
        break;
      case SDS_LT:
        if(data < compare_value){
          c_ret = SDS_TRUE;
        }else{
          c_ret = SDS_FALSE;
        }
        break;
      case SDS_LE:
        if(data <= compare_value){
          c_ret = SDS_TRUE;
        }else{
          c_ret = SDS_FALSE;
        }
        break;
        break;
      case SDS_EQ:
        if(data = compare_value){
          c_ret = SDS_TRUE;
        }else{
          c_ret = SDS_FALSE;
        }
        break;
      case SDS_NE:
        if(data != compare_value){
          c_ret = SDS_TRUE;
        }else{
          c_ret = SDS_FALSE;
        }
        break;
      default:
        log_quit("Unknown operator in SDS_Int_compare()! ");
        break;
    }

    if((ret == SDS_TRUE) && (c_ret == SDS_TRUE)){
      ret = SDS_TRUE;
    }else{
      ret = SDS_FALSE;
      break;
    }
  }
  return ret;
}


SDS_Bool  SDS_Float_compare(float data, SDS_Operation *object_operand){
  SDS_Bool                      ret = SDS_TRUE, c_ret;
  float                         compare_value;
  SDS_Object_operation_type     op_type;       
  int i;
  for(i = 0; i < object_operand->op_n; i++){
    op_type = object_operand->op_type[i];
    compare_value = object_operand->op_operand[i].d;
    switch(op_type){
      case SDS_GT:
        if(data > compare_value){
          c_ret = SDS_TRUE;
        }else{
          c_ret = SDS_FALSE;
        }
        //log_msg("%f %f, result %d", data, compare_value, c_ret);
        break;
      case SDS_GE:
        if(data >= compare_value){
          c_ret = SDS_TRUE;
        }else{
          c_ret = SDS_FALSE;
        }
        break;
      case SDS_LT:
        if(data < compare_value){
          c_ret = SDS_TRUE;
        }else{
          c_ret = SDS_FALSE;
        }
        break;
      case SDS_LE:
        if(data <= compare_value){
          c_ret = SDS_TRUE;
        }else{
          c_ret = SDS_FALSE;
        }
        break;
      case SDS_EQ:
        if(data = compare_value){
          c_ret = SDS_TRUE;
        }else{
          c_ret = SDS_FALSE;
        }
        break;
      case SDS_NE:
        if(data != compare_value){
          c_ret = SDS_TRUE;
        }else{
          c_ret = SDS_FALSE;
        }
        break;
      default:
        log_quit("Unknown operator in SDS_Float_compare()! ");
        break;
    }
    if((ret == SDS_TRUE) && (c_ret == SDS_TRUE)){
      ret = SDS_TRUE;
    }else{
      ret = SDS_FALSE;
      break;
    }
  }
  return ret;
}



SDS_Bool  SDS_Double_compare(double data, SDS_Operation *object_operand){
  SDS_Bool                      ret = SDS_TRUE, c_ret;
  double                        compare_value;
  SDS_Object_operation_type     op_type;
  int i;
  for(i = 0; i < object_operand->op_n; i++){
    op_type = object_operand->op_type[i];
    compare_value = object_operand->op_operand[i].d;
    switch(op_type){
      case SDS_GT:
        //printf("%f > %f\n", data, compare_value);
        if(data > compare_value){
          c_ret = SDS_TRUE;
        }else{
          c_ret = SDS_FALSE;
        }
        break;
      case SDS_GE:
        if(data >= compare_value){
          c_ret = SDS_TRUE;
        }else{
          c_ret = SDS_FALSE;
        }
        break;
      case SDS_LT:
        if(data < compare_value){
          c_ret = SDS_TRUE;
        }else{
          c_ret = SDS_FALSE;
        }
        break;
      case SDS_LE:
        if(data <= compare_value){
          c_ret = SDS_TRUE;
        }else{
          c_ret = SDS_FALSE;
        }
        break;
        break;
      case SDS_EQ:
        if(data = compare_value){
          c_ret = SDS_TRUE;
        }else{
          c_ret = SDS_FALSE;
        }
        break;
      case SDS_NE:
        if(data != compare_value){
          c_ret = SDS_TRUE;
        }else{
          c_ret = SDS_FALSE;
        }
        break;
      default:
        log_quit("Unknown operator in SDS_Double_compare()! ");
        break;
    }
    
    if(object_operand->op_n == 1)
      return c_ret;
    
    if(ret && c_ret){
      ret = SDS_TRUE;
    }
  }
  return ret;
}


void  SDS_Condition_tree_n_node(SDS_Condition_tree    *root, int *n_node){
  if(root == NULL){
    *n_node = *n_node + 1;
    return;
  }
  printf("%d ", *n_node);
  *n_node = *n_node + 1;
  SDS_Condition_tree_n_node(root->left, n_node);
  SDS_Condition_tree_n_node(root->right, n_node);
}

//Serilize the conditional tree into s_tree (an array) for communication
void SDS_Condition_tree_serialize(SDS_Condition_tree    *root, SDS_Condition_tree    **s_tree, int *n){
  
  if(root == NULL){
    s_tree[*n] =  NULL;
    *n = *n + 1;
    return;
  }
  
  s_tree[*n] = root;
  *n = *n + 1;
  SDS_Condition_tree_serialize(root->left,  s_tree, n);
  SDS_Condition_tree_serialize(root->right, s_tree, n);
}

//De-serilize the conditional tree from s_tree (an array) after communication
SDS_Condition_tree     *SDS_Condition_tree_deserialize(SDS_Condition_tree    **s_tree, int *n){
  if(s_tree[*n] == NULL){
    *n = *n + 1;
    return NULL;
  }

  SDS_Condition_tree  *root=malloc(sizeof(SDS_Condition_tree));
  root = s_tree[*n] ;
  *n = *n + 1;
  root->left  = SDS_Condition_tree_deserialize(s_tree, n);
  root->right = SDS_Condition_tree_deserialize(s_tree, n);

  return root;
}



/*
 * The following functions are used by sds-parser to build condition tree 
 *
 */

//Allocate mem space for a tree node
static SDS_Condition_tree *allocatenode()
{
    SDS_Condition_tree *root = (SDS_Condition_tree *)malloc(sizeof(SDS_Condition_tree));
 
    if (root == NULL)
        return NULL;
 
    root->left = NULL;
    root->right = NULL;
    root->operation = NULL;
    return root;
}

//Create a leaf node (sds-object) with a two side range conditional string
SDS_Condition_tree *create_leaf_between(char *name, double lv, int lt, double rv, int rt){
  SDS_Condition_tree *root = allocatenode();
  root->type= LEAF;
  root->object_name=strdup(name);
  root->operation= (SDS_Operation *)malloc(sizeof(SDS_Operation));
  root->operation->op_n = 2;
  root->operation->op_type[0] = (SDS_Object_operation_type)lt;
  root->operation->op_type[1] = (SDS_Object_operation_type)rt;
  root->operation->op_operand[0].d = lv;
  root->operation->op_operand[1].d = rv;
  root->left = NULL;
  root->right = NULL;

  return root;

}

//Create a leaf node (sds-object) with a one side range conditional string
SDS_Condition_tree *create_leaf_oneside(char *name, double v, int t){
  //printf("name %s , type %d, value %f \n", name, t, v);
  SDS_Condition_tree *root = allocatenode();
  root->type= LEAF;
  root->object_name=strdup(name);
  root->operation=(SDS_Operation *)malloc(sizeof(SDS_Operation));
  root->operation->op_n = 1;
  root->operation->op_type[0] = (SDS_Object_operation_type)t;
  root->operation->op_operand[0].d = v;
  root->left = NULL;
  root->right = NULL;
  return root;
}

//Create a none leaf node (sds operation) defined in ../common/sds-common.h
SDS_Condition_tree *create_nonleaf(int type, SDS_Condition_tree *left, SDS_Condition_tree *right){
  SDS_Condition_tree *root = allocatenode();
  root->type= NONE_LEAF;
  root->left=left;
  root->right=right;
  root->operation = (SDS_Operation *)malloc(sizeof(SDS_Operation));
  root->operation->op_n = 1;
  root->operation->op_type[0] = (SDS_Object_operation_type)type;
  return root;

}

//free each node
void  free_node(SDS_Condition_tree *node){
  if(node->type == LEAF){
    if(node->object_name != NULL)
      free(node->object_name);
  }

  if(node->operation != NULL)
    free(node->operation);
  free(node);
}

//free the whole conditional tree
void delete_SDS_Condition_tree(SDS_Condition_tree *root){
  if(root == NULL)
    return;
  delete_SDS_Condition_tree(root->left);
  delete_SDS_Condition_tree(root->right);
  
  free_node(root);

}


