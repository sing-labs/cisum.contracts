add_contract(nestar.token nestar.token ${CMAKE_CURRENT_SOURCE_DIR}/src/nestar.token.cpp)

target_include_directories(nestar.token
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(nestar.token
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/nestar.token.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/nestar.token.contracts.md @ONLY )

target_compile_options( nestar.token PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
target_link_libraries(nestar.token flon_base)