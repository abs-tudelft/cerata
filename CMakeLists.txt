cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(cerata VERSION 0.0.10 LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(cmake-modules
  GIT_REPOSITORY  https://github.com/abs-tudelft/cmake-modules.git
  GIT_TAG         master
  )
FetchContent_MakeAvailable(cmake-modules)

include(CompileUnits)

FetchContent_Declare(putong
  GIT_REPOSITORY  https://github.com/abs-tudelft/putong
  GIT_TAG         master
  )
FetchContent_MakeAvailable(putong)

option(BUILD_CERATA_TESTS "Build cerata tests" OFF)
set(BUILD_TESTS ${BUILD_CERATA_TESTS})

add_compile_unit(
  NAME cerata::core
  PRPS
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
  SRCS
    src/cerata/domain.cc
    src/cerata/edge.cc
    src/cerata/expression.cc
    src/cerata/flattype.cc
    src/cerata/graph.cc
    src/cerata/literal.cc
    src/cerata/node.cc
    src/cerata/array.cc
    src/cerata/object.cc
    src/cerata/output.cc
    src/cerata/parameter.cc
    src/cerata/pool.cc
    src/cerata/port.cc
    src/cerata/signal.cc
    src/cerata/transform.cc
    src/cerata/type.cc
    src/cerata/stream.cc
    src/cerata/utils.cc
  TSTS
    test/cerata/test_expressions.cc
    test/cerata/test_pool.cc
    test/cerata/test_types.cc
    test/cerata/test_nodes.cc
    test/cerata/test_graphs.cc
  DEPS
    putong
)

set(CERATA_MODULES)

# Dot back-end
if(BUILD_CERATA_DOT)
  add_compile_unit(
    NAME cerata::dot
    PRPS
      CXX_STANDARD 17
      CXX_STANDARD_REQUIRED ON
    SRCS
      src/cerata/dot/dot.cc
      src/cerata/dot/style.cc
    TSTS
      test/cerata/dot/test_dot.cc
    DEPS
      cerata::core
  )

  list(APPEND CERATA_MODULES "cerata::dot")
endif()

# VHDL back-end
if(BUILD_CERATA_VHDL)
  add_compile_unit(
    NAME cerata::vhdl
    PRPS
      CXX_STANDARD 17
      CXX_STANDARD_REQUIRED ON
    SRCS
      src/cerata/vhdl/architecture.cc
      src/cerata/vhdl/block.cc
      src/cerata/vhdl/declaration.cc
      src/cerata/vhdl/design.cc
      src/cerata/vhdl/identifier.cc
      src/cerata/vhdl/instantiation.cc
      src/cerata/vhdl/resolve.cc
      src/cerata/vhdl/template.cc
      src/cerata/vhdl/vhdl.cc
      src/cerata/vhdl/vhdl_types.cc
    TSTS
      test/cerata/vhdl/test_declarators.cc
      test/cerata/vhdl/test_designs.cc
      test/cerata/vhdl/test_instantiators.cc
      test/cerata/vhdl/test_assignments.cc
    DEPS
      cerata::core
      cerata::dot
  )

  list(APPEND CERATA_MODULES "cerata::vhdl")
endif()

# YAML back-end
if(BUILD_CERATA_YAML)
  FetchContent_Declare(yaml
    GIT_REPOSITORY  https://github.com/jbeder/yaml-cpp.git
    GIT_TAG         yaml-cpp-0.6.3
  )
  FetchContent_GetProperties(yaml)
  if(NOT yaml_POPULATED)
    FetchContent_Populate(yaml)
    set(YAML_CPP_BUILD_TESTS OFF CACHE INTERNAL "")
    add_subdirectory(${yaml_SOURCE_DIR} ${yaml_BINARY_DIR})
    set_property(TARGET yaml-cpp PROPERTY POSITION_INDEPENDENT_CODE ON)
  endif()

  add_compile_unit(
    NAME cerata::yaml
    PRPS
      CXX_STANDARD 17
      CXX_STANDARD_REQUIRED ON
    SRCS
      src/cerata/yaml/yaml.cc
    TSTS
      test/cerata/yaml/test_yaml.cc
    DEPS
      cerata::core
      yaml-cpp
  )

  list(APPEND CERATA_MODULES "cerata::yaml")
endif()

add_compile_unit(
  NAME cerata
  PRPS
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
  DEPS
    cerata::core
    ${CERATA_MODULES}
)

compile_units()

configure_file(src/cerata/config.h.in cerata_config/config.h)
include_directories(${PROJECT_BINARY_DIR})
