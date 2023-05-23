AUTOMAKE_OPTIONS=gnu
BUILT_SOURCES=message.protoc.pb-c.h
AM_LDFLAGS=@SDS_CLIENT_LIBS@
AM_CFLAGS=@SDS_CLIENT_CFLAGS@
AM_CPPFLAGS=@SDS_CLIENT_CPPFLAGS@

message.protoc.pb-c.h: #message.protoc
	$(PROTOBUFC) --c_out=. ./message.protoc -I ./ -I ../server

include_HEADERS=sds-common.h sds-error.h message.protoc.pb-c.h

##message.protoc.pb-c.c: message.protoc
##	echo "am here also message.c"
##	$(PROTOBUFC) --c_out=. ./message.protoc -I ./ -I ../server

clean-local:
	rm message.protoc.pb-c.h message.protoc.pb-c.c
