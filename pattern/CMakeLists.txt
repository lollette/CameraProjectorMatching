project(pattern)
file(GLOB patterns_hdrs RELATIVE ${CMAKE_SOURCE_DIR}/pattern *.h *.hpp)
add_library(pattern ${patterns_hdrs})
