add_contract(ops.cisum ops.cisum ${CMAKE_CURRENT_SOURCE_DIR}/src/ops.cisum.cpp)

target_include_directories(ops.cisum
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(ops.cisum
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/ops.cisum.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/ops.cisum.contracts.md @ONLY )

target_compile_options( ops.cisum PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
target_link_libraries(ops.cisum flon_base)

