#!/bin/bash

##This is example file to run SDS configuration (SDS Server)
##Please update following packages to your own 

##Directory to install SDS Server
prefix_dir=/global/homes/d/dbin/sds-0.0.1-install

##packages installed on edison
depend_package_home=/Users/john/Darwin
libevent_dir="$depend_package_home/libevent"
protobuf_dir="$depend_package_home/protobuf"
protobufc_dir="$depend_package_home/protobuf-c"
berkeley_db_dir="$depend_package_home/db"
hdf5_dir="/opt/local/"
fastquery_dir="$depend_package_home/fq"
fastbit_dir="$depend_package_home/fastbit-trunk"
mpi_dir="/opt/cray/pe/mpt/7.4.0/gni/mpich-intel/15.0"  
##mpi_dir="/opt/cray/mpt/7.2.1/gni/mpich2-gnu/49/"

##Specify --enable-server to configure SDS Server and also specify the packages for SDS Server
./configure CC=gcc --with-libevent=$libevent_dir --with-protobuf=$protobuf_dir --with-protobufc=$protobufc_dir --with-db=$berkeley_db_dir   --with-hdf5=$hdf5_dir --with-mpi=$mpi_dir --prefix=$prefix_dir --enable-server --with-fastquery=$fastquery_dir --with-fastbit=$fastbit_dir --enable-debug   --enable-clientmpi=yes
