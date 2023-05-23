#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#AC_PREREQ(2.59)
AC_INIT([SDS], [0.0.1], [dbin@lbl.gov])
AC_CONFIG_SRCDIR([config.h.in])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_HEADERS([config.h])

LT_INIT

#([disable-static])
AC_CONFIG_MACRO_DIR([m4])
# Checks for programs.

AC_PROG_CXX
AC_PROG_AWK
AC_PROG_YACC
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LEX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries.

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h libintl.h malloc.h netdb.h netinet/in.h stddef.h stdlib.h string.h sys/socket.h sys/time.h syslog.h unistd.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([dup2 gethostbyname gettimeofday memset mkdir socket strchr strdup strerror strrchr strstr strtol])


##############################################
## Start from here to check packages for SDS #
############################################## 

CPPFLAGS="$CPPFLAGS -fPIC"
CFLAGS="$CFLAGS -fPIC"

################################################################
## Using differnt FLAGS and LIBS for SDS Client and SDS Server #
## Because they depends on different packages.                 #
################################################################
AC_SUBST(SDS_CLIENT_CFLAGS)
AC_SUBST(SDS_CLIENT_CPPFLAGS)
AC_SUBST(SDS_CLIENT_LIBS)

AC_SUBST(SDS_SERVER_CFLAGS)
AC_SUBST(SDS_SERVER_CPPFLAGS)
AC_SUBST(SDS_SERVER_LIBS)

SDS_CLIENT_CPPFLAGS="-fPIC"
SDS_CLIENT_CFLAGS="-fPIC"
SDS_CLIENT_LIBS=""

SDS_SERVER_CPPFLAGS="-fPIC"
SDS_SERVER_CFLAGS="-fPIC"
SDS_SERVER_LIBS=""

 
EXTRA_CXXFLAGS="-std=c++11";
CXXFLAGS="$CXXFLAGS -std=c++11";

##########################################################
# By default, not using HDF5_VOL but using normal HDF5   #
# This will be applied to SDS Server                     # 
########################################################## 
AC_SUBST(HDF5_VOL)		
HDF5_VOL="no"; 
AC_ARG_ENABLE(hdf5vol, 
              [AC_HELP_STRING([--enable-hdf5vol], [enable HDF5 VOL (default: no) requiring special VOL version ])],
	       [if test "$enableval" != "no"; then
                  HDF5_VOL="yes";
		  CPPFLAGS="${CPPFLAGS} -D_HDF5_VOL -DSDS_HDF5_VOL";
	    	  CXXFLAGS="${CXXFLAGS} -D_HDF5_VOL -DSDS_HDF5_VOL";
	    	  CFLAGS="${CFLAGS}     -D_HDF5_VOL -DSDS_HDF5_VOL";
	       else
	    	  HDF5_VOL="no";
	      fi])
AM_CONDITIONAL([HAVE_HDF5_VOL], [test x$HDF5_VOL = xyes])


##########################################################
# By default, we enable MPI at SDS Client                #
# This flag has no impact on SDS Server                  # 
########################################################## 
#AC_SUBST(SDS_CLIENT_MPI)		
SDS_CLIENT_MPI_ENABLED="yes"; 
AC_ARG_ENABLE(clientmpi, 
              [AC_HELP_STRING([--enable-clientmpi], [enable SDS Client with MPI ])],
	       [if test x"$enableval" == x"no"; then
	          AC_MSG_NOTICE([SDS Client is configured as NON-MPI version !])
                  SDS_CLIENT_MPI_ENABLED="no";
	       else
	          AC_MSG_NOTICE([SDS Client is configured as MPI version !])
	       	  SDS_CLIENT_MPI_ENABLED="yes";
	      fi])
##echo $SDS_CLIENT_MPI_ENABLED
if test x"$SDS_CLIENT_MPI_ENABLED" = x"yes"; then 
   CPPFLAGS="${CPPFLAGS}  -DSDS_CLIENT_MPI";
   CXXFLAGS="${CXXFLAGS}  -DSDS_CLIENT_MPI";
   CFLAGS="${CFLAGS}      -DSDS_CLIENT_MPI";
fi

AM_CONDITIONAL([HAVE_HDF5_CLIENT_MPI], [test x$SDS_CLIENT_MPI_ENABLED = xyes])


