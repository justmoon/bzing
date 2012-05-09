# Copyright (c) 2012 Stefan Thomas <justmoon@members.fsf.org>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

SET (SRCS bzing.c bzing_parser.c util.c)
SET (HDRS bzing_chain.h bzing_parser.h)
SET (PUB_HDRS api/bzing_db.h api/bzing_common.h api/bzing_types.h api/bzing_util.h)

# Ensure defined when building bzing (as opposed to using it from
# another project). Used to ensure correct function export when
# building win32 DLL.
ADD_DEFINITIONS(-DBZING_BUILD)

# set up some paths
SET (libDir ${CMAKE_CURRENT_BINARY_DIR}/../${BZING_DIST_NAME}/lib)
SET (incDir ${CMAKE_CURRENT_BINARY_DIR}/../${BZING_DIST_NAME}/include/bzing)
SET (shareDir ${CMAKE_CURRENT_BINARY_DIR}/../${BZING_DIST_NAME}/share/pkgconfig)

# set the output path for libraries
SET(LIBRARY_OUTPUT_PATH ${libDir})

ADD_LIBRARY(bzing_s STATIC ${SRCS} ${HDRS} ${PUB_HDRS})

ADD_LIBRARY(bzing SHARED ${SRCS} ${HDRS} ${PUB_HDRS})

# setup shared library version number
SET_TARGET_PROPERTIES(bzing PROPERTIES
                      DEFINE_SYMBOL BZING_SHARED
                      SOVERSION ${BZING_MAJOR}
                      VERSION ${BZING_MAJOR}.${BZING_MINOR}.${BZING_MICRO})

# OpenSSL is required
FIND_PACKAGE(OpenSSL REQUIRED)

SET (LIBS lmc pthread rt crypto tokyocabinet db kyotocabinet)

TARGET_LINK_LIBRARIES(bzing ${LIBS})
TARGET_LINK_LIBRARIES(bzing_s ${LIBS})

# create some directories
FILE(MAKE_DIRECTORY ${libDir})
FILE(MAKE_DIRECTORY ${incDir})

# generate build-time source
SET(dollar $)
CONFIGURE_FILE(api/bzing_version.h.cmake ${incDir}/bzing_version.h)
CONFIGURE_FILE(bzing.pc.cmake ${shareDir}/bzing.pc)

# copy public headers to output directory
FOREACH (header ${PUB_HDRS})
  SET (header ${CMAKE_CURRENT_SOURCE_DIR}/${header})

  EXEC_PROGRAM(${CMAKE_COMMAND} ARGS -E copy_if_different ${header} ${incDir})

  ADD_CUSTOM_COMMAND(TARGET bzing_s POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${header} ${incDir})
ENDFOREACH (header ${PUB_HDRS})

INCLUDE_DIRECTORIES(${incDir}/..)

IF(NOT WIN32)
  # at build time you may specify the cmake variable LIB_SUFFIX to handle
  # 64-bit systems which use 'lib64'
  INSTALL(TARGETS bzing LIBRARY DESTINATION lib${LIB_SUFFIX})
  INSTALL(TARGETS bzing_s ARCHIVE DESTINATION lib${LIB_SUFFIX})
  INSTALL(FILES ${PUB_HDRS} DESTINATION include/bzing)
  INSTALL(FILES ${incDir}/bzing_version.h DESTINATION include/bzing)
  INSTALL(FILES ${shareDir}/bzing.pc DESTINATION share/pkgconfig)
ENDIF()
