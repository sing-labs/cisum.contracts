add_contract(nestar.cisum nestar.cisum ${CMAKE_CURRENT_SOURCE_DIR}/src/nestar.cisum.cpp)

target_include_directories(nestar.cisum
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(nestar.cisum
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/nestar.cisum.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/nestar.cisum.contracts.md @ONLY )

target_compile_options( nestar.cisum PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
