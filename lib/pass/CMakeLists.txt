set(PASS_SOURCES
  TyCartPass.cpp
)

add_llvm_pass_plugin(tycart_TransformPass
  "${PASS_SOURCES}"
)

mark_as_advanced(LLVM_TRANSFORMPASS_LINK_INTO_TOOLS)

target_define_file_basename(tycart_TransformPass)
target_compile_definitions(tycart_TransformPass
  PRIVATE LOG_LEVEL=${LOG_LEVEL}
          LLVM_ENABLE_STATS
)

set_target_properties(
  tycart_TransformPass
  PROPERTIES
  OUTPUT_NAME "${PROJECT_NAME}TransformPass"
  EXPORT_NAME "TransformPass"
)
add_library(tycart::TransformPass ALIAS tycart_TransformPass)

target_compile_definitions(tycart_TransformPass
  PRIVATE
    LLVM_VERSION=10
    LLVM_ENABLE_STATS
)

target_include_directories(tycart_TransformPass
  SYSTEM
  PRIVATE
  ${LLVM_INCLUDE_DIRS}
)

target_include_directories(tycart_TransformPass
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
         ${PROJECT_SOURCE_DIR}/lib/
)

target_link_libraries(tycart_TransformPass PRIVATE typeart::TypeGenStatic)