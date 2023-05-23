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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "sds-error.h"
#include "workqueue.h"
#include "sds-server.h"
#include "message.protoc.pb-c.h"
#include "metadata-db-access.h"


void workqueue_shutdown(workqueue_t *workqueue) {
    worker_t *worker = NULL;

    /* Set all workers to terminate. */
    for (worker = workqueue->workers; worker != NULL; worker = worker->next) {
        worker->terminate = 1;
    }

    /* Remove all workers and jobs from the work queue.
     * wake up all workers so that they will terminate. */
    pthread_mutex_lock(&workqueue->jobs_mutex);
    workqueue->workers = NULL;
    workqueue->waiting_jobs = NULL;
    pthread_cond_broadcast(&workqueue->jobs_cond);
    pthread_mutex_unlock(&workqueue->jobs_mutex);
}

void workqueue_add_job(workqueue_t *workqueue, job_t *job) {
    /* Add the job to the job queue, and notify a worker. */
    pthread_mutex_lock(&workqueue->jobs_mutex);
    LL_ADD(job, workqueue->waiting_jobs);
    pthread_cond_signal(&workqueue->jobs_cond);
    pthread_mutex_unlock(&workqueue->jobs_mutex);
}
