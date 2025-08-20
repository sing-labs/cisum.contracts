add_contract(pos.cisum pos.cisum ${CMAKE_CURRENT_SOURCE_DIR}/src/pos.cisum.cpp)

target_include_directories(pos.cisum
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(pos.cisum
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/pos.cisum.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/pos.cisum.contracts.md @ONLY )

target_compile_options( pos.cisum PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