#########################################################
## By default, this configure files compile SDS Client. #
## Using "configure --enalbe-server" to change it       #
#########################################################

AC_SUBST(SDS_SERVER_ENABLED)
SDS_SERVER_ENABLED=no
AC_ARG_ENABLE([server], 
[AC_HELP_STRING([--enable-server],[enable SDS server, you need specify libevent, hdf5, protobuf])],
  [SDS_SERVER_ENABLED=true],
  [SDS_SERVER_ENABLED=false])
AM_CONDITIONAL([HAVE_SERVER], [test x$SDS_SERVER_ENABLED = xtrue])
##echo $HAVE_SERVER

################################################################################# 
## Checking packages for both SDS Client and SDS Server starts                  #
## It means that these packages are required by both SDS Client and SDS Server  #
#################################################################################

## Check HDF5
AC_ARG_WITH([hdf5],
[AS_HELP_STRING([--with-hdf5=DIR],
[use DIR as the root directory for HDF5 include and lib ])],
[HDF5_HOME="$withval"],
[AC_MSG_ERROR([HDF5 should be specified using --withhdf5 option])]
)
AC_MSG_CHECKING([for HDF5 at $HDF5_HOME])
if test -d "$HDF5_HOME"   && test -d "$HDF5_HOME/include"  && test -d "$HDF5_HOME/lib"; then
SDS_CLIENT_CFLAGS="$SDS_CLIENT_CFLAGS       -I$HDF5_HOME/include"
SDS_CLIENT_CPPFLAGS="$SDS_CLIENT_CPPFLAGS   -I$HDF5_HOME/include"
SDS_CLIENT_LIBS="$SDS_CLIENT_CFLAGS         -L$HDF5_HOME/lib -lhdf5 -lhdf5_hl -lz"
SDS_SERVER_CFLAGS="$SDS_SERVER_CFLAGS       -I$HDF5_HOME/include"
SDS_SERVER_CPPFLAGS="$SDS_SERVER_CPPFLAGS   -I$HDF5_HOME/include"
SDS_SERVER_LIBS="$SDS_SERVER_LIBS           -L$HDF5_HOME/lib -lhdf5 -lhdf5_hl -lz"
AC_MSG_RESULT([yes])	    
else
AC_MSG_ERROR([no: HDF5 directory not found!])
fi

## Check protobuf 
AC_ARG_WITH([protobuf],
[AS_HELP_STRING([--with-protobuf=DIR],
[use DIR as the root directory for protobuf include and lib ])],
[
PROTOBUF_HOME="$withval"
SDS_CLIENT_CFLAGS="$SDS_CLIENT_CFLAGS     -I$PROTOBUF_HOME/include"
SDS_CLIENT_CPPFLAGS="$SDS_CLIENT_CPPFLAGS   -I$PROTOBUF_HOME/include"
SDS_CLIENT_LIBS="$SDS_CLIENT_CFLAGS     -L$PROTOBUF_HOME/lib -lprotobuf-c"
SDS_SERVER_CFLAGS="$SDS_SERVER_CFLAGS     -I$PROTOBUF_HOME/include"
SDS_SERVER_CPPFLAGS="$SDS_SERVER_CPPFLAGS   -I$PROTOBUF_HOME/include"
SDS_SERVER_LIBS="$SDS_SERVER_LIBS       -L$PROTOBUF_HOME/lib -lprotobuf-c"
],
[AC_MSG_ERROR([libevent should be specified using --with-protobuf option])])

