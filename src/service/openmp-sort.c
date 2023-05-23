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
/* 	quicksort_omp: OpenMP parallel version of quick sort.
 *
 * 	usage: quicksort_omp Nelements [Nthreads]
 *	Nelements = # of integers to sort 
 *	Nthreads = # of OMP threads -- MUST BE A POWER OF 2! (Can be 1)
 *		   If unspecified then it uses all available cores 
 *
 *	Compile: gcc -O2 quicksort_omp.c -o quicksort_omp -lm -fopenmp -std=c99
 *
 *	Generates Nelements random integers and sorts them in ascending order.  
 *	Runs a check at the end to make sure the list is sorted correctly.
 *
 *	Algorithm: The C qsort() function is run on OMP threads to create
 *	a piecewise sorted list, and then those pieces are merged into
 *	a final sorted list.  Note that it requires a temporary array of
 *	(max) size Nelements for the merging.
 *	
 *	Benchmarks (YMMV): On an 8-core Xeon, sort time for 100 million 
 *	elements is 14.0s on 1 thread, 7.7s on 2, 4.6s on 4, and 3.6s on 8.
 *
 *	Romeel Dave' 5.April.2012
 */
  
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <omp.h>
#include "mpi.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include <string.h>

#define VERBOSE 0
  
void srand48();
double drand48();
  
static int CmpInt(const void *a, const void *b)
{
  return ( *(int*)a - *(int*)b );
}
 

bool CPPCmpInt( int a, int b)
{
  return (a<b);
}
 
/* Merge sorted lists A and B into list A.  A must have dim >= m+n */
void merge(int A[], int B[], int m, int n) 
{
  int i=0, j=0, k=0, p;
  int size = m+n;
  int *C = (int *)malloc(size*sizeof(int));
  while (i < m && j < n) {
    if (A[i] <= B[j]){
      C[k] = A[i++];
    }else {
      C[k] = B[j++];
    }
    k++;
  }
  if (i < m){
    for (p = i; p < m; p++,k++){
      C[k] = A[p];
    }
  }else{
    for(p = j; p < n; p++,k++){
      C[k] = B[p];
    }
  }
  
  //for( i=0; i<size; i++ ) 
  //  A[i] = C[i];
  memcpy(A, C, sizeof(int)*size);
  
  free(C);
}
  
/* Merges N sorted sub-sections of array a into final, fully sorted array a */
void arraymerge(int *a, int size, int *index, int N)
{
  int i,   thread_size; 

  while(N>1){
    thread_size = size/N;
    for( i=0; i<N; i++ ){ 
      index[i]=i * thread_size; 
    }
    index[N]=size;
    
#pragma omp parallel for private(i) 
    for( i=0; i<N; i+=2 ) {
      merge(a+index[i],a+index[i+1],index[i+1]-index[i],index[i+2]-index[i+1]);
    }
    N /= 2;
  }
}

int main(int argc,char **argv)
{
  int i;
  if( argc != 2 && argc != 3 && argc != 4 ) {
    fprintf(stderr,"usage: quicksort_omp Nelements [Nthreads]\n");
    return -1;
  }
  
  int mpi_rank, mpi_size =1;

  std::vector<int> a;

  // set up array to be sorted
  int size = atoi(argv[1]);
  //int *a = (int *)malloc(size*sizeof(int));
  //if(a == NULL){
  //  printf("Memeory allocation error for a");
   // exit(-1);
  //}
  srand48(8675309);
  for(i=0; i<size; i++){
    //a[i] = (int) (size*drand48());
    a.push_back((int) (size*drand48()));
  }
  
  // set up threads
  int threads = omp_get_num_threads(); //omp_get_max_threads();
  if( argc >= 3 ){ 
     threads=atoi(argv[2]);
  }
  
  if(argc == 4){
    MPI_Init(&argc, &argv );
    MPI_Comm_size (MPI_COMM_WORLD, &mpi_size );
    MPI_Comm_rank (MPI_COMM_WORLD, &mpi_rank );
  }

  omp_set_num_threads(threads);
  
  //int threads = mic_threads * mpi_size;
  printf("Using [%d] threads in total (mpi size = %d)to openmp sort \n", threads, mpi_size);
  
  //omp_set_num_threads(threads);
  int *index = (int *)malloc((threads+1)*sizeof(int));
  int  thread_size = size/threads;
  for(i=0; i<threads; i++){
    index[i]=i * thread_size; 
    printf("%d ", index[i]);
  }
  index[threads]=size;
  printf("%d (index)\n", index[threads]);
  
  /* Main parallel sort loop */
  double start = omp_get_wtime();
  
#pragma omp parallel for private(i)
  for(i=0; i<threads; i++){ 
    //qsort(a+index[i], index[i+1]-index[i], sizeof(int), CmpInt);
    std::sort(a.begin()+index[i], a.begin()+index[i+1], CPPCmpInt);
  }
  
  printf("Sorting is done ! \n");
  double middle = omp_get_wtime();
  
  /* Merge sorted array pieces */
  //if(threads>1 ) 
  //  arraymerge(a,size,index,threads);

  double end = omp_get_wtime();
  fprintf(stderr,"sort time = %g s, of which %g s is merge time\n",end-start,end-middle);
  
  /* Check the sort -- output should never show "BAD: ..." */
  for(i=1; i<size; i++){ 
    if( a[i-1]>a[i] ) 
      fprintf(stderr,"BAD: %d %d %d\n",i,a[i-1],a[i]);
  }
  
  if( argc == 4 ){ 
    MPI_Finalize();
  }

  return 0;
}
