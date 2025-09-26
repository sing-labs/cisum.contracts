add_contract(sing.token sing.token ${CMAKE_CURRENT_SOURCE_DIR}/src/sing.token.cpp)

target_include_directories(sing.token
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(sing.token
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/sing.token.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/sing.token.contracts.md @ONLY )

target_compile_options( sing.token PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
