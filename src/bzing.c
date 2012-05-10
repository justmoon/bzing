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

#include "api/bzing_db.h"
#include "api/bzing_util.h"
#include "bzing_chain.h"
#include "bzing_parser.h"
#include "bzing.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// mapping spent bitset position to value
const uint64_t bz_txi_spent_map[] = {
  0x8000000000000000,
  0x4000000000000000,
  0x2000000000000000,
  0x1000000000000000,
  0x0800000000000000,
  0x0400000000000000,
  0x0200000000000000,
  0x0100000000000000
};

// define which engines support cursors
const bool bz_cursor_support[256] = {
  0, // NONE
  1, // KHASH
  1, // ALIGN
  0, // LMC
  0, // TC
  0, // KC
  0  // BDB
};

#define BZ_SPENT_INIT_SIZE 4096
#define BZ_SPENT_GROW_FACTOR 2

bzing_handle
bzing_alloc(void)
{
  bzing_handle hnd = NULL;
  int result;

#ifdef BZ_ENGINE_BDB
  DB *dbp;
  uint32_t flags;
#endif

#ifdef BZ_ENGINE_LDB
  char *ldb_err = NULL;
#endif

  hnd = malloc(sizeof(bzing_handle_t));
  //hnd->inv = alignhash_init_inv();
  hnd->engine_id = BZ_EID_DEFAULT;
  hnd->use_cursors = bz_cursor_support[hnd->engine_id];
  hnd->spent_size = BZ_SPENT_INIT_SIZE; // TODO
  hnd->spent_data = malloc(hnd->spent_size);
  // the spent offset mustn't be zero, otherwise a transaction might look like
  // a block
  hnd->spent_len = 1;

  switch (hnd->engine_id) {
  case BZ_EID_NONE:
    break;
#ifdef BZ_ENGINE_KHASH
  case BZ_EID_KHASH:
    hnd->kh_inv = kh_init(256);
    break;
#endif
#ifdef BZ_ENGINE_ALIGN
  case BZ_EID_ALIGN:
    hnd->ah_inv = alignhash_init_inv();
    break;
#endif
#ifdef BZ_ENGINE_LMC
  case BZ_EID_LMC:
    hnd->lmc_inv = local_memcache_create("inv", 0, 512000, 0, &hnd->lmc_error);
    if (!hnd->lmc_inv) {
      fprintf(stderr, "Couldn't create localmemcache: %s\n",
              (char *) &hnd->lmc_error.error_str);
      return NULL;
    }
    break;
#endif
#ifdef BZ_ENGINE_TC
  case BZ_EID_TC:
    hnd->tc_inv = tchdbnew();
    if (!tchdbopen(hnd->tc_inv, "inv.tch", HDBOWRITER | HDBOCREAT)) {
      result = tchdbecode(hnd->tc_inv);
      fprintf(stderr, "open error: %s\n", tchdberrmsg(result));
    }
    break;
#endif
#ifdef BZ_ENGINE_KC
  case BZ_EID_KC:
    hnd->kc_inv = kcdbnew();
    if (!kcdbopen(hnd->kc_inv, "inv.kch", KCOWRITER | KCOCREATE)) {
      fprintf(stderr, "open error: %s\n", kcecodename(kcdbecode(hnd->kc_inv)));
    }
    break;
#endif
#ifdef BZ_ENGINE_BDB
  case BZ_EID_BDB:
    result = db_create(&dbp, NULL, 0);
    if (result != 0) {
      fprintf(stderr, "open error: %s\n", db_strerror(result));
      return NULL;
    }

    flags = DB_CREATE;

    result = dbp->open(dbp,          // DB pointer
                       NULL,         // Transaction pointer
                       "inv.db",     // Database file
                       NULL,         // Database name (optional)
                       DB_HASH,      // Database access method
                       flags,        // Open flags
                       0);           // File mode (using defaults)
    if (result != 0) {

    }
    hnd->bdb_inv = dbp;
    break;
#endif
#ifdef BZ_ENGINE_LDB
  case BZ_EID_LDB:
    hnd->ldb_options = leveldb_options_create();
    leveldb_options_set_create_if_missing(hnd->ldb_options, 1);
    hnd->ldb_woptions = leveldb_writeoptions_create();
    hnd->ldb_roptions = leveldb_readoptions_create();
    hnd->ldb_inv = leveldb_open(hnd->ldb_options, "inv.leveldb", &ldb_err);
    if (ldb_err != NULL) {
      fprintf(stderr, "open error: %s\n", ldb_err);
      return NULL;
    }
    break;
#endif
  default:
    // TODO: error
    break;
  }
  return hnd;
}

