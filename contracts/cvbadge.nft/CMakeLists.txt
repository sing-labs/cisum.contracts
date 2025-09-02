add_contract(cvbadge.nft cvbadge.nft ${CMAKE_CURRENT_SOURCE_DIR}/src/cvbadge.nft.cpp)

target_include_directories(cvbadge.nft
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(cvbadge.nft
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/cvbadge.nft.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/cvbadge.nft.contracts.md @ONLY )

target_compile_options( cvbadge.nft PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
target_link_libraries(cvbadge.nft flon_base)