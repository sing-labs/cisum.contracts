add_contract(sbt.cvnft sbt.cvnft ${CMAKE_CURRENT_SOURCE_DIR}/src/sbt.cvnft.cpp)

target_include_directories(sbt.cvnft
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   ${CMAKE_CURRENT_SOURCE_DIR}/include/sbt.cvnft
   )

set_target_properties(sbt.cvnft
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/sbt.cvnft.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/sbt.cvnft.contracts.md @ONLY )

target_compile_options( sbt.cvnft PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
