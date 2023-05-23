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
#include "sds-parser.h"
#include "sds-cond-parser.h"
#include "sds-cond-lexer.h"

//Build the conditional tree from a sting
SDS_Condition_tree *parse_condition_tree(const char *expr, SDS_Condition_tree **root){
  //SDS_Condition_tree *root;
  yyscan_t scanner;
  YY_BUFFER_STATE state;
  
  if (yylex_init(&scanner)) {
    printf("Error happening !\n");
    // couldn't initialize
    return NULL;
  }
    
  state = yy_scan_string(expr, scanner);
  
  if(yyparse(&root, scanner)) {
    // error parsing
    printf("Error here !");
    return NULL;
  }
  
  yy_delete_buffer(state, scanner);
  yylex_destroy(scanner);
  
  //printf("Parser work is done type [%d] , name %s !\n", root->type, root->object_name);
  //print_condition_tree(*root);
  //return root;
}

//For test
int print_condition_tree(SDS_Condition_tree *root){
  //printf("I am in print !\n");
  if(root != NULL){
    printf(" Node TYPE:  %d \n", root->type);
    switch (root->type) {
      case LEAF:
        printf(" VAR NAME:  %s (%f, %f) \n", root->object_name, root->operation->op_operand[0].d, root->operation->op_operand[1].d);
        break;
      case NONE_LEAF:
        printf(" OP TYPE:  %d \n", root->operation->op_type[0]);
        if(root->left != NULL)
          print_condition_tree(root->left);
        if(root->right != NULL)
          print_condition_tree(root->right);
        break;
      default:
        // shouldn't be here
        break;
    }
  }else{
    log_quit("The condition tree is NULL !\n");
  }
  return 0;
}
