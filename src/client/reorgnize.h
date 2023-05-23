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
#ifndef __SDS_REORGANIZE_H__
#define __SDS_REORGANIZE_H__

#include "sds-query.h"

typedef struct SDS_Reorg_status{
  int32_t                    error_code;  //0: NO_REORGANIZATION and 1: HAS_REORGANIZATION
  int32_t                    reor_status;
}SDS_Reorg_status;

int  SDS_Var_start_reorg(SDS_Object **vars, int *type, int *cores, int *time, char *argv[], int size,   SDS_Reorg_status status);
int  SDS_Var_monitor_reorg(SDS_Object **vars, int *type, int *cores, int *time, char *argv[], int n);

#endif
