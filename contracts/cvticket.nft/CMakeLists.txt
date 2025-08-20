add_contract(cvticket.nft cvticket.nft ${CMAKE_CURRENT_SOURCE_DIR}/src/cvticket.nft.cpp)

target_include_directories(cvticket.nft
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(cvticket.nft
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/cvticket.nft.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/cvticket.nft.contracts.md @ONLY )

target_compile_options( cvticket.nft PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