## Check protobuf-c
AC_ARG_WITH([protobufc],
[AS_HELP_STRING([--with-protobufc=DIR],
[use DIR as the root directory for protobuf-c include and lib ])],
[PROTOBUFC_HOME="$withval"],
[AC_MSG_ERROR([protobufc should be specified using --with-protobufc option])])
AC_MSG_CHECKING([for protobufc at $PROTOBUFC_HOME])
if test -d "$PROTOBUFC_HOME"   && test -d "$PROTOBUFC_HOME/include"  && test -d "$PROTOBUFC_HOME/lib" && test -d "$PROTOBUFC_HOME/bin" ; then
   AC_ARG_VAR([PROTOBUFC], [protobuf compiler command])
   ##AC_ARG_PROG(PROTOBUFC, protobuf-c, yes, missing -- the GCC Gnu C compile, path=$PROTOBUFC_HOME/bin)	
   AC_CHECK_PROG(PROTOBUFC, [$PROTOBUFC_HOME/bin/protoc-c], [$PROTOBUFC_HOME/bin/protoc-c], [$PROTOBUFC_HOME/bin/protoc-c])
   SDS_CLIENT_CFLAGS="$SDS_CLIENT_CFLAGS     -I$PROTOBUFC_HOME/include"
   SDS_CLIENT_CPPFLAGS="$SDS_CLIENT_CPPFLAGS   -I$PROTOBUFC_HOME/include"
   SDS_CLIENT_LIBS="$SDS_CLIENT_CFLAGS     -L$PROTOBUFC_HOME/lib -lprotobuf-c"
   SDS_SERVER_CFLAGS="$SDS_SERVER_CFLAGS     -I$PROTOBUFC_HOME/include"
   SDS_SERVER_CPPFLAGS="$SDS_SERVER_CPPFLAGS   -I$PROTOBUFC_HOME/include"
   SDS_SERVER_LIBS="$SDS_SERVER_LIBS       -L$PROTOBUFC_HOME/lib -lprotobuf-c"
else
AC_MSG_ERROR([no: protobuf-c directory not found!])
fi

##Check fastquery
AC_ARG_WITH([fastquery],
[AS_HELP_STRING([--with-fastquery=DIR],
[use DIR as the root directory for FastQuery include and lib ])],
[FQ_HOME="$withval"],
[AC_MSG_ERROR([FastQuery should be specified using --with-fastquery option])])
AC_MSG_CHECKING([for FastQuery at $FQ_HOME])
if test -d "$FQ_HOME"   && test -d "$FQ_HOME/include"  && test -d "$FQ_HOME/lib"; then
   ##Fastqery might be installed under include/fastqery
   if test -d "$FQ_HOME/include/fastquery"; then
      SDS_SERVER_CFLAGS="$SDS_SERVER_CFLAGS       -I$FQ_HOME/include/fastquery"
      SDS_SERVER_CPPFLAGS="$SDS_SERVER_CPPFLAGS   -I$FQ_HOME/include/fastquery"
      SDS_CLIENT_CFLAGS="$SDS_SERVER_CFLAGS       -I$FQ_HOME/include/fastquery"
      SDS_CLIENT_CPPFLAGS="$SDS_SERVER_CPPFLAGS   -I$FQ_HOME/include/fastquery"
   else
      SDS_SERVER_CFLAGS="$SDS_SERVER_CFLAGS       -I$FQ_HOME/include/"
      SDS_SERVER_CPPFLAGS="$SDS_SERVER_CPPFLAGS   -I$FQ_HOME/include/"
      SDS_CLIENT_CFLAGS="$SDS_SERVER_CFLAGS       -I$FQ_HOME/include/"
      SDS_CLIENT_CPPFLAGS="$SDS_SERVER_CPPFLAGS   -I$FQ_HOME/include/"
   fi
   SDS_CLIENT_LIBS="$SDS_SERVER_LIBS           -L$FQ_HOME/lib -lfastquery"
   SDS_SERVER_LIBS="$SDS_SERVER_LIBS           -L$FQ_HOME/lib -lfastquery"
AC_MSG_RESULT([yes])	    
else
AC_MSG_ERROR([no: FastQuery directory not found!])
fi

##Check fastbit
AC_ARG_WITH([fastbit],
[AS_HELP_STRING([--with-fastbit=DIR],
[use DIR as the root directory for FastBit include and lib ])],
[FB_HOME="$withval"],
[AC_MSG_ERROR([FastBit should be specified using --with-fastbit option])])

