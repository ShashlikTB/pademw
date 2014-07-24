project(pademiddleWare)
cmake_minimum_required(VERSION 2.6)

FIND_PACKAGE(Qt5Widgets)
FIND_PACKAGE(Qt5Core)




execute_process(COMMAND root-config --cflags OUTPUT_VARIABLE ROOTCFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND root-config --libdir OUTPUT_VARIABLE ROOTLDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND root-config --libs OUTPUT_VARIABLE ROOTLIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
include_directories(${include_directories} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} $ENV{GMOCK_HOME}/include $ENV{GMOCK_HOME}/gtest/include "${PROJECT_SOURCE_DIR}/include" /usr/include/qt5 /usr/include/qt5/QtWidgets /usr/include/qt5/QtCore )
link_directories($ENV{GMOCK_HOME}/mybuild $ENV{GMOCK_HOME}/gtest/mybuild ${ROOTLDIR} )
find_library(TB_LIBRARY TB ${CMAKE_SOURCE_DIR}/shared)

add_definitions(-std=c++0x -ggdb -fPIC  )
set(CMAKE_CXX_FLAGS "${CMAXE_CXX_FLAGS} -m64 -Wall ${ROOTCFLAGS} -DQT_NO_KEYWORDS")


set(uitestsources
	src/padeBoard.cpp
	src/uitest.cpp
	src/form.cpp)


set(padeserversources
	src/padeBoard.cpp
	src/padeServer.cpp
	src/udpServer.cpp
	src/padeServerTest.cpp)	


QT5_WRAP_UI(UISrcs src/form.ui)
QT5_WRAP_CPP(MOCSrcs include/form.h)

add_executable(uitest ${uitestsources} ${MOCSrcs} ${UISrcs} )
target_link_libraries(uitest ${Qt5Widgets_LIBRARIES})
target_link_libraries(uitest pthread)
target_link_libraries(uitest event)
target_link_libraries(uitest boost_system)
target_link_libraries(uitest boost_thread)			   
target_link_libraries(uitest boost_regex)			   
target_link_libraries(uitest ${ROOTLIBS})
target_link_libraries(uitest ${TB_LIBRARY})


add_executable(padeServerTest ${padeserversources} )
target_link_libraries(padeServerTest pthread)
target_link_libraries(padeServerTest event)
target_link_libraries(padeServerTest boost_system)
target_link_libraries(padeServerTest boost_thread)
target_link_libraries(padeServerTest boost_regex)		
target_link_libraries(padeServerTest ${ROOTLIBS})
target_link_libraries(padeServerTest ${TB_LIBRARY})
