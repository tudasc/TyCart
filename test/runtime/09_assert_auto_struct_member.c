// RUN: %run-tycart %s
// XFAIL: *

#include "../../lib/runtime/TycartAssert.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct {
  int data;
  double x[10];
} DataStruct;

int main() {
  const int size   = 10;
  DataStruct* data = (DataStruct*)malloc(sizeof(DataStruct) * size);

  {
    // addr(data.data) == addr(data), strict mode demands data_stub == DataStruct
    int* data_stub = NULL;
    tycart_assert_auto_stub_((const void*)&data[0].data, data_stub, 22);
  }

  return 0;
}
