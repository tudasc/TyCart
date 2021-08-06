add_library(tycart_Runtime SHARED Runtime.cpp TycartAssert.h)
set_target_properties(
  tycart_Runtime
  PROPERTIES
  OUTPUT_NAME "${PROJECT_NAME}Runtime"
  EXPORT_NAME "Runtime"
)
add_library(tycart::Runtime ALIAS tycart_Runtime)

target_include_directories(tycart_Runtime
  SYSTEM
  PRIVATE
  ${LLVM_INCLUDE_DIRS}
)

target_include_directories(tycart_Runtime
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
         ${PROJECT_SOURCE_DIR}/lib/
)

target_link_libraries(tycart_Runtime PUBLIC typeart::Runtime)

target_define_file_basename(tycart_Runtime)
