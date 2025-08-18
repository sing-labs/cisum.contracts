add_contract(grab.cisum grab.cisum ${CMAKE_CURRENT_SOURCE_DIR}/src/grab.cisum.cpp)

target_include_directories(grab.cisum
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(grab.cisum
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/grab.cisum.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/grab.cisum.contracts.md @ONLY )

target_compile_options( grab.cisum PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
