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
#ifndef C_FQ_H
#define C_FQ_H

#ifndef  SDS_CLIENT_MPI
#define FQ_NOMPI
#endif

#include "sds-common.h"

#ifdef __cplusplus
extern "C" {
#endif
  
unsigned int fq_multi_dsets(char *qstr, char *filename, char *group, const char **dataset, int datasetcount, char *indexfilename, int mpi_length, SDS_Value_union **buf, unsigned int *fq_hits, char *other);


int fq_get_hits(char *qstr, char *filename, char *group, char *dataset[], int datasetcount, char *indexfilename, void *buf, int length, char *other);

int fq_read_data(char *qstr, char *filename, char *group, char *dataset[], int datasetcount, char *indexfilename, void *buf, int length, char *other);

int fq_hist(char *qstr, char *filename, char *group, const char **dataset, int dimension, char *indexfilename, int mpi_len, char *begin_str, char *stride_str, char *end_str,  int **fq_count, int *fq_count_size);


int get_mpi_length(char *fq_parameter);

#ifdef __cplusplus
}
#endif

#endif
