// RUN: echo --- > types.yaml
// RUN: %c-to-llvm %s | %apply-tycart -S | FileCheck %s

#include "TycartUtil.h"

void foo(const void* data) {
  double* typeart_type_id_placeholder = NULL;
  tycart_assert_stub_(data, typeart_type_id_placeholder, 1, 22);
}

// CHECK-NOT: Error