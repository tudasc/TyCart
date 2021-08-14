// RUN: %run-tycart %s | FileCheck %s
// XFAIL: *

#include "../../lib/runtime/TycartAssert.h"
#include "stdlib.h"

int main() {
  const int size = 10;
  int* data      = (int*)malloc(sizeof(int) * size);

  {
    double* data_stub = NULL;
    tycart_assert_auto_stub_((const void*)data, data_stub, 22);
  }

  printf("Test succeeded\n");
  return 0;
}
