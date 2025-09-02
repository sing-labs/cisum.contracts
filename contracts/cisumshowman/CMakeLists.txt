add_contract(cisumshowman cisumshowman ${CMAKE_CURRENT_SOURCE_DIR}/src/cisumshowman.cpp)

if(DEFINED ENV{DAY_SECONDS_FOR_TEST})
   message(WARNING "ENV{DAY_SECONDS_FOR_TEST}=$ENV{DAY_SECONDS_FOR_TEST} should use only for test!!!")
   target_compile_definitions(cisumshowman PUBLIC "DAY_SECONDS_FOR_TEST=$ENV{DAY_SECONDS_FOR_TEST}")
endif()


target_include_directories(cisumshowman
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(cisumshowman
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/cisumshowman.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/cisumshowman.contracts.md @ONLY )
target_compile_options( cisumshowman PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
target_link_libraries(cisumshowman flon_base)