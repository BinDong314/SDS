#include "sds-query.h"

#ifdef SDS_CLIENT_MPI
#include "mpi.h"
#endif

/*
 *Following example code selects dataset "testd" based on the ranges "testd > 0.1".
 * Dataset "testd" is stored in group "/testg" of HDF5 file "testf.h5".
 *
 *  SQL-like expression is:
 *         select testd
 *         from  ./testf.h5/testg 
 *         where  testd >0.1 
 *
 * Tips for run: 
 * 1, Generate testf.h5 with fake-hdf5 
 *   mpirun -n 1 ./fake-hdf5 -f ./testf.h5p -g /testg -d /testg/testd -n 1 -s 100 -t 0
 *
 * 2, Then:
 *   mpirun -n 1 ./sds-query-test
 */
int main(int argc, char *argv[]){
  SDS_Object          *e_obj;  
  SDS_Collection      *c_id, *c_result;
  SDS_Query_tree      *q_tree_id;
  SDS_Query_handle    *query_handle;
  SDS_Condition_tree  *c_tree_id;
  char                *qstr="testd>0.92";

#ifdef SDS_CLIENT_MPI
  MPI_Init(&argc, &argv);
#endif

  //Creat a object from a HDF5 dataset
  e_obj        = SDS_Object_init("./testf.h5p", "/testg", "testd", SDS_HDF5, SDS_FLOAT);
  SDS_Object_print_id(e_obj); //for debug

  //Create a SDS Collection and append the object to the end
  c_id         = SDS_Collection_init(1); //"1" means that this collection has one objects
  SDS_Collection_append(c_id, e_obj);
  SDS_Collection_list_objects(c_id); //for debug

  //Create a query tree on this collection
  q_tree_id    = SDS_Query_tree_init(c_id);
  q_tree_id    = SDS_Query_tree_apply(q_tree_id, SDS_SELECT, qstr);
  
  //SDS_Query_tree_op_type(q_tree_id);
  //parse_condition_tree(qstr, &c_tree_id);

  //Create a query tree with a single "select" operation
  //c_tree_id is the parameter of "select"
  //SDS_Query_tree_op_type(q_tree_id);
  
  //Init a query
#ifdef SDS_CLIENT_MPI
  query_handle = SDS_Query_init(q_tree_id, MPI_COMM_WORLD);
#else
  query_handle = SDS_Query_init(q_tree_id, 1);
#endif

  //Run the query
  SDS_Query_run(query_handle);
  
  //Print the results (for debug)
  c_result = SDS_Query_get_result(query_handle);
  SDS_Collection_list_data(c_result, 0);

  //Release the query
  SDS_Query_finalize(query_handle);

#ifdef SDS_CLIENT_MPI
  MPI_Finalize();
#endif

  return 0;
}
