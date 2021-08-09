// RUN: rm types.yaml
// RUN: %cpp-to-llvm %s | %apply-typeart | %apply-tycart -S | FileCheck %s
// RUN: %cpp-to-llvm %s | %apply-tycart -S | FileCheck %s

// XFAIL: *
// Fails due to forward declaration of DataStruct

#include "TycartUtil.h"

namespace detail {
class DataStruct;
}

void foo(const void* data) {
  detail::DataStruct* typeart_type_id_placeholder = nullptr;
  tycart_assert_stub_(data, typeart_type_id_placeholder, 1, 22);
}

// CHECK-NOT: Error
// CHECK: tycart_assert_(i32 22, i8* %{{[0-9]+}}, i64 1, i64 16, i32 256)
