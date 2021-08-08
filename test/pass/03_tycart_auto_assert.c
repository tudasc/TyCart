// RUN: echo --- > types.yaml
// RUN: %c-to-llvm %s | %apply-tycart -S | FileCheck %s

#include "TycartUtil.h"

void foo(const void* data) {
  double* typeart_type_id_placeholder = NULL;
  tycart_assert_auto_stub_(data, typeart_type_id_placeholder, 22);
}

// CHECK-NOT: Error
// CHECK: tycart_assert_auto_(i32 22, i8* %{{[0-9]+}}, i64 8, i32 6)
