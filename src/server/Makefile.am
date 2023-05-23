AUTOMAKE_OPTIONS=gnu
AM_CPPFLAGS=-I$(top_srcdir)/src/common
BUILT_SOURCES=db-message.protoc.pb-c.h ../common/message.protoc.pb-c.h

bin_PROGRAMS=$(top_builddir)/bin/server
__top_builddir__bin_server_SOURCES=sds-server.c metadata-db-access.c workqueue.c reorganization-manager-thread.c query-handler-thread.c db-message.protoc.pb-c.c reorganization-job.c data-reorganizer.c qsub.c listen-thread.c db-message.protoc.pb-c.h message.protoc.pb-c.h ../common/message.protoc.pb-c.c ../common/sds-error.c sds-config-server.c  mpirun.c db-access.c query-db-access.c ../common/iniparser.c ../common/dictionary.c ../common/sds-common.c  trace_log.c sbatch.c
__top_builddir__bin_server_CFLAGS=@SDS_SERVER_CFLAGS@ -I$(top_srcdir)/src/common
__top_builddir__bin_server_CPPFLAGS=@SDS_SERVER_CPPFLAGS@ -I$(top_srcdir)/src/common
__top_builddir__bin_server_LDFLAGS=@SDS_SERVER_LIBS@



##include_HEADERS=attribute-db-access.h query-handler-thread.h data-reorganizer.h reorganization-job.h listen-thread.h reorganization-manager-thread.h metadata-db-access.h sds-server.h qsub.h workqueue.h db-message.protoc.pb-c.h

# db-message.protoc.pb-c.c: db-message.protoc
#         $(PROTOBUFC) --c_out=. db-message.protoc
db-message.protoc.pb-c.c: db-message.protoc
	$(PROTOBUFC) --c_out=. ./db-message.protoc

db-message.protoc.pb-c.h: db-message.protoc
	$(PROTOBUFC) --c_out=. ./db-message.protoc

message.protoc.pb-c.h:  
	cd ../common && $(MAKE) message.protoc.pb-c.h 
message.protoc.pb-c.c: 
	cd ../common && $(MAKE) message.protoc.pb-c.c

clean-local:
	rm ./db-message.protoc.pb-c.c 
	rm ./db-message.protoc.pb-c.h
