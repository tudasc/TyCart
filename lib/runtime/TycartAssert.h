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

void tycart_assert_stub_(const void* pointer, void* tycart_stub_ptr, size_t count, int checkpoint_id);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // TYCART_TYCARTASSERT_H