void
bzing_free(bzing_handle hnd)
{
  if (!hnd) return;

  switch (hnd->engine_id) {
#ifdef BZ_ENGINE_KHASH
  case BZ_EID_KHASH:
    kh_destroy(256, hnd->kh_inv);
    break;
#endif
#ifdef BZ_ENGINE_ALIGN
  case BZ_EID_ALIGN:
    alignhash_destroy_inv(hnd->ah_inv);
    break;
#endif
#ifdef BZ_ENGINE_LMC
  case BZ_EID_LMC:
    local_memcache_free(hnd->lmc_inv, &hnd->lmc_error);
    break;
#endif
#ifdef BZ_ENGINE_TC
  case BZ_EID_TC:
    tchdbdel(hnd->tc_inv);
    break;
#endif
#ifdef BZ_ENGINE_KC
  case BZ_EID_KC:
    kcdbdel(hnd->kc_inv);
    break;
#endif
#ifdef BZ_ENGINE_BDB
  case BZ_EID_BDB:
    if (hnd->bdb_inv != NULL) {
      hnd->bdb_inv->close(hnd->bdb_inv, 0);
    }
    break;
#endif
#ifdef BZ_ENGINE_LDB
  case BZ_EID_LDB:
    leveldb_close(hnd->ldb_inv);
    break;
#endif
  default:
    // TODO: error
    break;
  }
  free(hnd);
}

void
bzing_reset(bzing_handle hnd)
{
  switch (hnd->engine_id) {
#ifdef BZ_ENGINE_KHASH
  case BZ_EID_KHASH:
    kh_clear(256, hnd->kh_inv);
    break;
#endif
#ifdef BZ_ENGINE_ALIGN
  case BZ_EID_ALIGN:
    alignhash_clear_inv(hnd->ah_inv);
    break;
#endif
#ifdef BZ_ENGINE_LMC
  case BZ_EID_LMC:
    local_memcache_drop_namespace("main", 0, 0, &hnd->lmc_error);
    break;
#endif
#ifdef BZ_ENGINE_TC
  case BZ_EID_TC:
    tchdbvanish(hnd->tc_inv);
    break;
#endif
#ifdef BZ_ENGINE_KC
  case BZ_EID_KC:
    kcdbclear(hnd->kc_inv);
    break;
#endif
#ifdef BZ_ENGINE_BDB
  case BZ_EID_BDB:
    hnd->bdb_inv->truncate(hnd->bdb_inv, NULL, NULL, 0);
    break;
#endif
#ifdef BZ_ENGINE_LDB
  case BZ_EID_LDB:
    // TODO
    break;
#endif
  default:
    // TODO: error
    break;
  }
}

uint64_t
bzing_spent_reserve(bzing_handle hnd, uint64_t count)
{
  uint64_t pos = hnd->spent_len;

  hnd->spent_len += count;

  // resize necessary?
  if ((hnd->spent_len * sizeof(uint64_t)) > hnd->spent_size) {
    printf("Spent resize %lu\n", hnd->spent_size*BZ_SPENT_GROW_FACTOR);
    // double the size of the allocated memory
    hnd->spent_data = realloc(hnd->spent_data, hnd->spent_size*BZ_SPENT_GROW_FACTOR);
    // zero out the newly allocated half
    memset(hnd->spent_data + hnd->spent_size, 0, hnd->spent_size*(BZ_SPENT_GROW_FACTOR-1));
    // update the size information
    hnd->spent_size *= BZ_SPENT_GROW_FACTOR;
  }

  return pos;
}

#ifdef BZ_ENGINE_BDB
  DBT bdb_key, bdb_data;
#endif

