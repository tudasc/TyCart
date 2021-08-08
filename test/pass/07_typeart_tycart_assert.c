// RUN: rm types.yaml
// RUN: %c-to-llvm %s | %apply-typeart | %apply-tycart -S | FileCheck %s

#include "TycartUtil.h"

#include <stdlib.h>

struct DataStruct {
  double d;
  float f;
};

struct DataStruct* bar() {
  struct DataStruct* data = (struct DataStruct*)malloc(sizeof(struct DataStruct));
  return data;
}

void foo(const void* data) {
  struct DataStruct* typeart_type_id_placeholder = NULL;
  tycart_assert_stub_(data, typeart_type_id_placeholder, 1, 22);
}

// CHECK-NOT: Error
// CHECK: tycart_assert_(i32 22, i8* %{{[0-9]+}}, i64 1, i64 16, i32 256)
