add_contract(poh.cisum poh.cisum ${CMAKE_CURRENT_SOURCE_DIR}/src/poh.cisum.cpp)

target_include_directories(poh.cisum
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(poh.cisum
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/poh.cisum.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/poh.cisum.contracts.md @ONLY )

target_compile_options( poh.cisum PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
target_link_libraries(poh.cisum flon_base)
