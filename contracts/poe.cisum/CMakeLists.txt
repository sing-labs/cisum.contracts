add_contract(poe.cisum poe.cisum ${CMAKE_CURRENT_SOURCE_DIR}/src/poe.cisum.cpp)

target_include_directories(poe.cisum
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(poe.cisum
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/poe.cisum.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/poe.cisum.contracts.md @ONLY )

target_compile_options( poe.cisum PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
target_link_libraries(poe.cisum flon_base)
