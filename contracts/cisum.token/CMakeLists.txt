add_contract(cisum.token cisum.token ${CMAKE_CURRENT_SOURCE_DIR}/src/cisum.token.cpp)

target_include_directories(cisum.token
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(cisum.token
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/cisum.token.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/cisum.token.contracts.md @ONLY )

target_compile_options( cisum.token PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
