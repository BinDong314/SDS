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

#include "sds-histgram.h"



SDS_Histgram_operand   *SDS_Histgram_operand_init(int dim, char *begin_str, char *stripe_str, char *end_str, char *query_str){
  SDS_Histgram_operand *hist_ope;
  
  hist_ope = malloc(sizeof(SDS_Histgram_operand));
  
  hist_ope->dim = dim;
  hist_ope->begin_str  = strdup(begin_str);
  hist_ope->stripe_str = strdup(stripe_str);
  hist_ope->end_str    = strdup(end_str);
  hist_ope->query_str  = strdup(query_str);
  
  return hist_ope;
}

int  SDS_Histgram_operand_finalize(SDS_Histgram_operand *hist_operand){
  if(hist_operand == NULL){
    return 0;
  }
  
  if(hist_operand->begin_str != NULL)
    free(hist_operand->begin_str);

  if(hist_operand->stripe_str != NULL)
    free(hist_operand->stripe_str);

  if(hist_operand->end_str != NULL)
    free(hist_operand->end_str);

  if(hist_operand->query_str != NULL)
    free(hist_operand->query_str);
  
  free(hist_operand);
  
  return 0;
}