void
bzing_inv_set(bzing_handle hnd, bz_uint256_t *hash, bz_inv_t *data)
{
  int result;
#ifdef BZ_ENGINE_KHASH
  khiter_t kh_iter;
#endif
#ifdef BZ_ENGINE_ALIGN
  ah_iter_t ah_iter;
#endif

  switch (hnd->engine_id) {
#ifdef BZ_ENGINE_KHASH
  case BZ_EID_KHASH:
    kh_iter = kh_put(256, hnd->kh_inv, *hash, &result);
    kh_val(hnd->kh_inv, kh_iter) = *data;
    break;
#endif
#ifdef BZ_ENGINE_ALIGN
  case BZ_EID_ALIGN:
    // TODO: Calculate hash
    ah_iter = alignhash_set(inv, hnd->ah_inv, hash->d64[0], &result);
    alignhash_value(hnd->ah_inv, ah_iter) = *data;
    break;
#endif
#ifdef BZ_ENGINE_LMC
  case BZ_EID_LMC:
    local_memcache_set(hnd->lmc_inv,
                       (char *) hash->d8, 32,
                       (char *) data, sizeof(bz_inv_t));
    break;
#endif
#ifdef BZ_ENGINE_TC
  case BZ_EID_TC:
    tchdbput(hnd->tc_inv,
             hash->d8, 32,
             (char *) data, sizeof(bz_inv_t));
    break;
#endif
#ifdef BZ_ENGINE_KC
  case BZ_EID_KC:
    kcdbset(hnd->kc_inv,
            (char *) hash->d8, 32,
            (char *) data, sizeof(bz_inv_t));
    break;
#endif
#ifdef BZ_ENGINE_BDB
  case BZ_EID_BDB:
    memset(&bdb_key, 0, sizeof(DBT));
    memset(&bdb_data, 0, sizeof(DBT));
    bdb_key.data = hash->d8;
    bdb_key.size = 32;
    bdb_data.data = (char *) data;
    bdb_data.size = sizeof(bz_inv_t);

    result = hnd->bdb_inv->put(hnd->bdb_inv, NULL, &bdb_key, &bdb_data, 0);
    break;
#endif
#ifdef BZ_ENGINE_LDB
  case BZ_EID_LDB:
    leveldb_put(hnd->ldb_inv, hnd->ldb_woptions,
                (char *) hash->d8, 32,
                (char *) data, sizeof(bz_inv_t),
                &hnd->ldb_err);
    break;
#endif
  default:
    break;
  }
}

bz_inv
bzing_inv_get(bzing_handle hnd, bz_uint256_t *hash)
{
  int result;
  size_t sp;

  bz_inv inv = NULL;
#ifdef BZ_ENGINE_KHASH
  khiter_t kh_iter;
#endif
#ifdef BZ_ENGINE_ALIGN
  ah_iter_t ah_iter;
#endif

  switch (hnd->engine_id) {
#ifdef BZ_ENGINE_KHASH
  case BZ_EID_KHASH:
    kh_iter = kh_put(256, hnd->kh_inv, *hash, &result);
    inv = malloc(sizeof(bz_inv_t));
    *inv = kh_val(hnd->kh_inv, kh_iter);
    break;
#endif
#ifdef BZ_ENGINE_ALIGN
  case BZ_EID_ALIGN:
    // TODO: Calculate hash
    ah_iter = alignhash_set(inv, hnd->ah_inv, hash->d64[0], &result);
    inv = malloc(sizeof(bz_inv_t));
    *inv = alignhash_value(hnd->ah_inv, ah_iter);
    break;
#endif
#ifdef BZ_ENGINE_LMC
  case BZ_EID_LMC:
    local_memcache_get_new(hnd->lmc_inv,
                           (char *) hash->d8, 32, &sp);
    break;
#endif
#ifdef BZ_ENGINE_TC
  case BZ_EID_TC:
    inv = tchdbget(hnd->tc_inv,
                   hash->d8, 32, &result);
    break;
#endif
#ifdef BZ_ENGINE_KC
  case BZ_EID_KC:
    inv = (bz_inv) kcdbget(hnd->kc_inv,
                           (char *) hash->d8, 32, &sp);
    break;
#endif
#ifdef BZ_ENGINE_BDB
  case BZ_EID_BDB:
    memset(&bdb_key, 0, sizeof(DBT));
    memset(&bdb_data, 0, sizeof(DBT));
    bdb_key.data = hash->d8;
    bdb_key.size = 32;
    bdb_data.flags = DB_DBT_MALLOC;

    result = hnd->bdb_inv->get(hnd->bdb_inv, NULL, &bdb_key, &bdb_data, 0);
    inv = bdb_data.data;
    break;
#endif
#ifdef BZ_ENGINE_LDB
  case BZ_EID_LDB:
    inv = (bz_inv) leveldb_get(hnd->ldb_inv, hnd->ldb_roptions,
                               (char *) hash->d8, 32,
                               &sp,
                               &hnd->ldb_err);
    break;
#endif
  default:
    break;
  }

  return inv;
}

bz_cursor
bzing_inv_cursor_new(bzing_handle hnd)
{
  bz_cursor c = (bz_cursor) malloc(sizeof(bz_cursor_t));

  c->hnd = hnd;

  switch (hnd->engine_id) {
#ifdef BZ_ENGINE_KC
  case BZ_EID_KC:
    c->u.kc_cursor = kcdbcursor(hnd->kc_inv);
    break;
#endif
  default:
    break;
  }

  return c;
}

