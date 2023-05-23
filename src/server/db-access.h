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

#ifndef __DB_ACCESS_H__
#define __DB_ACCESS_H__

#include        <db.h>

typedef struct sds_dbs {
  DB   *sds_metadata_dbp;               /* Database containing metadata */
  DB   *sds_io_pattern_dbp;             /* Database containing the io_pattern information (not used)*/
  DB   *sds_query_dbp;                  /* Database containing the query (for running analysis) */
  char *db_home_dir;                    /* Directory containing the database files */
  char *sds_metadata_db_name;           /* Name of the inventory database */
  char *sds_query_dbp_name;             /* Name of the vendor database */
  char *sds_io_pattern_db_name;         /* Name of the vendor database */
}sds_dbs_t;

//Open and close the raw Berkeley DB
int open_db(char *table_name, DB **dbpp, int mode);
int close_db(DB *dbpp);


#endif
