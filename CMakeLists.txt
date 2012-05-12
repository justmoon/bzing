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


CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(BitcoinZing C)

SET (BZING_MAJOR 0)
SET (BZING_MINOR 1)
SET (BZING_MICRO 0)

SET (BZING_DIST_NAME "bzing-${BZING_MAJOR}.${BZING_MINOR}.${BZING_MICRO}")

IF (NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "Release")
ENDIF (NOT CMAKE_BUILD_TYPE)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
IF(CMAKE_COMPILER_IS_GNUCC)
  INCLUDE(CheckCCompilerFlag)
  CHECK_C_COMPILER_FLAG(-fvisibility=hidden HAVE_GCC_VISIBILITY)
  IF(HAVE_GCC_VISIBILITY)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
  ENDIF(HAVE_GCC_VISIBILITY)
ENDIF(CMAKE_COMPILER_IS_GNUCC)
SET(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -std=c99 -pedantic -Wpointer-arith -Wno-format-y2k -Wstrict-prototypes -Wmissing-declarations -Wnested-externs -Wextra -Wundef -Wwrite-strings -Wold-style-definition -Wredundant-decls -Wno-unused-parameter -Wno-sign-compare -Wmissing-prototypes")

SET(CMAKE_C_FLAGS_DEBUG "-DDEBUG -g")
SET(CMAKE_C_FLAGS_RELEASE "-DNDEBUG -O3 -Wuninitialized")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(test)

# Dummy target for now
ADD_CUSTOM_TARGET(doc)

ADD_CUSTOM_TARGET(test
                  ./run_tests.sh ${CMAKE_CURRENT_BINARY_DIR}/test/bzing_test
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test)
