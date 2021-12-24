project(utils)
file(GLOB utils_hdrs RELATIVE ${CMAKE_SOURCE_DIR}/utils *.h *.hpp)
add_library(utils ${utils_hdrs})
