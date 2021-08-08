include(CMakeDependentOption)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
include(FeatureSummary)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE STRING "" FORCE)
include(FetchContent)

find_package(LLVM 10 REQUIRED CONFIG)
set_package_properties(LLVM PROPERTIES
  URL https://llvm.org/
  TYPE REQUIRED
  PURPOSE
  "LLVM framework installation required to compile (and apply) TypeART and TyCart."
)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

set(MPI_INTERCEPT_LIB OFF)
set(ENABLE_MPI_WRAPPER OFF)
FetchContent_Declare(
  typeart
  GIT_REPOSITORY https://github.com/tudasc/TypeART.git
  GIT_TAG devel
  GIT_SHALLOW 1
)
FetchContent_MakeAvailable(typeart)

option(TYCART_TEST_CONFIGURE_IDE "Add targets so the IDE (e.g., Clion) can interpret test files better" ON)
mark_as_advanced(TYCART_TEST_CONFIGURE_IDE)

if(NOT CMAKE_BUILD_TYPE)
  # set default build type
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
  message(STATUS "Building as debug (default)")
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  # set default install path
  set(CMAKE_INSTALL_PREFIX
      "${tycart_SOURCE_DIR}/install/tycart"
      CACHE PATH "Default install path" FORCE
  )
  message(STATUS "Installing to (default): ${CMAKE_INSTALL_PREFIX}")
endif()
