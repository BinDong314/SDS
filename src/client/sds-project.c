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
#include "sds-project.h"

SDS_Project_operand *SDS_Project_operand_init(int *object_index, int length){
  SDS_Project_operand *new_op;
  int i;
  new_op = malloc(sizeof(SDS_Project_operand));
  new_op->object_index = malloc(length*sizeof(int));
  for(i = 0; i < length; i++){
    new_op->object_index[i] = object_index[i];
  }
  return new_op;
}
int SDS_Project_operand_finalize(SDS_Project_operand *p){
  if( p != NULL){
    if(p->object_index != NULL)
      free(p->object_index);
    free(p);
  }
}

//Serialize to "int array" : first one is length, left are index array
void                *SDS_Project_operand_serilize(SDS_Project_operand *project_operand){
  
}
SDS_Project_operand *SDS_Project_operand_deserilize(void *buf){

}