void
bzing_inv_cursor_find(bz_cursor c, bz_uint256_t *hash)
{
  int result;

  switch (c->hnd->engine_id) {
#ifdef BZ_ENGINE_KHASH
  case BZ_EID_KHASH:
    c->u.kh_iter = kh_put(256, c->hnd->kh_inv, *hash, &result);
    break;
#endif
#ifdef BZ_ENGINE_ALIGN
  case BZ_EID_ALIGN:
    // TODO: Calculate hash
    c->u.ah_iter = alignhash_set(inv, c->hnd->ah_inv, hash->d64[0], &result);
    break;
#endif
#ifdef BZ_ENGINE_KC
  case BZ_EID_KC:
    kccurjumpkey(c->u.kc_cursor, (char *) hash, 32);
    break;
#endif
  default:
    break;
  }
}

bz_inv
bzing_inv_cursor_get(bz_cursor c)
{
  size_t sp;
  bz_inv data = NULL;

  switch (c->hnd->engine_id) {
#ifdef BZ_ENGINE_KHASH
  case BZ_EID_KHASH:
    data = malloc(sizeof(bz_inv_t));
    *data = kh_val(c->hnd->kh_inv, c->u.kh_iter);
    break;
#endif
#ifdef BZ_ENGINE_ALIGN
  case BZ_EID_ALIGN:
    data = malloc(sizeof(bz_inv_t));
    *data = alignhash_value(c->hnd->ah_inv, c->u.ah_iter);
    break;
#endif
#ifdef BZ_ENGINE_KC
  case BZ_EID_KC:
    data = (bz_inv) kccurgetvalue(c->u.kc_cursor, &sp, false);
    break;
#endif
  default:
    break;
  }

  return data;
}

void
bzing_inv_cursor_set(bz_cursor c, bz_inv data)
{
  switch (c->hnd->engine_id) {
#ifdef BZ_ENGINE_KHASH
  case BZ_EID_KHASH:
    kh_val(c->hnd->kh_inv, c->u.kh_iter) = *data;
    break;
#endif
#ifdef BZ_ENGINE_ALIGN
  case BZ_EID_ALIGN:
    alignhash_value(c->hnd->ah_inv, c->u.ah_iter) = *data;
    break;
#endif
#ifdef BZ_ENGINE_KC
  case BZ_EID_KC:
    kccursetvalue(c->u.kc_cursor, (char *) data, sizeof(bz_inv_t), false);
    break;
#endif
  default:
    break;
  }
}

void
bzing_inv_cursor_free(bz_cursor c)
{
  free(c);
}

void
bzing_inv_data_free(bzing_handle hnd, void *data)
{
  switch (hnd->engine_id) {
#ifdef BZ_ENGINE_KC
  case BZ_EID_KC:
    kcfree(data);
    break;
#endif
  default:
    free(data);
    break;
  }
}

uint64_t
bzing_spent_mark(bzing_handle hnd, const uint8_t *outpoint, uint64_t offset)
{
  bz_cursor prev = NULL;
  bz_inv prev_inv;
  uint32_t n_txout, i_txout;
  uint64_t spent_offset, spent_pos, *spent_slot;
  bool inv_dirty = false;

  if (hnd->use_cursors) {
    prev = bzing_inv_cursor_new(hnd);
    bzing_inv_cursor_find(prev, (bz_uint256_t *)outpoint);
    prev_inv = bzing_inv_cursor_get(prev);
  } else {
    prev_inv = bzing_inv_get(hnd, (bz_uint256_t *)outpoint);
  }

  // index of the spent output
  i_txout = *((uint32_t *) (outpoint+32));

  if (prev_inv->spent == UINT64_MAX) {
    // TODO: error, transaction tried to spend a block
    return 0;
  } else if ((prev_inv->spent & BZ_TXI_SPENT_UMARK) == BZ_TXI_SPENT_UMARK) {
    n_txout = prev_inv->spent & BZ_TXI_SPENT_UMASK;
    if (n_txout == 0) {
      // TODO: error, previous transaction has no outputs
      return 0;
    } else if (n_txout < i_txout) {
      // TODO: error, previous transaction has too few outputs
      // Note: We do not catch this error if the spent map for the previous
      //       transaction is already initialized, this is simply the cost
      //       of saving every bit of memory we can. However, this inconsistency
      //       is of course detected when a transaction is fully verified.
    }

    // reserve space in spent map
    spent_pos = bzing_spent_reserve(hnd, n_txout);

    if (spent_pos & ~BZ_TXI_SPENT_MASK) {
      // TODO: error, spent map overflow
    }

    prev_inv->spent = spent_pos;
    inv_dirty = true;
  } else {
    spent_pos = prev_inv->spent & BZ_TXI_SPENT_MASK;
  }

  // set quick spent index bits
  if (i_txout < BZ_TXI_SPENT_BITS) {
    prev_inv->spent |= bz_txi_spent_map[i_txout];
    inv_dirty = true;
  }

  // actual position depends on the index of the output
  spent_pos += i_txout;
  spent_offset = spent_pos * sizeof(uint64_t);
  spent_slot = (uint64_t *) (hnd->spent_data + spent_offset);

  *spent_slot = offset;

  if (inv_dirty) {
    if (hnd->use_cursors) {
      bzing_inv_cursor_set(prev, prev_inv);
      bzing_inv_cursor_free(prev);
    } else {
      bzing_inv_set(hnd, (bz_uint256_t *)outpoint, prev_inv);
    }
  }

  bzing_inv_data_free(hnd, prev_inv);

  return spent_offset;
}

