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

#ifndef TYCART_SUPPORT_TYPE_UTIL_H
#define TYCART_SUPPORT_TYPE_UTIL_H

namespace llvm {
class DataLayout;
class Type;
}  // namespace llvm

namespace tycart::util {

unsigned getTypeSizeInBytes(llvm::Type* t, const llvm::DataLayout& dl);

}  // namespace tycart::util

#endif  // TYCART_SUPPORT_TYPE_UTIL_H