AC_MSG_CHECKING([for FastBit at $FB_HOME])
if test -d "$FB_HOME"   && test -d "$FB_HOME/include"  && test -d "$FB_HOME/lib"; then
SDS_SERVER_CFLAGS="$SDS_SERVER_CFLAGS       -I$FB_HOME/include"
SDS_SERVER_CPPFLAGS="$SDS_SERVER_CPPFLAGS   -I$FB_HOME/include"
SDS_SERVER_LIBS="$SDS_SERVER_LIBS           -L$FB_HOME/lib -lfastbit"
SDS_CLIENT_CFLAGS="$SDS_SERVER_CFLAGS       -I$FB_HOME/include"
SDS_CLIENT_CPPFLAGS="$SDS_SERVER_CPPFLAGS   -I$FB_HOME/include"
SDS_CLIENT_LIBS="$SDS_SERVER_LIBS           -L$FB_HOME/lib -lfastbit"
AC_MSG_RESULT([yes])	    
else
AC_MSG_ERROR([no: FastBit directory not found!])
fi


## Check MPI
## To support no-mpi version of SDS Client, we need this check
if test x"$SDS_CLIENT_MPI_ENABLED" = x"yes"; then 
TRY_MPI="no"
AC_ARG_WITH([mpi],
[AS_HELP_STRING([--with-mpi=DIR],
	[use DIR as the root directory for MPICH include and lib ])],
	[MPI_HOME="$withval"
	TRY_MPI="yes"],
[AC_MSG_ERROR([MPICH is not specified using --withmpi option, using default])
        TRY_MPI="no"])
AC_MSG_CHECKING([for MPICH at $MPI_HOME])
if test "$TRY_MPI" = "yes"; then 
if test -d "$MPI_HOME"   && test -d "$MPI_HOME/include"  && test -d "$MPI_HOME/lib"; then
SDS_CLIENT_CFLAGS="$SDS_CLIENT_CFLAGS     -I$MPI_HOME/include"
SDS_CLIENT_CPPFLAGS="$SDS_CLIENT_CPPFLAGS   -I$MPI_HOME/include"
SDS_CLIENT_LIBS="$SDS_CLIENT_CFLAGS     -L$MPI_HOME/lib -lmpich"
SDS_SERVER_CFLAGS="$SDS_SERVER_CFLAGS     -I$MPI_HOME/include"
SDS_SERVER_CPPFLAGS="$SDS_SERVER_CPPFLAGS   -I$MPI_HOME/include"
SDS_SERVER_LIBS="$SDS_SERVER_LIBS       -L$MPI_HOME/lib -lmpich"
AC_MSG_RESULT([yes])	    
else
AC_MSG_ERROR([no: MPICH directory not found!])
fi
fi
fi

################################################################### 
## Starting from here, checking the packages for SDS Server  #
################################################################### 
if test x$SDS_SERVER_ENABLED = xtrue; then 
AC_MSG_NOTICE([SDS Server is enabled. Chenk Libevent and Berkeley DB Library, ...])
AC_ARG_WITH(libevent,
AC_HELP_STRING([--with-libevent=DIR],[usr DIR as the root direcory for libevent include and lib]),
[LIBEVENT_HOME="$withval"],
[AC_MSG_ERROR([libevent should be specified using --with-libevent option])])

AC_MSG_CHECKING([for libevent at $LIBEVENT_HOME])
if test -d "$LIBEVENT_HOME"   && test -d "$LIBEVENT_HOME/include"  && test -d "$LIBEVENT_HOME/lib" ; then
SDS_SERVER_CPPFLAGS="$SDS_SERVER_CPPFLAGS -I$LIBEVENT_HOME/include"
SDS_SERVER_CFLAGS="$SDS_SERVER_CFLAGS   -I$LIBEVENT_HOME/include"
SDS_SERVER_LIBS="$SDS_SERVER_LIBS     -L$LIBEVENT_HOME/lib  -levent_pthreads -levent -levent_extra -lpthread" # -lrt
AC_MSG_RESULT([yes])	    
else
AC_MSG_ERROR([no: libevent directory not found!])
fi

AC_ARG_WITH([db],
[AS_HELP_STRING([--with-db=DIR],
[use DIR as the root directory for Berkeley DB include and lib ])],
[DB_HOME="$withval"],
[AC_MSG_ERROR([Berkeley DB should be specified using --withdb option])])

