add_contract(cisumairdrop cisumairdrop ${CMAKE_CURRENT_SOURCE_DIR}/src/cisumairdrop.cpp)

target_include_directories(cisumairdrop PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(cisumairdrop PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ricardian/cisumairdrop.contracts.md.in
   ${CMAKE_CURRENT_BINARY_DIR}/ricardian/cisumairdrop.contracts.md @ONLY)
target_compile_options(cisumairdrop PUBLIC
   -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian
   -R${CMAKE_CURRENT_BINARY_DIR}/ricardian)
target_link_libraries(cisumairdrop flon_base)
