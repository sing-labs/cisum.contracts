add_contract(pop.cisum pop.cisum ${CMAKE_CURRENT_SOURCE_DIR}/src/pop.cisum.cpp)

target_include_directories(pop.cisum
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(pop.cisum
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/pop.cisum.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/pop.cisum.contracts.md @ONLY )

target_compile_options( pop.cisum PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
target_link_libraries(pop.cisum flon_base)
