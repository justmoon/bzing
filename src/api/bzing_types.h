/*
 * Copyright (c) 2012 Stefan Thomas <justmoon@members.fsf.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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

// -----------------------------------------------------------------------------

// inventory index entry (tx or block)
struct bz_inv
{
  uint64_t offset;
  uint64_t spent;
};

typedef struct bz_inv bz_inv_t;

#define BZ_INV_TX      1
#define BZ_INV_BLOCK   2

// high byte used as a bitset
#define BZ_TXI_SPENT_BITS  8
// lower 15 bytes are used for the offset
#define BZ_TXI_SPENT_MASK   0x00ffffffffffffff
// maximum unambiguous offset
#define BZ_TXI_SPENT_MAX    0x00fffffeffffffff
// marker for uninitialized spend values
#define BZ_TXI_SPENT_UMARK  0x00ffffff00000000
// mask for n_txout in uninitialized spend values
#define BZ_TXI_SPENT_UMASK  0x00000000ffffffff

#endif
