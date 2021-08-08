// RUN: echo --- > types.yaml
// RUN: %c-to-llvm %s | %apply-tycart -S | FileCheck %s

#include "TycartUtil.h"

void foo(const void* data) {
  double* typeart_type_id_placeholder = NULL;
  tycart_register_FTI_t_stub_(typeart_type_id_placeholder);
}

// CHECK-NOT: Error
// CHECK: tycart_register_FTI_t_(i32 6)
