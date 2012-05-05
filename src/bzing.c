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

#define BZ_SPENT_INIT_SIZE 4096
#define BZ_SPENT_GROW_FACTOR 2

bzing_handle
bzing_alloc(void)
{
  bzing_handle hnd = NULL;
  int result;

#ifdef BZ_ENGINE_BDB
  DB *dbp;
#endif

  hnd = malloc(sizeof(bzing_handle_t));
  //hnd->inv = alignhash_init_inv();
  hnd->engine_id = BZ_EID_DEFAULT;
  hnd->spent_size = BZ_SPENT_INIT_SIZE; // TODO
  hnd->spent_data = malloc(hnd->spent_size);
  // the spent offset mustn't be zero, otherwise a transaction might look like
  // a block
  hnd->spent_len = 1;

  printf("%d %d\n", hnd->engine_id, BZ_EID_DEFAULT);

#ifdef BZ_ENGINE_BDB
  uint32_t flags;
#endif

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
    hnd->align_inv = alignhash_init_inv();
    break;
#endif
#ifdef BZ_ENGINE_LMC
  case BZ_EID_LMC:
    hnd->lmc_inv = local_memcache_create("main", 0, 512000, 0, &hnd->lmc_error);
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
    if (!tchdbopen(hnd->tc_inv, "main.tch", HDBOWRITER | HDBOCREAT)) {
      result = tchdbecode(hnd->tc_inv);
      fprintf(stderr, "open error: %s\n", tchdberrmsg(result));
    }
    break;
#endif
#ifdef BZ_ENGINE_KC
  case BZ_EID_KC:
    hnd->kc_inv = kcdbnew();
    if (!kcdbopen(hnd->kc_inv, "main.kch", KCOWRITER | KCOCREATE)) {
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
                       "main.db",    // Database file
                       NULL,         // Database name (optional)
                       DB_HASH,      // Database access method
                       flags,        // Open flags
                       0);           // File mode (using defaults)
    if (result != 0) {

    }
    hnd->bdb_inv = dbp;
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

  //alignhash_destroy_inv(hnd->inv);
  switch (hnd->engine_id) {
#ifdef BZ_ENGINE_KHASH
  case BZ_EID_KHASH:
    kh_destroy(256, hnd->kh_inv);
    break;
#endif
#ifdef BZ_ENGINE_LMC
  case BZ_EID_LMC:
    local_memcache_free(hnd->lmc_inv, &hnd->lmc_error);
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
#ifdef BZ_ENGINE_KHASH
  if (hnd->engine_id == BZ_EID_KHASH) {
    // TODO
  }
#endif
#ifdef BZ_ENGINE_LMC
  if (hnd->engine_id == BZ_EID_LMC) {
    local_memcache_drop_namespace("main", 0, 0, &hnd->lmc_error);
  }
#endif
}

uint64_t
bzing_spent_reserve(bzing_handle hnd, uint64_t count)
{
  uint64_t pos = hnd->spent_len;

  hnd->spent_len += count;

  // resize necessary?
  if ((hnd->spent_len * sizeof(uint64_t)) > hnd->spent_size) {
    printf("Spent resize %d\n", hnd->spent_size*BZ_SPENT_GROW_FACTOR);
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
bzing_inv_add(bzing_handle hnd,
              bz_uint256_t hash, bz_inv_t *data)
{
  int result;
#ifdef BZ_ENGINE_KHASH
  khiter_t kh_iter;
#endif
#ifdef BZ_ENGINE_ALIGN
  ah_iter_t align_iter;
#endif

  switch (hnd->engine_id) {
#ifdef BZ_ENGINE_KHASH
  case BZ_EID_KHASH:
    kh_iter = kh_put(256, hnd->kh_inv, hash, &result);
    kh_val(hnd->kh_inv, kh_iter) = *data;
    break;
#endif
#ifdef BZ_ENGINE_ALIGN
  case BZ_EID_ALIGN:
    // TODO: Calculate hash
    align_iter = alignhash_set(inv, hnd->align_inv, hash.d64[0], &result);
    alignhash_value(hnd->align_inv, align_iter) = *data;
    break;
#endif
#ifdef BZ_ENGINE_LMC
  case BZ_EID_LMC:
    local_memcache_set(hnd->lmc_inv,
                       (char *) hash.d8, 32,
                       (char *) data, sizeof(bz_inv_t));
    break;
#endif
#ifdef BZ_ENGINE_TC
  case BZ_EID_TC:
    tchdbput(hnd->tc_inv,
             hash.d8, 32,
             (char *) data, sizeof(bz_inv_t));
    break;
#endif
#ifdef BZ_ENGINE_KC
  case BZ_EID_KC:
    kcdbset(hnd->kc_inv,
            (char *) hash.d8, 32,
            (char *) data, sizeof(bz_inv_t));
    break;
#endif
#ifdef BZ_ENGINE_BDB
  case BZ_EID_BDB:
    memset(&bdb_key, 0, sizeof(DBT));
    memset(&bdb_data, 0, sizeof(DBT));
    bdb_key.data = hash.d8;
    bdb_key.size = 32;
    bdb_data.data = (char *) data;
    bdb_data.size = sizeof(bz_inv_t);

    result = hnd->bdb_inv->put(hnd->bdb_inv, NULL, &bdb_key, &bdb_data, 0);
    break;
#endif
  default:
    break;
  }
}

void
bzing_block_add(bzing_handle hnd, uint32_t blk_no,
                const uint8_t *data, size_t max_len, size_t *actual_len)
{
  int i;
  uint64_t n_tx, n_txin, n_txout, script_len, offset = 80, tx_start,
           spent_offset;
  bz_inv_t inv;
  bz_uint256_t block_hash, *tx_hashes = NULL, merkle_root;

  double_sha256(data, 80, &block_hash);

  /*kstring_t k_block_hash;
  k_block_hash.l = sizeof(bz_uint256_t);
  k_block_hash.m = kroundup32(k_block_hash.l);
  k_block_hash.s = (char *) &k_block_hash;
  kh_put(bin, hnd->inv, &k_block_hash, &result);*/

  // create index entry for block
  inv.offset = offset;
  inv.spent = 0;
  bzing_inv_add(hnd, block_hash, &inv);

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
      offset += 36;
      script_len = parse_var_int(data, &offset);
      offset += script_len + 4;
      n_txin--;
    }

    // parse outputs
    n_txout = parse_var_int(data, &offset);

    // reserve space in spent map
    spent_offset = bzing_spent_reserve(hnd, n_txout);

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
    inv.offset = offset;
    inv.spent = spent_offset;
    bzing_inv_add(hnd, tx_hashes[i], &inv);
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
    n_blocks++;
    printf("Block #%lu %llu\n", (long unsigned int) n_blocks, (long long unsigned int) offset);
    bzing_block_add(hnd, n_blocks, data + offset, len - offset, &block_len);
    offset += block_len;
  }
}
