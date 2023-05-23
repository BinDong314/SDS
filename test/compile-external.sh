#!/bin/bash
##This is am example to compile hdf5 code using SDS' external plugin to access

##packages installed on edison
depend_package_home=/project/projectdirs/m1248/sds-depends-packages/install

##modify following packages based on your own installation
protobuf_dir="$depend_package_home/protobuf"
protobufc_dir="$depend_package_home/protobuf-c"
hdf5_dir="$depend_package_home/hdf5-vol-r25561"

##if you use cc on Edison, no need to use mpi_dir
mpi_dir=/opt/cray/mpt/7.0.0/gni/mpich2-gnu/49

##path to directory where SDS client is installed
sds_dir=/global/homes/d/dbin/sds-0.0.1-install

##Compile test code "read-interface-test-external-plugin.c", which use sds-external-vol.c plugin

rm read-test-external 

gcc read-interface-test-external-plugin.c ../src/client/sds-external-vol.c  -o read-test-external -L$sds_dir/lib -lsdsclient  -I $sds_dir/include -I $mpi_dir/include -I $hdf5_dir/include -I $protobuf_dir/include -I$protobufc_dir/include   -L$mpi_dir/lib -L$hdf5_dir/lib -L$protobuf_dir/lib -L$protobufc_dir/lib  -lhdf5 -lprotobuf -lprotobuf-c -lmpich -lhdf5_hl -lz
