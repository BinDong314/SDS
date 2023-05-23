#include "sds-query.h"
#include "reorgnize.h"

int main(int argc, char *argv[]){
  SDS_Object     *x;
  char            *file  = "./testf.h5p";
  char            *group = "/testg";
  char            *dset  = "testd";
  int              mpi_rank = 100;
  char            *dir_name = "./";
  char            *app_name = "text.exe";
  int              dim_rank = 3;
  SDS_Reorg_status status;
  
  //Intialize the object
  x = SDS_Object_init(file, group, dset, SDS_HDF5, SDS_FLOAT);
  
  //Start trace log analysis on x
  SDS_start_trace_analysis(x, mpi_rank, dir_name, app_name, dim_rank);
  
  SDS_Object_finalize(x);
}
