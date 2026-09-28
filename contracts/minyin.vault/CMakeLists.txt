add_contract(minyin.vault minyin.vault ${CMAKE_CURRENT_SOURCE_DIR}/src/minyin.vault.cpp)

target_include_directories(minyin.vault
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(minyin.vault
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/minyin.vault.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/minyin.vault.contracts.md @ONLY )

target_compile_options( minyin.vault PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
target_link_libraries(minyin.vault flon_base)
