
#include "sds-pquery.h"

#ifdef SDS_CLIENT_MPI
#include "mpi.h"
#endif

/*
 * This file contains simple test codes for the interface defined in src/clinet/sds-pquery.c
 */
int  main(int argc, char *argv[]){

#ifdef SDS_CLIENT_MPI
  MPI_Init(&argc, &argv);
#endif

  //char  *file[1] = {"vpic-2GB-energy.h5p"};
  char  *file[1] ={"vpic-2GB-energy.h5p"};
  char  *group[1] = {"/Step#0"};
  char  *dataset[1] = {"Energy"};
  char  *rfile[1] = {"vpic-2GB-energy-filterd.h5p"};

  int    cores = 2;
  char  *other = "-r -l 5000000";
  
  SDS_H5index(file, 1, group, 1, dataset, 1, cores, NULL, other);

#ifdef SDS_CLIENT_MPI
  SDS_H5filter(file, 1, group, 1, dataset, 1, "Energy > 4", rfile, 1, MPI_COMM_WORLD);
#else
  SDS_H5filter(file, 1, group, 1, dataset, 1, "Energy > 4", rfile, 1, 1);
#endif
  
#ifdef SDS_CLIENT_MPI
  MPI_Finalize();
#endif

  return 0;
}
