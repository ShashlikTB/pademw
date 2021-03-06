project(pademiddleWare)
cmake_minimum_required(VERSION 2.6)

execute_process(COMMAND root-config --cflags OUTPUT_VARIABLE ROOTCFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND root-config --libdir OUTPUT_VARIABLE ROOTLDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND root-config --libs OUTPUT_VARIABLE ROOTLIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
include_directories($ENV{GMOCK_HOME}/include $ENV{GMOCK_HOME}/gtest/include "${PROJECT_SOURCE_DIR}/include" )
link_directories($ENV{GMOCK_HOME}/mybuild $ENV{GMOCK_HOME}/gtest/mybuild ${ROOTLDIR} )
find_library(TB_LIBRARY TB ${CMAKE_SOURCE_DIR}/shared)

add_definitions(-std=c++0x -ggdb)
set(CMAKE_CXX_FLAGS "${CMAXE_CXX_FLAGS} -m64 -Wall ${ROOTCFLAGS} ")

set(testsources 
   src/main.cpp
   src/padeBoard.cpp
   src/padeClient.cpp
   src/padeUDPServer.cpp
   tests/pademwTests.cpp)
   
set(standalonesources
	src/padeBoard.cpp
	src/padeClient.cpp
	src/padeUDPServer.cpp
	src/standalone.cpp)	

   
add_executable(test ${testsources} )

target_link_libraries(test pthread)
target_link_libraries(test gmock)
target_link_libraries(test gtest)
target_link_libraries(test event)
target_link_libraries(test boost_system)
target_link_libraries(test boost_thread)			   
target_link_libraries(test boost_regex)			   

target_link_libraries(test ${ROOTLIBS})

add_executable(standalone ${standalonesources} )
target_link_libraries(standalone pthread)
target_link_libraries(standalone event)
target_link_libraries(standalone boost_system)
target_link_libraries(standalone boost_thread)			   
target_link_libraries(standalone boost_regex)			   
target_link_libraries(standalone ${ROOTLIBS})
target_link_libraries(standalone ${TB_LIBRARY})
