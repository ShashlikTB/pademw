project(pademiddleWare)
cmake_minimum_required(VERSION 2.6)

execute_process(COMMAND root-config --cflags OUTPUT_VARIABLE ROOTCFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND root-config --libdir OUTPUT_VARIABLE ROOTLDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND root-config --libs OUTPUT_VARIABLE ROOTLIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
include_directories($ENV{GMOCK_HOME}/include $ENV{GMOCK_HOME}/gtest/include "${PROJECT_SOURCE_DIR}/include" "/home/hurt/tools/gmock")
link_directories($ENV{GMOCK_HOME}/mybuild $ENV{GMOCK_HOME}/gtest/mybuild ${ROOTLDIR} "/home/hurt/tools/gmock/gtest/mybuild")
add_definitions(-std=c++0x -ggdb)
set(CMAKE_CXX_FLAGS "${CMAXE_CXX_FLAGS} -m64 -Wall ${ROOTCFLAGS}")

set(sources 
   src/main.cpp
   src/controller.cpp
   src/pademw.cpp
   tests/pademwTests.cpp)
   
add_executable(test ${sources})
target_link_libraries(test pthread)
target_link_libraries(test gmock)
target_link_libraries(test gtest)
target_link_libraries(test event)
target_link_libraries(test boost_system)
target_link_libraries(test boost_thread)			   

target_link_libraries(test ${ROOTLIBS})
