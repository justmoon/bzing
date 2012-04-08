#include <inttypes.h>

#ifndef __BZING_TYPES_H__
#define __BZING_TYPES_H__

/** binary-safe string type */
#ifndef KSTRING_T
#define KSTRING_T kstring_t
typedef struct __kstring_t {
	size_t l, m;
	char *s;
} kstring_t;
#endif

/** 256-bit integer type */
typedef union __bz_uint256
{
  uint8_t  d8[32];
  uint32_t d32[8];
  uint64_t d64[4];
} bz_uint256_t;

#endif
