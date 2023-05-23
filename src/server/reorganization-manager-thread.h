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
 * This file is the main thread who receives "build reorganization/build index/run analysis" request from listen thread
 * Author: Bin Dong <dbin at lbl.gov >
 * Copyright 2015 the Regents of the University of California
 * 
 */

#ifndef __REORGANIZATION_MANAGER_THREAD_H__
#define __REORGANIZATION_MANAGER_THREAD_H__

#include <pthread.h>
#include "workqueue.h"
#include "sds-server.h"
#include "workqueue.h"
#include "message.protoc.pb-c.h"
#include "metadata-db-access.h"
#include "data-reorganizer.h"
#include "sds-error.h"
//#include "attribute-db-access.h"
#include "sds-config-server.h"
#include "sds-job.h"
#include "sds-common.h"

int reorganization_manager_thread_init(workqueue_t *workqueue, int numWorkers);
int prepare_and_start_one_reorganization(SdsObject *object, int job_type, int job_subtype, int cores, int time_secs, char *other_pamaters);


#endif
