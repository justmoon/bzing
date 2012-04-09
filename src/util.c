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

#include "api/bzing_util.h"
#include "openssl/sha.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void
double_sha256(const uint8_t *data, size_t len, bz_uint256_t *hash)
{
  SHA256_CTX ctx;

  SHA256_Init(&ctx);
  SHA256_Update(&ctx, data, len);
  SHA256_Final(hash->d8, &ctx);
  SHA256_Init(&ctx);
  SHA256_Update(&ctx, hash->d8, SHA256_DIGEST_LENGTH);
  SHA256_Final(hash->d8, &ctx);
}

size_t
calc_merkle_size(size_t leaf_count)
{
  size_t len = 0, i;
  for (i = leaf_count; i > 1; i = (i+1)>>1) {
    len += i;
  }
  return len >= 2 ? ++len : 1;
}

void
calc_merkle_root(bz_uint256_t *tx_hashes, size_t n_tx, bz_uint256_t *result)
{
  int i, i2, j = 0, p, s;
  size_t n_nodes;
  bz_uint256_t *tree;
  SHA256_CTX ctx;

  n_nodes = calc_merkle_size(n_tx);
  tree = malloc(sizeof(bz_uint256_t) * n_nodes);

  memcpy(tree, tx_hashes, sizeof(bz_uint256_t) * n_tx);

  j = 0; p = n_tx;
  for (s = n_tx; s > 1; s = (s + 1) / 2) {
    for (i = 0; i < s; i += 2) {
      i2 = MIN(i+1, s-1);
      SHA256_Init(&ctx);
      SHA256_Update(&ctx, tree[j+i].d8, sizeof(bz_uint256_t));
      SHA256_Update(&ctx, tree[j+i2].d8, sizeof(bz_uint256_t));
      SHA256_Final(tree[p].d8, &ctx);
      SHA256_Init(&ctx);
      SHA256_Update(&ctx, tree[p].d8, sizeof(bz_uint256_t));
      SHA256_Final(tree[p].d8, &ctx);
      p++;
    }
    j += s;
  }

  if (p == 0) {
    memset(result->d8, 0, sizeof(bz_uint256_t));
  } else {
    memcpy(result->d8, tree[n_nodes-1].d8, sizeof(bz_uint256_t));
  }
}

void
print_uint256(const bz_uint256_t *hash)
{
  int i;

  for (i = 0; i < 32; i++) {
    printf("%02x", hash->d8[i]);
  }
  printf("\n");
}
