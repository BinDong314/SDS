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
 * This file is the interfaces for metadata Berkeley DB
 * 
 *  Author: Bin Dong <dbin at lbl.gov >
 *  Copyright 2015 the Regents of the University of California
 * 
 */

#ifndef __QUERY_DB_ACCESS_H__
#define __QUERY_DB_ACCESS_H__

#include <db.h>
#include <string.h>
#include "db-message.protoc.pb-c.h"
#include "message.protoc.pb-c.h"
#include "sds-common.h"
#include "reorganization-job.h"
#include "data-reorganizer.h"
#include "sds-error.h"
#include "db-access.h"

#define SDS_QUERY_TABLE_NAME                      "sds_query_table.db"

int open_sds_query_db();
int create_sds_query_db();
int close_sds_query_db(sds_dbs_t *sds_dbs);
int write_query_record(DB *dbpp, RequestAnalyData *key, ResponseAnalyData *value);
int read_query_record(DB *dbpp, RequestAnalyData *key, ResponseAnalyData *value);
#endif
