add_contract(cisumreserve cisumreserve ${CMAKE_CURRENT_SOURCE_DIR}/src/cisumreserve.cpp)

target_include_directories(cisumreserve
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(cisumreserve
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/cisumreserve.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/cisumreserve.contracts.md @ONLY )

target_compile_options( cisumreserve PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
target_link_libraries(cisumreserve flon_base)
