add_contract(mall.cisum mall.cisum ${CMAKE_CURRENT_SOURCE_DIR}/src/mall.cisum.cpp)
target_include_directories(mall.cisum PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../ticket.cvnft/include)
target_link_libraries(mall.cisum flon_base)
set_target_properties(mall.cisum PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

target_compile_options(mall.cisum PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian)
