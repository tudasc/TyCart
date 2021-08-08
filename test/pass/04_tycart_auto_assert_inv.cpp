// RUN: echo --- > types.yaml
// RUN: %cpp-to-llvm %s | %apply-tycart -S | FileCheck %s

#include "TycartUtil.h"

void foo(const void* data) {
  try {
    double* typeart_type_id_placeholder = nullptr;
    tycart_assert_auto_stub_(data, typeart_type_id_placeholder, 22);
  } catch (...) {
  }
}

// CHECK-NOT: Error
// CHECK: invoke void @tycart_assert_auto_(i32 22, i8* %{{[0-9]+}}, i64 8, i32 6)
