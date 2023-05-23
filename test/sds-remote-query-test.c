#include "sds-query.h"
//#include "mpi.h"
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
 *   mpirun -n 1 ./fake-hdf5 -f ./testf.h5 -g /testg -d /testg/testd -n 1 -s 100 -t 1
 *
 * 2, Then:
 *   mpirun -n 1 ./sds-query-test
 */
int main(int argc, char *argv[]){
  SDS_Object          *e_obj;  
  SDS_Collection      *c_id, *c_result;
  SDS_Query_tree      *q_tree_id;
  SDS_Condition_tree  *c_tree_id;
  SDS_Query_handle    *query_handle;

  //Creat a HDF5 object
  e_obj        = SDS_Object_init("./testf.h5", "/testg", "testd", SDS_HDF5, SDS_FLOAT);
  SDS_Object_print_id(e_obj);

  //Create a SDS Collection and append the object to the end
  c_id         = SDS_Collection_init(1);
  SDS_Collection_append(c_id, e_obj);
  SDS_Collection_list_objects(c_id);

  //Create a query tree on this collection
  q_tree_id    = SDS_Query_tree_init(c_id);
  SDS_Query_tree_op_type(q_tree_id);
  
  SDS_Value_union value;
  value.f=0.1;
  //Init and apply "select" to the collection
  c_tree_id    = SDS_Condition_tree_init(0);
  SDS_Condition_tree_apply(c_tree_id, SDS_GT, value);

  //value.f=0.999700;
  //SDS_Condition_tree_apply(c_tree_id, SDS_LT, value);

  q_tree_id    = SDS_Query_tree_apply(q_tree_id, SDS_SELECT, c_tree_id);
  SDS_Query_tree_op_type(q_tree_id);
  
  //Init a query
  query_handle = SDS_Query_init(q_tree_id, 1);
  //Run the query
  SDS_Query_rrun(query_handle);
  
  //Print the results
  c_result = SDS_Query_get_result(query_handle);
  SDS_Collection_list_data(c_result, 0);

  //Releaset the query
  SDS_Query_finalize(query_handle);

  
  //MPI_Finalize();
  return 0;
}
