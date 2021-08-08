// RUN: rm types.yaml
// RUN: %cpp-to-llvm %s | %apply-typeart | %apply-tycart -S | FileCheck %s

#include "TycartUtil.h"

struct DataStruct {
  double d;
  float f;
};

struct DataStruct* bar() {
  DataStruct* data = new DataStruct;
  return data;
}

void foo(const void* data) {
  try {
    DataStruct* typeart_type_id_placeholder = nullptr;
    tycart_assert_stub_(data, typeart_type_id_placeholder, 1, 22);
  } catch (...) {
  }
}

// CHECK-NOT: Error
// CHECK: invoke void @tycart_assert_(i32 22, i8* %{{[0-9]+}}, i64 1, i64 16, i32 256)
