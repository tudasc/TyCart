// TyCart library
//
// Copyright (c) 2021-2021 TyCart Authors
// Distributed under the BSD 3-Clause license.
// (See accompanying file LICENSE or copy at
// https://opensource.org/licenses/BSD-3-Clause)
//
// Project home: https://github.com/tudasc/TyCart
//
// SPDX-License-Identifier: BSD-3-Clause
//

#ifndef TYCART_TYCARTASSERT_H
#define TYCART_TYCARTASSERT_H

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif  // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// Assert functions
void tycart_assert_auto_(int checkpoint_id, const void* pointer, size_t type_size, int type_id);
void tycart_assert_(int checkpoint_id, const void* pointer, size_t count, size_t type_size, int type_id);
void tycart_register_FTI_t_(int type_id);

// Mock or "stub" functions of the corresponding assert functions. They ares replaced by the TyCart LLVM pass.
void tycart_assert_stub_(const void* pointer, void* tycart_stub_ptr, size_t count, int checkpoint_id);  // NOLINT
void tycart_assert_auto_stub_(const void* pointer, void* tycart_stub_ptr, int checkpoint_id);           // NOLINT
void tycart_register_FTI_t_stub_(const void* pointer);                                                  // NOLINT

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // TYCART_TYCARTASSERT_H
