function(configure_tycart_script input output)
  cmake_parse_arguments(
    ARG "" "" "APPLY_MODE" ${ARGN}
  )

  set(TYPEART_SAN_FLAGS "")

  set(TYCART_INCLUDE_DIRS "-I${PROJECT_SOURCE_DIR}/lib/runtime")

  if(ARG_APPLY_MODE)
    set(TYCART_RUN_SCRIPT 0)
  else()
    set(TYCART_RUN_SCRIPT 1)
  endif()

  target_generate_file(${input} ${output})
endfunction()

function(find_clang_progs target names default)
  find_program(
    target-prog
    NAMES ${names}
    HINTS ${LLVM_TOOLS_BINARY_DIR}
    NO_DEFAULT_PATH
  )

  if(NOT target-prog)
    set(${target}
        ${default}
        PARENT_SCOPE
    )
    message(
      STATUS
        "Did not find clang program ${names} in ${LLVM_TOOLS_BINARY_DIR}. Using def. value: ${default}"
    )
  else()
    set(${target}
        ${target-prog}
        PARENT_SCOPE
    )
  endif()
  unset(target-prog CACHE)
endfunction()

find_clang_progs(CLANG_EXEC "clang;clang-10" "clang")
find_clang_progs(CLANGCXX_EXEC "clang++;clang++-10" "clang++")
find_clang_progs(LLC_EXEC "llc;llc-10" "llc")
find_clang_progs(OPT_EXEC "opt;opt-10" "opt")

configure_tycart_script(tycart-tmpl.in run.sh)
