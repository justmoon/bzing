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

#ifndef __BZING_ENGINES_H__
#define __BZING_ENGINES_H__

#include "api/bzing_db.h"

#define BZ_ENGINE_NONE
#define BZ_ENGINE_KHASH
#define BZ_ENGINE_ALIGN
#define BZ_ENGINE_LMC
#define BZ_ENGINE_TC
#define BZ_ENGINE_KC
#define BZ_ENGINE_BDB

// Engine IDs
#define BZ_EID_NONE    255
#define BZ_EID_KHASH     1
#define BZ_EID_ALIGN     2
#define BZ_EID_LMC       3
#define BZ_EID_TC        4
#define BZ_EID_KC        5
#define BZ_EID_BDB       6

#define BZ_EID_DEFAULT BZ_EID_KHASH

// Select default engine
// (precedence: KC, TC, LMC, BDB, KHASH)
#ifndef BZ_EID_DEFAULT
#ifdef BZ_ENGINE_KC
#define BZ_EID_DEFAULT BZ_EID_KC
#endif
#endif

#ifndef BZ_EID_DEFAULT
#ifdef BZ_ENGINE_TC
#define BZ_EID_DEFAULT BZ_EID_TC
#endif
#endif

#ifndef BZ_EID_DEFAULT
#ifdef BZ_ENGINE_LMC
#define BZ_EID_DEFAULT BZ_EID_LMC
#endif
#endif

#ifndef BZ_EID_DEFAULT
#ifdef BZ_ENGINE_BDB
#define BZ_EID_DEFAULT BZ_EID_BDB
#endif
#endif

#ifndef BZ_EID_DEFAULT
#ifdef BZ_ENGINE_KHASH
#define BZ_EID_DEFAULT BZ_EID_KHASH
#endif
#endif

#ifndef BZ_EID_DEFAULT
#ifdef BZ_ENGINE_ALIGN
#define BZ_EID_DEFAULT BZ_EID_ALIGN
#endif
#endif

#ifndef BZ_EID_DEFAULT
#error No engines selected for compilation
#endif

//------------------------------------------------------------------------------

#ifdef BZ_ENGINE_ALIGN

#include "ulib/alignhash_tpl.h"

DECLARE_ALIGNHASH(inv, uint64_t, bz_inv_t, 1, alignhash_hashfn, alignhash_equalfn)

#endif

//------------------------------------------------------------------------------

/*
  #include "klib/kstring.h"


static inline khint_t __ac_X31_hash_bin(kstring_t *s)
{
	khint_t h = 0, i = 0;
	if (s->l) for (;i<s->l;i++) h = (h << 5) - h + s->s[i];
	return h;
}
#define kh_bin_hash_func(key) __ac_X31_hash_bin(key)
#define kh_bin_hash_equal(a, b) (a->l == b->l && \
                                 memcmp(a->s, b->s, a->l))


#define KHASH_SET_INIT_BIN(name)										\
	KHASH_INIT(name, kstring_t *, char, 0, kh_bin_hash_func, kh_bin_hash_equal)

#define KHASH_MAP_INIT_BIN(name, khval_t)								\
	KHASH_INIT(name, kstring_t *, khval_t, 1, kh_bin_hash_func, kh_bin_hash_equal)


KHASH_MAP_INIT_BIN(bin, kstring_t *)
*/

//------------------------------------------------------------------------------

#ifdef BZ_ENGINE_KHASH

#include "lookup3.h"
#include "klib/khash.h"

static inline khint_t __ac_X31_hash_bin(kstring_t *s)
{
	khint_t h = 0, i = 0;
	if (s->l) for (;i<s->l;i++) h = (h << 5) - h + s->s[i];
	return h;
}

#define kh_256_hash_func(key) CrapWow((key).d8, 32, 0)
//#define kh_256_hash_func(key) (khint_t) (key).d8[0]
#define kh_256_hash_equal(a, b) ( \
  (a).d64[0] == (b).d64[0] &&     \
  (a).d64[1] == (b).d64[1] &&     \
  (a).d64[2] == (b).d64[2] &&     \
  (a).d64[3] == (b).d64[3] )

#define KHASH_SET_INIT_256(name)										\
	KHASH_INIT(name, bz_uint256_t, char, 0, kh_256_hash_func, kh_256_hash_equal)

#define KHASH_MAP_INIT_256(name, khval_t)								\
	KHASH_INIT(name, bz_uint256_t, khval_t, 1, kh_256_hash_func, kh_256_hash_equal)


KHASH_MAP_INIT_256(256, bz_inv_t)

#endif

//------------------------------------------------------------------------------

#ifdef BZ_ENGINE_LMC

#include <localmemcache.h>

#endif

//------------------------------------------------------------------------------

#ifdef BZ_ENGINE_TC

#include <tcutil.h>
#include <tchdb.h>

#endif

//------------------------------------------------------------------------------

#ifdef BZ_ENGINE_KC

#include <kclangc.h>

#endif

//------------------------------------------------------------------------------

#ifdef BZ_ENGINE_BDB

typedef unsigned int            u_int;
typedef unsigned long           u_long;
#include <db.h>

#endif

//------------------------------------------------------------------------------

#endif
