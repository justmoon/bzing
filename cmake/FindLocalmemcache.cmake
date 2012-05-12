# Find liblocalmemcache.a - key/value storage system

find_path(Localmemcache_INCLUDE_PATH NAMES localmemcache.h)
find_library(Localmemcache_LIBRARY NAMES liblmc.a liblmc.lib)

if(Localmemcache_INCLUDE_PATH AND Localmemcache_LIBRARY)
  set(Localmemcache_FOUND TRUE)
endif(Localmemcache_INCLUDE_PATH AND Localmemcache_LIBRARY)

if(Localmemcache_FOUND)
  if(NOT Localmemcache_FIND_QUIETLY)
    message(STATUS "Found Localmemcache: ${Localmemcache_LIBRARY}")
  endif(NOT Localmemcache_FIND_QUIETLY)
else(Localmemcache_FOUND)
  if(Localmemcache_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find leveldb library.")
  endif(Localmemcache_FIND_REQUIRED)
endif(Localmemcache_FOUND)
