file(GLOB_RECURSE SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

add_library(flon_base INTERFACE)
target_include_directories(flon_base INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)