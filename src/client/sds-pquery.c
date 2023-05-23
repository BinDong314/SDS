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
 * This file is a wrapper for python query interface of SDS 
 * 
 *  Author: Bin Dong <dbin at lbl.gov >
 *  Copyright 2015 the Regents of the University of California
 * 
 */

#include "sds-pquery.h"

/*
 * SDS method to filter hdf5 datasets "(filenames/groups/dsets)" with "qstr" and
 * stores the results into the files specified by "rfname".
 * Inside SDS Client, this is "Select" operation
 *
 * -- (fames[0,..,nf]/groups[0,..,ng]/dsets[0,..,nd]), the triples used to specify HDF5 dataset. 
 *                 -- When nf = ng = nd, datasets in different files and different groups
 *                 -- When nf = ng = 1,  datasets in same file/group 
 *                 -- When ng = nd = 1,  datasets with the same name and group path in differnt files
 *                 -- Other cominbations (no support).
 *
 * -- "qstr",      conditional string on dsets. In this conditional string, dataset names in "dsets[0,..n]" are used as variable. 
 *                 Range operations on each variable: <, >, =, !=
 *                 Operations  between variables: AND (&&), OR (||)
 *                 --- When the datasets have same name (when n2 = n3 = 1 above), 
 *                     "qstr" with("dsets[0]") is extended to be "qstr OR(||) qstr ... " on all datasets
                        or:
 *                     "dataset name + order number" could be used as variable to specify condition on different datasets (no support).
 *
 *
 * -- "rfname[0,..,nr]",    the name of the files (automatically created by sds-pquery-h5) used to store the results .
 *                          the files are the HDF5 format and share the same group/dataset names as input files                  
 *                          --- When nr=0 ("rfname" in NULL), it uses the "fnames[0,..,n1].sds" as the results files
 *                          --- When nr=1 ("rfname "has only one file), all query results are concatenated into this file based 
 *                              on the order in  "fnames/groups/dsets".
 *                          --- Otherwise, "nr" of "rfname" must be equal to "ng" of "fnames"
 *
 *  Example for filtering dataset "testd" under two HDF5 files "testf1.h5" and  "testf2.h5". 
 *  The results is stored in "testf.result.h5"
 *
 *     fnames[0] = "testf1.h5" 
 *     fnames[1] = "testf2.h5"
 *     groups[0] = "/testg"
 *     dsets[0]  = "testd"
 *     rfname[0] = "testf.result.h5"
 *     SDS-Filter-h5d(fnames, 2,  groups, 1, dsets, 1, "testd > 1.5", rfname, 1);
 *     //As above "testd > 1.5" will be exteneded to "testf1.h5/testg/testd > 1.5 || testf2.h5/testg/testd > 1.5"
 *     //These is one file in "rfname", results will be concatenated into this sinle file.  
 */
int  SDS_H5filter(char **fnames, int nf, char **groups,  int ng, char **dsets, int nd, const char *qstr, char **rfname, int nr, SDS_Query_comm comm){
  int mpi_flag = 1;
  
#ifdef SDS_CLIENT_MPI
  MPI_Initialized(&mpi_flag);
  if(!mpi_flag){
    MPI_Init(NULL, NULL);
  }
#endif  

  int                  i;
  SDS_Object          *e_obj;  
  SDS_Collection     **c_id,  *c_id_temp,  *c_result;
  SDS_Query_tree      *q_tree_id;
  SDS_Condition_tree  *c_tree_root;
  SDS_Query_handle    *query_handle;
  int                  data_type;

  //log_msg("Create SDS Object and Collection ...");
  if(ng == 1 && nd == 1){
    //Create a SDS Collection and append the object to the end
    c_id =  malloc(nf * sizeof(SDS_Collection *));
    for(i = 0; i < nf; i++){
      log_msg(" object:  .%s [%s/%s] ", fnames[i], groups[0], dsets[0]);
      c_id_temp    = SDS_Collection_init(1); //"1" means that this collection has one objects
      data_type    = hdf5_dataset_datatype(fnames[i], groups[0], dsets[0]);
      e_obj        = SDS_Object_init(fnames[i], groups[0], dsets[0], SDS_HDF5, data_type);
      SDS_Collection_append(c_id_temp, e_obj);
      c_id[i] = c_id_temp;
    }
  }else if(nf == 1 && ng == 1){
    printf("To implement in furture ! \n");
  }else if(nf == ng && ng == nd && nf == nd){
    printf("To implement in furture ! \n");
  }else{
    printf("No support for the combination of (fames[0,..,nf]/groups[0,..,ng]/dsets[0,..,nd])!");
    exit(-1);
  }
  
  //Create a conditional tree from "qstr" 
  //log_msg("Parseing the %s ...", qstr);  //c_tree_root = 
  //parse_condition_tree(qstr, &c_tree_root);
  //printf("c_tree_id type %d, name %s \n", c_tree_root->type, c_tree_root->object_name);
  //print_condition_tree(c_tree_root); //For test
  
  //return 0;
  log_msg("Run the query ..." );
  //Create a query tree with a single "select" operation
  for(i = 0 ; i < nf; i++){
    //Create a query tree on this collecxtion
    SDS_Collection_list_objects(c_id[i]); //for debug
    
    q_tree_id    = SDS_Query_tree_init(c_id[i]);

    q_tree_id    = SDS_Query_tree_apply(q_tree_id, SDS_SELECT, (void *)qstr);
    
    //q_tree_id    = SDS_Query_tree_apply(q_tree_id, SDS_SELECT, c_tree_root);

    //SDS_Query_tree_op_type(q_tree_id);
    
    //Init a query
    query_handle = SDS_Query_init(q_tree_id, comm);
    
    //Run the query
    SDS_Query_run(query_handle);
    

    //Print results, for test
    c_result = SDS_Query_get_result(query_handle);
    SDS_Collection_list_data(c_result, 0);
    
    //Handle the results
    SDS_Collection_append_result(query_handle->result, SDS_FALSE, comm, rfname[0]);
    
    //Release the query
    SDS_Query_finalize(query_handle);
  }
 
#ifdef SDS_CLIENT_MPI
  if(!mpi_flag)
    MPI_Finalize();
#endif
}