AC_MSG_CHECKING([for Berkeley DB at $DB_HOME])
if test -d "$DB_HOME"   && test -d "$DB_HOME/include"  && test -d "$DB_HOME/lib"; then
SDS_SERVER_CFLAGS="$SDS_SERVER_CFLAGS     -I$DB_HOME/include"
SDS_SERVER_CPPFLAGS="$SDS_SERVER_CPPFLAGS   -I$DB_HOME/include"
SDS_SERVER_LIBS="$SDS_SERVER_LIBS       -L$DB_HOME/lib -ldb"
AC_MSG_RESULT([yes])	    
else
AC_MSG_ERROR([no: Berkeley DB directory not found!])
fi
fi 
###########################################  
## The end of SDS Server packages testing #
###########################################

AC_MSG_CHECKING(whether to enable debug)
AC_ARG_ENABLE(debug,
              [AC_HELP_STRING([--enable-debug], [turn on debugging (default: set by autoconf)])],
              [if test "$enableval" = "yes" ; then debug="yes" ; else debug="no" ; fi],
	      [debug=""])
AC_MSG_RESULT($debug)

AC_MSG_CHECKING(whether to enable extra optimization)
AC_ARG_ENABLE(xopt,
              [AC_HELP_STRING([--enable-xopt], [turn on extra optimization (default: no, leave the default CXXFLAGS alone)])],
              [if test "$enableval" = "yes" ; then xopt="yes" ; else xopt="no" ; fi],
	      [xopt="no"])
AC_MSG_RESULT($xopt)
if test "x$debug" = "xyes" ; then
  if test `echo $CFLAGS | fgrep -- '-g' | wc -l` -eq 0 ; then
    CFLAGS="$CFLAGS -g"
  fi
  if test `echo $CXXFLAGS | fgrep -- '-g' | wc -l` -eq 0 ; then
    CXXFLAGS="$CXXFLAGS -g"
  fi
elif test "x$debug" = "xno" ; then
  if test `echo $CFLAGS | fgrep -- '-g' | wc -l` -gt 0 ; then
    OIFS="$IFS"
    IFS=' '
    newflags=""
    for g in $CFLAGS ; do if test "x$g" != "x-g[0123]" -a "x$g" != "x-g" ; then newflags="$newflags $g"; fi; done
    CFLAGS="$newflags"
    IFS=$OIFS
  fi
  if test `echo $CXXFLAGS | fgrep -- '-g' | wc -l` -gt 0 ; then
    OIFS="$IFS"
    IFS=' '
    newflags=""
    for g in $CXXFLAGS ; do if test "x$g" != "x-g[0123]" -a "x$g" != "x-g" ; then newflags="$newflags $g"; fi; done
    CXXFLAGS="$newflags"
    IFS=$OIFS
  fi
fi
if test "x$xopt" = "xyes" ; then
   # add -O5 if the compiler accepts it, otherwise -O3
   OLDCXXFLAGS="$CXXFLAGS";
   CXXFLAGS="$CXXFLAGS -O5";
   AC_COMPILE_IFELSE([
      AC_LANG_PROGRAM([[
        #include <cmath>
        ]], [[
        return (1 == static_cast<int>(std::sqrt(2.0)));
        ]])], [], [CXXFLAGS="$OLDCXXFLAGS -O3"])
fi
if test "x$xopt" = "xno" ; then
  if test `echo $CFLAGS | fgrep -- '-O' | wc -l` -gt 0 ; then
    OIFS=$IFS
    IFS=' '
    newflags=""
    for g in $CFLAGS ; do if test "x$g" != "x-O" -a "x$g" != "x-O2" -a "x$g" != "x-O3" ; then newflags="$newflags $g"; fi; done
    CFLAGS=$newflags
    IFS=$OIFS
  fi
  if test `echo $CXXFLAGS | fgrep -- '-O' | wc -l` -gt 0 ; then
    OIFS=$IFS
    IFS=' '
    newflags=""
    for g in $CXXFLAGS ; do if test "x$g" != "x-O" -a "x$g" != "x-O2" -a "x$g" != "x-O3" ; then newflags="$newflags $g"; fi; done
    CXXFLAGS=$newflags
    IFS=$OIFS
  fi
fi

AC_CONFIG_FILES([Makefile
                 doc/Makefile
                 man/Makefile
                 src/Makefile
                 src/client/Makefile
                 src/server/Makefile
                 src/common/Makefile
                 src/service/Makefile
                 test/Makefile])
AC_OUTPUT
