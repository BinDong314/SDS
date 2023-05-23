AUTOMAKE_OPTIONS=gnu
AM_CPPFLAGS=-I$(top_srcdir)/src/common -I$(top_srcdir)/src/server
BUILT_SOURCES=../common/message.protoc.pb-c.h sds-cond-lexer.h sds-cond-parser.h
lib_LTLIBRARIES=libsdsclient.la
##libsdsvol_la_SOURCES=sds-external-vol.c server-connector.c 
libsdsclient_la_SOURCES=../common/sds-error.c  sds-collection.c sds-condition-tree.c sds-hdf5.c sds-binary.c sds-netcdf.c sds-index-file.c sds-object.c sds-project.c sds-query-tree.c sds-query.c  sds-histgram.c sds-reorg-file.c ../common/sds-common.c  sds-client-communicator.c  ../common/message.protoc.pb-c.c ../common/mp-fill.c sds-cond-parser.c sds-cond-lexer.c sds-parser.c  sds-pquery.c sds-config-client.c ../common/iniparser.c ../common/dictionary.c fastquery-c-interface.cpp
libsdsclient_la_LDFLAGS=-version-info 0:0:0 @SDS_CLIENT_LIBS@ -shared @SDS_SERVER_LIBS@ 
#-shrext ".so"
libsdsclient_la_CFLAGS=@SDS_CLIENT_CFLAGS@ -I$(top_srcdir)/src/common -I$(top_srcdir)/src/server @SDS_SERVER_CFLAGS@
libsdsclient_la_CPPFLAGS=@SDS_CLIENT_CPPFLAGS@ -I$(top_srcdir)/src/common -I$(top_srcdir)/src/server @SDS_SERVER_CPPFLAGS@

include_HEADERS=sds-external-vol.h ../common/sds-common.h ../common/sds-error.h ../common/message.protoc.pb-c.h fastquery-c-interface.h reorgnize.h sds-array.h sds-collection.h sds-condition-tree.h sds-hdf5.h sds-index-file.h sds-object.h sds-project.h sds-query-tree.h sds-query.h sds-reorg-file.h sds-select.h sds-client-communicator.h sds-cond-parser.h sds-cond-lexer.h sds-parser.h sds-pquery.h  ../common/mp-fill.h sds-histgram.h


#sds-client-communicator.c:message.protoc.pb-c.h
#	cd ../common && $(MAKE) message.protoc.pb-c.h
message.protoc.pb-c.h:  
	cd ../common && $(MAKE) message.protoc.pb-c.h 

../common/message.protoc.pb-c.h:
	$(PROTOBUFC) --c_out=../common ../common/message.protoc -I ../common -I ../server

sds-cond-lexer.c:sds-cond-lexer.l
	flex sds-cond-lexer.l
sds-cond-parser.c:sds-cond-parser.y
	bison sds-cond-parser.y

sds-cond-lexer.h:sds-cond-lexer.l
	flex sds-cond-lexer.l
sds-cond-parser.h:sds-cond-parser.y
	bison sds-cond-parser.y