/*
 * Create bitmap index on HDF5 datasets 
 * It only supports ng = nd = 1, which means create index  on datasets in all "fnames"
 * Other paremters:
 *    "core" :   the number of CPU cores to build the index
 *    "timestr": the time to run the job. It is in "hour:minute:second" formad. It is only usefull 
 *               for cluster version and NULL for single host.
 *    "other":   some other parameters to build bitmap index using fastquery (e.g., "-r" , force to build
 *               and "-l", length of data partition)  
 * 
 *  example for a single host
 *     fnames[0] = "testf1.h5" 
 *     fnames[1] = "testf2.h5"
 *     groups[0] = "/testg"
 *     dsets[0]  = "testd"
 *     rfname[0] = "testf.result.h5"
 *     cores     = 4
 *     SDS-Filter-h5d(fnames, 2,  groups, 1, dsets, 1, cores, NULL, "-r -l 100");
 *
 * example for cluster version  
 *     fnames[0] = "testf1.h5" 
 *     fnames[1] = "testf2.h5"
 *     groups[0] = "/testg"
 *     dsets[0]  = "testd"
 *     rfname[0] = "testf.result.h5"
 *     cores     = 4
 *     SDS-Filter-h5d(fnames, 2,  groups, 1, dsets, 1, cores, "00:30:00", "-r -l 100");
 */

int  SDS_H5index(char **fnames, int nf, char **groups,  int ng, char **dsets, int nd, int core, char *timestr, char *other){
  
  int mpi_flag = 1;
#ifdef SDS_CLIENT_MPI
  MPI_Initialized(&mpi_flag);
  if(!mpi_flag){
    MPI_Init(NULL, NULL);
  }
#endif 

  SDS_Object     **x;
  int              i, index_type[1], reorg_type[1], cores[1], time[1];
  char            *augs[1];
  int              status;
  int              data_type;
  
  //log_msg("Create SDS Object and Collection ...");
  if(ng == 1 && nd == 1){
    for(i = 0 ; i < nf; i++){
      //Intialize the variable in conditional string
      x = malloc(sizeof(SDS_Object *));
      data_type    = hdf5_dataset_datatype(fnames[i], groups[0], dsets[0]);
      x[0] = SDS_Object_init(fnames[i], groups[0], dsets[0], SDS_HDF5, data_type);
      
      index_type[0]  = BITMAP_INDEX; //0: sort. 1: transform. 2: index
      reorg_type[0]  = NONE_REORG;
      cores[0] = core;
      if(timestr != NULL){
        time[0]  = time_to_secs(timestr);
      }else{
        time[0]  = 0;
      }
      augs[0]  = other;
      
      //Start reorganization on x
      log_msg("Build index for %s (core=%d, time %d, other %s)", fnames[i], cores[0], time[0], augs[0]);
      if(SDS_start_collection_reorg(x, index_type, reorg_type, cores, time, augs, 1, status) < 0){
        log_msg("Could not build index for the file (use full-scan)");
      }
      
      SDS_Object_finalize(x[0]);
      free(x);
    }
  }else if(nf == 1 && ng == 1){
    printf("To implement in furture ! \n");
  }else if(nf == ng && ng == nd && nf == nd){
    printf("To implement in furture ! \n");
  }else{
    printf("No support for the combination of (fames[0,..,nf]/groups[0,..,ng]/dsets[0,..,nd])!");
    exit(-1);
  }

#ifdef SDS_CLIENT_MPI
  if(!mpi_flag)
    MPI_Finalize();
#endif
}



