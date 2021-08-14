// RUN: %run-tycart %s | FileCheck %s

#include "../../lib/runtime/TycartAssert.h"

#include <stdio.h>
#include <stdlib.h>

int main() {
  const int size = 10;
  int* data      = (int*)malloc(sizeof(int) * size);

  {
    int* data_stub = NULL;
    tycart_assert_stub_((const void*)data, data_stub, size, 22);
  }

  printf("Test succeeded\n");
  return 0;
}

// CHECK: Test succeeded
