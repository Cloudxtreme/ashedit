project(AshEdit2)
cmake_minimum_required(VERSION 2.6)

option(MO2 "Build for Monster RPG 2 format levels" on)

if(NOT ("${USER_INCLUDE_PATH}" STREQUAL ""))
	include_directories(${USER_INCLUDE_PATH})
endif(NOT ("${USER_INCLUDE_PATH}" STREQUAL ""))
if(NOT ("${USER_LIBRARY_PATH}" STREQUAL ""))
	link_directories(${USER_LIBRARY_PATH})
endif(NOT ("${USER_LIBRARY_PATH}" STREQUAL ""))

set(ASHEDIT2_SRC main.cpp general.cpp)

if(WIN32)
	add_executable(ashedit2 WIN32 ${ASHEDIT2_SRC})
else()
	add_executable(ashedit2 ${ASHEDIT2_SRC})
endif()

target_link_libraries(ashedit2 "allegro_monolith" "tgui2")