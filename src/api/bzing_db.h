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

#include <bzing/bzing_common.h>

#ifndef __BZING_DB_H__
#define __BZING_DB_H__

#ifdef __cplusplus
extern "C" {
#endif

  /** block chain database instance handle */
  typedef struct bzing_handle *bzing_handle;

  /**
   * Allocate a new database handle.
   */
  BZING_API bzing_handle bzing_alloc(void);

  /**
   * Free a database handle.
   */
  BZING_API void bzing_free(bzing_handle handle);

  /**
   * Reset a database.
   */
  BZING_API void bzing_reset(bzing_handle handle);

  /**
   * Add an item to the inventory index.
   */
  BZING_API void bzing_inv_add(bzing_handle hnd,
                               bz_uint256_t hash, bz_inv_t *data);

  /**
   * Add a block to the chain.
   */
  BZING_API void bzing_block_add(bzing_handle handle,
                                 uint32_t blk_no, const uint8_t *data,
                                 size_t max_len, size_t *actual_len);

  /**
   * Regenerate indexes for specific block chain data.
   */
  BZING_API void bzing_index_regen(bzing_handle handle,
                                   const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif
