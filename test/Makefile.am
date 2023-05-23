AUTOMAKE_OPTIONS=gnu
AM_CPPFLAGS=-I$(top_srcdir)/src/client -I$(top_srcdir)/src/common @SDS_SERVER_CPPFLAGS@ #@SDS_CLIENT_CPPFLAGS@
AM_CFLAGS=@SDS_SERVER_CFLAGS@ #@SDS_CLIENT_CFLAGS@
AM_LDFLAGS=@SDS_SERVER_LIBS@ #@SDS_CLIENT_LIBS@

##LDADD = ../src/client/libsdsvol.la
bin_PROGRAMS=sds-query-test sds-remote-query-test fake-hdf5 sds-pquery-test reorganize sds-perf-test trace-test sds-hist-test
##reorganize fake-hdf5 read-test-external reorganize-new
#EXTRA_PROGRAMS=sds-query-test reorganize fake-hdf5 read-test-external reorganize-new

##Compile external plugin still has problem
##read_test_external_SOURCES=read-interface-test-external-plugin.c ../src/client/server-connector.c ../src/common/message.protoc.pb-c.h ../src/common/message.protoc.pb-c.c  ../src/client/sds-external-vol.c ../src/common/sds-error.c 
##read_test_external_DEPENDENCIES=../src/client/libsdsvol.la 
##read_test_external_LDADD=../src/client/libsdsvol.la

sds_hist_test_SOURCES = sds-hist-test.c
sds_hist_test_DEPENDENCIES=../src/client/libsdsclient.la
sds_hist_test_LDADD=../src/client/libsdsclient.la


trace_test_SOURCES = trace-test.c
trace_test_DEPENDENCIES=../src/client/libsdsclient.la
trace_test_LDADD=../src/client/libsdsclient.la

sds_perf_test_SOURCES=sds-perf-test.c
sds_perf_test_DEPENDENCIES=../src/client/libsdsclient.la
sds_perf_test_LDADD=../src/client/libsdsclient.la

reorganize_SOURCES=reorganize.c	
reorganize_DEPENDENCIES=../src/client/libsdsclient.la
reorganize_LDADD=../src/client/libsdsclient.la

fake_hdf5_SOURCES=fake-HDF5-data.c
fake_hdf5_DEPENDENCIES=../src/client/libsdsclient.la
fake_hdf5_LDADD=../src/client/libsdsclient.la

sds_pquery_test_SOURCES=sds-pquery-test.c
sds_pquery_test_DEPENDENCIES=../src/client/libsdsclient.la
sds_pquery_test_LDADD=../src/client/libsdsclient.la


sds_query_test_SOURCES=sds-query-test.c 
sds_query_test_DEPENDENCIES=../src/client/libsdsclient.la
sds_query_test_LDADD=../src/client/libsdsclient.la

sds_remote_query_test_SOURCES=sds-remote-query-test.c 
sds_remote_query_test_DEPENDENCIES=../src/client/libsdsclient.la
sds_remote_query_test_LDADD=../src/client/libsdsclient.la

#reorganize_new_SOURCES=reorganize-new.c ../src/client/sds-query.c ../src/client/reorgnize.c ../src/client/server-connector.c ../src/common/message.protoc.pb-c.h ../src/common/message.protoc.pb-c.c ../src/common/sds-error.c 

#sds-query-test:
#	gcc -o sds-query-test sds-query-test.c ../src/client/sds-query.c ../src/client/server-connector.c ../src/common/message.protoc.pb-c.h ../src/common/message.protoc.pb-c.c ../src/common/sds-error.c  @SDS_SERVER_CFLAGS@ @SDS_CLIENT_CFLAGS@  @SDS_SERVER_LIBS@ @SDS_CLIENT_LIBS@ -I../src/common -I../src/client -I ../

../src/client/libsdsclient.la:
	cd ../src/client && $(MAKE)

