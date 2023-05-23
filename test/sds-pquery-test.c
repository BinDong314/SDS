
#include "sds-pquery.h"

#ifdef SDS_CLIENT_MPI
#include "mpi.h"
#endif

/*
 * This file contains simple test codes for the interface defined in src/clinet/sds-pquery.c
 *  Filter two datasets (testd) of two files (testf1.h5p and testf2.h5p) with "testd>97"
 *  Results are stored in "testf3.h5p"
 *
 *  Author: Bin Dong <dbin at lbl.gov >
 *  Copyright 2015 the Regents of the University of California
 *
 * Step to test:
 *  1, mpirun -n 1 ./fake-hdf5 -f testf1.h5p -g /testg -d /testg/testd -n 1 -s 100 -t 0
 *  2, mpirun -n 1 ./fake-hdf5 -f testf2.h5p -g /testg -d /testg/testd -n 1 -s 100 -t 0
 *  3, mpirun -n 1 ./sds-pquery-test
 *  4, h5dump testf3.h5p (check the results)
 */
int  main(int argc, char *argv[]){

#ifdef SDS_CLIENT_MPI
  MPI_Init(&argc, &argv);
#endif

  char  *file[2] = {"testf1.h5p", "testf2.h5p"};
  char  *group[1] = {"/testg"};
  char  *dataset[1] = {"testd"};
  char  *rfile[1] = {"testf3.h5p"};

  int   cores = 2;
  char *other = "-r -l 10";
  
  SDS_H5index(file, 2, group, 1, dataset, 1, cores, NULL, other);

#ifdef SDS_CLIENT_MPI
  SDS_H5filter(file, 2, group, 1, dataset, 1, "testd > 97", rfile, 1, MPI_COMM_WORLD);
#else
  SDS_H5filter(file, 2, group, 1, dataset, 1, "testd > 97", rfile, 1, 1);
#endif
  
#ifdef SDS_CLIENT_MPI
  MPI_Finalize();
#endif

  return 0;
}
