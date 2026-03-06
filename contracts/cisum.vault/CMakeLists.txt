add_contract(cisum.vault cisum.vault ${CMAKE_CURRENT_SOURCE_DIR}/src/cisum.vault.cpp)

target_include_directories(cisum.vault
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(cisum.vault
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/cisum.vault.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/cisum.vault.contracts.md @ONLY )

target_compile_options( cisum.vault PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
target_link_libraries(cisum.vault flon_base)
