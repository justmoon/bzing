# Copyright (c) 2012 Stefan Thomas <justmoon@members.fsf.org>
# Copyright (c) 2007-2011, Lloyd Hilaiel <lloyd@hilaiel.com>
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

# OpenSSL is required
FIND_PACKAGE(OpenSSL REQUIRED)
link_directories ( ${OpenSSL_LIBRARY_DIRS} )
include_directories ( ${OpenSSL_INCLUDE_DIRS} )
SET (LIBS crypto)

# Backend: khash
add_definitions(-DBZ_ENGINE_KHASH)

# Backend: alignhash
add_definitions(-DBZ_ENGINE_ALIGN)

# Backend: sparsehash
if (WITH_SPARSE)
  message(STATUS "Enabled sparsehash backend")
  add_definitions(-DBZ_ENGINE_SPARSE)
  SET (SRCS ${SRCS} sparsehash/libchash.c)
endif (WITH_SPARSE)

# Backend: Localmemcache
find_package(Localmemcache)
if (Localmemcache_FOUND)
  add_definitions(-DBZ_ENGINE_LMC)
  include_directories ( ${Localmemcache_INCLUDE_DIR} )
  SET (LIBS ${LIBS} lmc pthread rt)
endif (Localmemcache_FOUND)

# Backend: BerkeleyDB
find_package(BerkeleyDB)
if (DB_FOUND)
  add_definitions(-DBZ_ENGINE_BDB)
  include_directories ( ${DB_INCLUDE_DIR} )
  SET (LIBS ${LIBS} ${DB_LIBRARIES})
endif (DB_FOUND)

# Backend: Tokyo Cabinet
find_package(TokyoCabinet)
if (TokyoCabinet_FOUND)
  add_definitions(-DBZ_ENGINE_TC)
  include_directories ( ${TokyoCabinet_INCLUDE_DIRS} )
  SET (LIBS ${LIBS} ${TokyoCabinet_LIBRARIES})
endif (TokyoCabinet_FOUND)

# Backend: Kyoto Cabinet
find_package(KyotoCabinet)
if (KyotoCabinet_FOUND)
  add_definitions(-DBZ_ENGINE_KC)
  include_directories ( ${KyotoCabinet_INCLUDE_PATH} )
  SET (LIBS ${LIBS} ${KyotoCabinet_LIBRARY})
endif (KyotoCabinet_FOUND)

# Backend: LevelDB
find_package(LevelDB)
if (LevelDB_FOUND)
  add_definitions(-DBZ_ENGINE_LDB)
  include_directories ( ${LevelDB_INCLUDE_PATH} )
  # TODO: Detect Snappy and only include if available
  SET (LIBS ${LIBS} leveldb snappy)
endif (LevelDB_FOUND)

ADD_LIBRARY(bzing_s STATIC ${SRCS} ${HDRS} ${PUB_HDRS})

ADD_LIBRARY(bzing SHARED ${SRCS} ${HDRS} ${PUB_HDRS})

# setup shared library version number
SET_TARGET_PROPERTIES(bzing PROPERTIES
                      DEFINE_SYMBOL BZING_SHARED
                      SOVERSION ${BZING_MAJOR}
                      VERSION ${BZING_MAJOR}.${BZING_MINOR}.${BZING_MICRO})


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
