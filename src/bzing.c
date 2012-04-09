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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

bzing_handle
bzing_alloc(void)
{
  bzing_handle hnd = NULL;

  hnd = malloc(sizeof(hnd));
  //hnd->inv = alignhash_init_inv();
#if BZ_DB_ENGINE == BZ_DBE_KHASH
  hnd->inv = kh_init(256);
#elif BZ_DB_ENGINE == BZ_DBE_LMC
  hnd->inv = local_memcache_create("main", 0, 0, 0, &hnd->inv_error);
  if (!hnd->inv) {
    fprintf(stderr, "Couldn't create localmemcache: %s\n",
            (char *) &hnd->inv_error.error_str);
    return NULL;
  }
#endif
  return hnd;
}

void
bzing_free(bzing_handle hnd)
{
  if (!hnd) return;

  //alignhash_destroy_inv(hnd->inv);
#if BZ_DB_ENGINE == BZ_DBE_KHASH
  kh_destroy(256, hnd->inv);
#elif BZ_DB_ENGINE == BZ_DBE_LMC
  local_memcache_free(hnd->inv, &hnd->inv_error);
#endif
  free(hnd);
}

void
bzing_reset(bzing_handle hnd)
{
#if BZ_DB_ENGINE == BZ_DBE_KHASH
  // TODO
#elif BZ_DB_ENGINE == BZ_DBE_LMC
  local_memcache_drop_namespace("main", 0, 0, &hnd->inv_error);
#endif
}

void
bzing_block_add(bzing_handle hnd,
                const uint8_t *data, size_t max_len, size_t *actual_len)
{
  int i;
  uint64_t n_tx, n_txin, n_txout, script_len, offset = 80, tx_start;
  bz_uint256_t block_hash, *tx_hashes = NULL, merkle_root;

  double_sha256(data, 80, &block_hash);

  /* ah_iter_t iter;
  iter = alignhash_set(inv, hnd->inv, block_hash.d64[0], &result);
  alignhash_value(hnd->inv, iter) = offset;*/

  /*kstring_t k_block_hash;
  k_block_hash.l = sizeof(bz_uint256_t);
  k_block_hash.m = kroundup32(k_block_hash.l);
  k_block_hash.s = (char *) &k_block_hash;
  kh_put(bin, hnd->inv, &k_block_hash, &result);*/

#if BZ_DB_ENGINE == BZ_DBE_KHASH
  khiter_t iter;
  int result;
  iter = kh_put(256, hnd->inv, block_hash, &result);
  kh_val(hnd->inv, iter) = offset;
#elif BZ_DB_ENGINE == BZ_DBE_LMC
  local_memcache_set(hnd->inv, (char *) block_hash.d8, 32, (char *) &offset, 8);
#endif

  //print_uint256(&block_hash);

  n_tx = parse_var_int(data, &offset);
  if (n_tx > 0) {
    tx_hashes = malloc(sizeof(bz_uint256_t) * n_tx);
  }
  for (i = 0; i < n_tx; i++) {
    tx_start = offset;
    offset += 4;
    n_txin = parse_var_int(data, &offset);
    while (n_txin > 0) {
      offset += 36;
      script_len = parse_var_int(data, &offset);
      offset += script_len + 4;
      n_txin--;
    }
    n_txout = parse_var_int(data, &offset);
    while (n_txout > 0) {
      offset += 8;
      script_len = parse_var_int(data, &offset);
      offset += script_len;
      n_txout--;
    }
    offset += 4;
    double_sha256(data+tx_start, offset-tx_start, &tx_hashes[i]);

#if BZ_DB_ENGINE == BZ_DBE_KHASH
    iter = kh_put(256, hnd->inv, tx_hashes[i], &result);
    kh_val(hnd->inv, iter) = offset;
#elif BZ_DB_ENGINE == BZ_DBE_LMC
    local_memcache_set(hnd->inv, (char *)tx_hashes[i].d8, 32, (char *) &offset, 8);
#endif
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
  size_t block_len, n_blocks = 0;
  uint64_t offset = 0;

  while (offset < (len-1)) {
    n_blocks++;
    printf("Block #%lu %llu\n", (long unsigned int) n_blocks, (long long unsigned int) offset);
    bzing_block_add(hnd, data + offset, len - offset, &block_len);
    offset += block_len;
  }
}
