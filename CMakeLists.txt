cmake_minimum_required(VERSION 3.0)
project(CameraProjectorMatching)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(OpenCV REQUIRED)
include_directories($(OpenCV_INCLUDE_DIRS))

#add_executable(main main.cpp)
#target_link_libraries(main ${OpenCV_LIBS})

# ----------------------------------------------------------------------------
# Process subdirectories
# ----------------------------------------------------------------------------

include_directories(pattern)

include_directories(utils)
#add_subdirectory(test)

file( GLOB TEST_SOURCES RELATIVE ${CMAKE_SOURCE_DIR}/test test/*.cpp )
foreach( testsourcefile ${TEST_SOURCES} )
    MESSAGE( STATUS "variable is 1 : " ${testsourcefile} )
    # I used a simple string replace, to cut off .cpp.
    string( REPLACE ".cpp" "" testname ${testsourcefile} )
    #MESSAGE( STATUS "variable is 2 : " ${testname} )
    add_executable( ${testname} ${CMAKE_SOURCE_DIR}/test/${testsourcefile} )
    # Make sure YourLib is linked to each app
    target_link_libraries( ${testname} ${OpenCV_LIBS} ${pattern}${utils} gmp)
endforeach( testsourcefile ${TEST_SOURCES} )



