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
 * This file defines the mapping between index/reorganization type and its corresponding service code 
 *
 *  Author: Bin Dong <dbin at lbl.gov >
 *  Copyright 2015 the Regents of the University of California
 * 
 */

#include "data-reorganizer.h"

//Hardcode here:
void  find_reorganizer(int type, int subtype, reorganizer_t *reorganizer){
  switch(type){
    case BUILD_INDEX:
      switch(subtype){
        case SORT_INDEX:
          *reorganizer = reorganizer_array[0];
          break;
        case BITMAP_INDEX:
          *reorganizer = reorganizer_array[2];
          break;
        case MDBIN_INDEX:
          *reorganizer = reorganizer_array[3];
          break;
        default:
          log_msg("UnKnow type in find_reorganizer()");
          break;
      }
      break;
    case REORGANIZE_DATA:
      switch(subtype){
        case SORT:
          *reorganizer = reorganizer_array[0];
          break;
        case TRANSFORM:
          *reorganizer = reorganizer_array[1];
          break;
        case MDBIN:
          *reorganizer = reorganizer_array[3];
          break;
        case SFC:
          *reorganizer = reorganizer_array[4];
          break;
        case CONCATENATION:
          *reorganizer = reorganizer_array[5];
          break;
       default:
          log_msg("UnKnow type in find_reorganizer()");
          break;
      }
      break;
    default:
      log_msg("UnKnow type in find_reorganizer()");
      break;
  }
}



