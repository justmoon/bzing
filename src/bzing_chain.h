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

#ifndef __BZING_CHAIN_H__
#define __BZING_CHAIN_H__

#include "bzing_engines.h"

struct bzing_handle
{
  uint8_t engine_id; // BZ_EID_*

#ifdef BZ_ENGINE_KHASH
  // index of inventories
  khash_t(256) *kh_inv;
#endif

#ifdef BZ_ENGINE_ALIGN
  // index of inventories
  alignhash_inv_t *ah_inv;
#endif

#ifdef BZ_ENGINE_LMC
  // index of inventories
  local_memcache_t *lmc_inv;
  lmc_error_t lmc_error;
#endif

#ifdef BZ_ENGINE_TC
  TCHDB *tc_inv;
#endif

#ifdef BZ_ENGINE_KC
  KCDB *kc_inv;
#endif

#ifdef BZ_ENGINE_BDB
  DB *bdb_inv;
#endif

  // spent map
  char *spent_data;
  // used length
  uint64_t spent_len;
  // available length (before memory region must be resized)
  uint64_t spent_size;
};

typedef struct bzing_handle bzing_handle_t;

struct __bz_cursor
{
  union {
#ifdef BZ_ENGINE_KHASH
    khiter_t kh_iter;
#endif

#ifdef BZ_ENGINE_ALIGN
    ah_iter_t ah_iter;
#endif
  }
};

typedef struct __bz_cursor bz_cursor_t;

#endif
