#! /bin/bash
libtoolize
aclocal
autoheader
automake --force-missing --add-missing --copy 
autoconf
