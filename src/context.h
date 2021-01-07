/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#ifndef __LITECO_CONTEXT_H__
#define __LITECO_CONTEXT_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*
 * x86_64:
 *  [0010]: R8
 *  [0020]: R9
 *  [0030]: R10
 *  [0040]: R11
 *  [0050]: R12
 *  [0060]: R13
 *  [0070]: R14
 *  [0100]: R15
 *  [0110]: RDI
 *  [0120]: RSI
 *  [0130]: RBP
 *  [0140]: RBX
 *  [0150]: RDX
 *  [0160]: RAX
 *  [0170]: RCX
 *  [0200]: RSP
 *  [0210]: func_addr
 *
 */
typedef uint8_t liteco_context_t[256];

void liteco_context_init(liteco_context_t *const context, uint8_t *const st, size_t st_size, void (*fn) (void *const), void *const args);
void liteco_context_swap(liteco_context_t const from, liteco_context_t const to);
bool liteco_cas(uint32_t *const ptr, const uint32_t old, const uint32_t next);
void liteco_cas_yield(uint32_t n);

#endif
