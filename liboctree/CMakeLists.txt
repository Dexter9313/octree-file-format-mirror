#INIT
cmake_minimum_required(VERSION 3.0.0)

set(PROJECT_NAME octree)
set(PROJECT_TEST_NAME tests)
project(${PROJECT_NAME} LANGUAGES C CXX VERSION 1.0.0)

#GET CMAKE_SYSTEM_NAME VARIABLE
include(../cmake/common/SystemName.cmake)
message(STATUS "Architecture : ${CMAKE_SYSTEM_NAME}")

#SET UP COMPILER
include(../cmake/common/SetupCompiler.cmake)

#DEPENDENCIES

##NONE

#FILES/DIRECTORIES

set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)

file(GLOB_RECURSE TEST_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)


#INCLUDE

#set(SYSTEM_INCLUDES ${EXTERNAL_LIBS_INCLUDE_DIRS})
include_directories(${COMMON_INCLUDES})# SYSTEM ${SYSTEM_INCLUDES})

#LIBRARY : liboctree
add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION 1)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PROJECT_SOURCE_DIR}/include/Octree.hpp;${PROJECT_SOURCE_DIR}/include/binaryrw.hpp")
target_include_directories(${PROJECT_NAME} PRIVATE include)

#EXECUTABLE 1 : tests
add_executable(${PROJECT_TEST_NAME} ${TEST_FILES})
target_link_libraries(${PROJECT_TEST_NAME} ${PROJECT_NAME})

enable_testing ()
add_test (NAME Tests COMMAND ${PROJECT_TEST_NAME})

include(GNUInstallDirs)

INSTALL(TARGETS ${PROJECT_NAME}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/liboctree)

SET(CPACK_GENERATOR "DEB")
SET(CPACK_PACKAGE_NAME "lib${PROJECT_NAME}")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Library that can build octrees, or read/write them from/to *.octree* files used by virup-prototype.
")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Florian Cabot <florian.cabot@epfl.ch>")
if(TAG)
	SET(CPACK_PACKAGE_VERSION ${TAG})
endif()

INCLUDE(CPack)

#Create uninstall target
include(../cmake/common/Uninstall.cmake)

# Including extra cmake rules
#include(cmake/clang-dev-tools.cmake)

file(GLOB_RECURSE DOCUMENTED_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/include/*.hpp)
include(../cmake/common/Doc.cmake)