/*
 * Create histgram on HDF5 datasets 
 * It only supports ng = nd = 1 
 * Other paremters:
 *    "condstr" :   the conditional string 
 *    "begin", "stripe", "end":   paramters used to build histgram 
 *    "mpi_length":   the length of the sub array partition. 
 * 
 *  example for a single host
 *     fnames[0] = "testf1.h5" 
 *     groups[0] = "/testg"
 *     dsets[0]  = "testd"
 *     rfname[0] = "testf.result.h5"
 *     SDS_H5histgram(fnames, 1, groups, 1, dsets, 1, "testd > 1" , 10, "10", "10", "100");
 *
 */
int  SDS_H5histgram(char **fnames, int nf, char **groups,  int ng, char **dsets, int nd, int dim, char *condstr, char *begin, char *stripe, char *end, int **hist_count, int *nc){
  int                  mpi_flag = 1, i;
  SDS_Object          *obj;  
  SDS_Collection      *c_id,  *c_result;
  SDS_Query_tree      *q_tree_id;
  SDS_Condition_tree  *c_tree_root;
  SDS_Query_handle    *query_handle;
  int                  data_type, comm;
  
#ifdef SDS_CLIENT_MPI
  MPI_Initialized(&mpi_flag);
  if(!mpi_flag){
    MPI_Init(NULL, NULL);
  }
#endif
  
  //log_msg("Create SDS Object and Collection ...");
  if(nf == 1 && ng == 1){
    c_id  = SDS_Collection_init(nd); //"1" means that this collection has one objects
    for(i = 0 ; i < nd; i++){
      //Intialize the variable in conditional string
      //x = malloc(sizeof(SDS_Object *));
      data_type   = hdf5_dataset_datatype(fnames[0], groups[0], dsets[i]);
      obj         = SDS_Object_init(fnames[0], groups[0], dsets[i], SDS_HDF5, data_type);
      SDS_Collection_append(c_id, obj);
    }
  }else{
    printf("No support for the combination of (fames[0,..,nf]/groups[0,..,ng]/dsets[0,..,nd])!");
    exit(-1);
  }
  
  
  SDS_Histgram_operand   *hist_ope;
  hist_ope     = SDS_Histgram_operand_init(dim, begin, stripe, end, condstr);
  q_tree_id    = SDS_Query_tree_init(c_id);
  q_tree_id    = SDS_Query_tree_apply(q_tree_id, SDS_HISTGRAM, hist_ope);

  //Init a query
  query_handle = SDS_Query_init(q_tree_id, comm);
    
  //Run the query
  SDS_Query_run(query_handle);
  
  //printf("Query is finished ! \n");
  //Print results, for test
  c_result = SDS_Query_get_result(query_handle);
  (*nc)    = c_result->object_array[0]->my_size;
  int size_p = c_result->object_array[0]->my_size;
  int *temp;
  temp = malloc(sizeof(int)* size_p);
  SDS_Value_union *data = c_result->object_array[0]->data_buffer;
  for(i = 0; i < *nc ; i++){
    temp[i] = data[i].i;
    //printf("temp %d \n", temp[i]);
  }
  *hist_count = temp;

  //printf("temp sizeof  %d, sizeof hist = %d  \n", sizeof(temp), sizeof(*hist_count));
  //free(data);
  //SDS_Collection_list_data(c_result, 0);
  //SDS_Collection_list_objects(c_result);
  //SDS_Object_print_id();
  //obj = SDS_Collection_get_object(c_result, 0)
  //int count    obj-other;
  //Handle the results
  //SDS_Collection_append_result(query_handle->result, SDS_FALSE, comm, rfname[0]);
  
  //SDS_Collection_finalize(c_id);
  //SDS_Collection_finalize(c_result);
  //Release the query
  SDS_Query_finalize(query_handle);
  
#ifdef SDS_CLIENT_MPI
  if(!mpi_flag){
    MPI_Finalize();
  }
#endif
  
  return 0;
}
