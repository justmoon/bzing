prefix=${CMAKE_INSTALL_PREFIX}
libdir=${dollar}{prefix}/lib${LIB_SUFFIX}
includedir=${dollar}{prefix}/include/bzing

Name: Bitcoin Zing Library
Description: A fast Bitcoin database library in ANSI C
Version: ${BZING_MAJOR}.${BZING_MINOR}.${BZING_MICRO}
Cflags: -I${dollar}{includedir}
Libs: -L${dollar}{libdir} -lbzing
