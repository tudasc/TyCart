// RUN: echo --- > types.yaml
// RUN: %cpp-to-llvm %s | %apply-tycart -S | FileCheck %s

#include "TycartUtil.h"

void foo(const void* data) {
  try {
    double* typeart_type_id_placeholder = nullptr;
    tycart_register_FTI_t_stub_(typeart_type_id_placeholder);
  } catch (...) {
  }
}

// CHECK-NOT: Error
// CHECK: invoke void @tycart_register_FTI_t_(i32 6)
