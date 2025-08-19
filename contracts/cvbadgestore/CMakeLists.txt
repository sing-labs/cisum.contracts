add_contract(cvbadgestore cvbadgestore ${CMAKE_CURRENT_SOURCE_DIR}/src/cvbadgestore.cpp)

target_include_directories(cvbadgestore
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(cvbadgestore
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/cvbadgestore.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/cvbadgestore.contracts.md @ONLY )

target_compile_options( cvbadgestore PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
