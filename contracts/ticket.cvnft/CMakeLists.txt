add_contract(ticket.cvnft ticket.cvnft ${CMAKE_CURRENT_SOURCE_DIR}/src/ticket.cvnft.cpp)

target_include_directories(ticket.cvnft
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(ticket.cvnft
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/ticket.cvnft.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/ticket.cvnft.contracts.md @ONLY )

target_compile_options( ticket.cvnft PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
target_link_libraries(ticket.cvnft flon_base)