void
bzing_block_add(bzing_handle hnd, uint32_t blk_no, uint64_t outer_offset,
                const uint8_t *data, size_t max_len, size_t *actual_len)
{
  int i;
  uint64_t n_tx, n_txin, n_txout, script_len, offset = 80, tx_start;
  bz_inv_t inv;
  bz_uint256_t block_hash, *tx_hashes = NULL, merkle_root;

  double_sha256(data, 80, &block_hash);

  /*kstring_t k_block_hash;
  k_block_hash.l = sizeof(bz_uint256_t);
  k_block_hash.m = kroundup32(k_block_hash.l);
  k_block_hash.s = (char *) &k_block_hash;
  kh_put(bin, hnd->inv, &k_block_hash, &result);*/

  // create index entry for block
  inv.offset = outer_offset + offset;
  inv.spent = UINT64_MAX;
  bzing_inv_set(hnd, &block_hash, &inv);

  //print_uint256(&block_hash);

  n_tx = parse_var_int(data, &offset);
  if (n_tx > 0) {
    tx_hashes = malloc(sizeof(bz_uint256_t) * n_tx);
  }
  for (i = 0; i < n_tx; i++) {
    tx_start = offset;

    // skip version
    offset += 4;

    // parse inputs
    n_txin = parse_var_int(data, &offset);
    while (n_txin > 0) {
      if (i != 0) {
        bzing_spent_mark(hnd, data + offset, outer_offset + tx_start);
      }
      offset += 36;
      script_len = parse_var_int(data, &offset);
      offset += script_len + 4;
      n_txin--;
    }

    // parse outputs
    n_txout = parse_var_int(data, &offset);
    if (n_txout > UINT32_MAX) {
      // TODO: Error
    }
    inv.spent = BZ_TXI_SPENT_UMARK | (n_txout & BZ_TXI_SPENT_UMASK);
    while (n_txout > 0) {
      offset += 8;
      script_len = parse_var_int(data, &offset);
      offset += script_len;
      n_txout--;
    }

    // skip lock_time
    offset += 4;

    // calculate tx hash
    double_sha256(data+tx_start, offset-tx_start, &tx_hashes[i]);

    // create index entry for transaction
    inv.offset = outer_offset + tx_start;
    bzing_inv_set(hnd, &tx_hashes[i], &inv);
  }
  calc_merkle_root(tx_hashes, n_tx, &merkle_root);
  if (0 != memcmp(merkle_root.d8, data+36, sizeof(bz_uint256_t))) {
    printf("Invalid merkle root!\n");
    // TODO: Handle error
  }
  if (n_tx > 0) {
    free(tx_hashes);
  }
  *actual_len = offset;
  //printf("Block size: %d\n", *actual_len);
}

void
bzing_index_regen(bzing_handle hnd,
                  const uint8_t *data, size_t len)
{
  size_t block_len;
  uint32_t n_blocks = 0;
  uint64_t offset = 0;

  while (offset < (len-1)) {
    offset += 8;
    n_blocks++;
    printf("Block #%lu %llu\n", (long unsigned int) n_blocks, (long long unsigned int) offset);
    bzing_block_add(hnd, n_blocks, offset, data + offset, len - offset, &block_len);
    offset += block_len;
  }
}
