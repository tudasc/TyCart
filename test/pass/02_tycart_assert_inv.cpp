// RUN: echo --- > types.yaml
// RUN: %c-to-llvm %s | %apply-tycart -S | FileCheck %s

#include "TycartUtil.h"

void foo(const void* data) {
  try {
    double* typeart_type_id_placeholder = NULL;
    tycart_assert_stub_(data, typeart_type_id_placeholder, 1, 22);
  } catch (...) {
  }
}

// CHECK-NOT: Error
// CHECK: invoke void @tycart_assert_(i32 22, i8* %{{[0-9]+}}, i64 1, i64 8, i32 6)