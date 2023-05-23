#include "sds-pquery.h"
/*
 * This file contains simple test codes for the interface defined in src/clinet/sds-pquery.c
 * 1, ./fake-hdf5 -f testf.h5p -g /testg -d /testg/testd -n 1 -s 100 -t 0 
 * 2, ./sds-hist-test. it will print below results at end
 * hist_count 10
 * hist_count 10
 * hist_count 10
 * hist_count 10
 * hist_count 10
 * hist_count 10
 * hist_count 10
 * hist_count 10
 * hist_count 0   
 */
int  main(int argc, char *argv[]){
  
  char  *file[1]    = {"testf.h5p"};
  char  *group[1]   = {"/testg"};
  char  *dataset[1] = {"testd"};
  char  *qstr = "testd > 10";
  char  *begin = "10";
  char  *stripe = "10";
  char  *end    = "100";
  
  int   *hist_count;
  int    hist_count_size;
  
  SDS_H5histgram(file, 1, group, 1, dataset, 1, 1, qstr, begin, stripe, end,  &hist_count, &hist_count_size);
  int i;
  for(i = 0; i < hist_count_size; i++)
    printf("hist_count %d \n", hist_count[i]);
  
  return 0;
}
