# Copyright (C) 2007-2009 LuaDist.
# Created by Peter Kapec <kapecp@gmail.com>
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
#	Note:
#		Searching headers and libraries is very simple and is NOT as powerful as scripts
#		distributed with CMake, because LuaDist defines directories to search for.
#		Everyone is encouraged to contact the author with improvements. Maybe this file
#		becomes part of CMake distribution sometimes.

# - Find TokyoCabinet
# Find the native TokyoCabinet headers and libraries.
#
# TokyoCabinet_INCLUDE_DIRS	- where to find m_apm.h, etc.
# TokyoCabinet_LIBRARIES	- List of libraries when using TokyoCabinet.
# TokyoCabinet_FOUND	- True if TokyoCabinet found.

# Look for the header file.
FIND_PATH(TokyoCabinet_INCLUDE_DIR NAMES tcadb.h)

# Look for the library.
FIND_LIBRARY(TokyoCabinet_LIBRARY NAMES tokyocabinet)

# Handle the QUIETLY and REQUIRED arguments and set TokyoCabinet_FOUND to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TokyoCabinet DEFAULT_MSG TokyoCabinet_LIBRARY TokyoCabinet_INCLUDE_DIR)

# Copy the results to the output variables.
IF(TokyoCabinet_FOUND)
	SET(TokyoCabinet_LIBRARIES ${TokyoCabinet_LIBRARY})
	SET(TokyoCabinet_INCLUDE_DIRS ${TokyoCabinet_INCLUDE_DIR})
ELSE(TokyoCabinet_FOUND)
	SET(TokyoCabinet_LIBRARIES)
	SET(TokyoCabinet_INCLUDE_DIRS)
ENDIF(TokyoCabinet_FOUND)

MARK_AS_ADVANCED(TokyoCabinet_INCLUDE_DIRS TokyoCabinet_LIBRARIES)
