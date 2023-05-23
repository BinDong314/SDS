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
 * This file is the raw interface for Berkeley DB
 * 
 *  Author: Bin Dong <dbin at lbl.gov >
 *  Copyright 2015 the Regents of the University of California
 * 
 */

#include "db-access.h"
#include "sds-common.h"
#include "sds-config-server.h"

//extern SDS_Bool    metadb_in_memory;

int open_db(char *table_name, DB **dbpp, int mode){
  DB *dbp;
  u_int32_t flags;
  int ret;
  /* DB structure handle */
  /* database open flags */
  /* function return value */
  /* Initialize the structure. This
   * database is not opened in an environment,
   * so the environment pointer is NULL. */
  ret = db_create(&dbp, NULL, 0);
  if (ret != 0) {
    printf("db create error ! \n");
    exit(-1);
  }
  
  *dbpp = dbp;
  
  /* Database open flags */
  flags = mode; 
		
  dbp->set_cachesize(dbp, 0, 4*1024*1024, 0);

  /* open the database */
  if (metadb_in_memory == SDS_TRUE){
    ret = dbp->open(dbp,        /* DB structure pointer */
                    NULL,       /* Transaction pointer */
                    //table_name, /* On-disk file that holds the database. */
                    NULL,      /*Try to store in Memory */
                    NULL,       /* Optional logical database name */
                    DB_BTREE,   /* Database access method */
                    flags,      /* Open flags */
                    0);         /* File mode (using defaults) */
  }else{
    ret = dbp->open(dbp,        /* DB structure pointer */
                    NULL,       /* Transaction pointer */
                    table_name, /* On-disk file that holds the database. */
                    NULL,       /* Optional logical database name */
                    DB_BTREE,   /* Database access method */
                    flags,      /* Open flags */
                    0);         /* File mode (using defaults) */
  }
  if (ret != 0) {
    printf("dbp->open in create error ! \n");
    exit(-1);
  }
  return 1;
}


int close_db(DB *dbpp){
  int ret;
  if (dbpp != NULL) {
    ret = dbpp->close(dbpp, 0);
    if (ret != 0) 
      log_msg("SDS Server Closing DB failed. \n");
  }

  return ret;
}
