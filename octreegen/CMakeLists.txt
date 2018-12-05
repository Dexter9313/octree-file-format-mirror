#INIT
cmake_minimum_required(VERSION 3.0.0)

set(PROJECT_NAME octreegen)
set(PROJECT_TEST_NAME tests)
project(${PROJECT_NAME} C CXX)

#GET CMAKE_SYSTEM_NAME VARIABLE
include(../cmake/common/SystemName.cmake)
message(STATUS "Architecture : ${CMAKE_SYSTEM_NAME}")

#SET UP COMPILER
include(../cmake/common/SetupCompiler.cmake)

#DEPENDENCIES
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/../cmake/modules)

find_package(HDF5 REQUIRED)
find_package(Octree REQUIRED)

set(EXTERNAL_LIBS_INCLUDE_DIRS ${HDF5_INCLUDE_DIR} ${OCTREE_INCLUDE_DIR})

#FILES/DIRECTORIES

set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE MAIN_SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)

file(GLOB SRC_FILES ${MAIN_SRC_FILES})
list(REMOVE_ITEM SRC_FILES ${PROJECT_SOURCE_DIR}/src/main.cpp)
file(GLOB MAIN_FILE ${PROJECT_SOURCE_DIR}/src/main.cpp)

file(GLOB_RECURSE TEST_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)


#INCLUDE

set(SYSTEM_INCLUDES ${EXTERNAL_LIBS_INCLUDE_DIRS})
include_directories(${COMMON_INCLUDES} SYSTEM ${SYSTEM_INCLUDES})

#LINKING SETUP
set(LD_LIBS ${HDF5_LIBRARIES} ${OCTREE_LIBRARIES})

#COMMON OBJECTS
add_library(objects OBJECT ${SRC_FILES})

#EXECUTABLE 1 : tests
add_executable(${PROJECT_TEST_NAME} $<TARGET_OBJECTS:objects> ${TEST_FILES})
target_link_libraries(${PROJECT_TEST_NAME} ${LD_LIBS})

#EXECUTABLE 2 : octreegen
add_executable(${PROJECT_NAME} $<TARGET_OBJECTS:objects> ${MAIN_FILE})
target_link_libraries(${PROJECT_NAME} ${LD_LIBS})

enable_testing ()
add_test (NAME Tests COMMAND ${PROJECT_TEST_NAME})

INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} DESTINATION bin)

SET(CPACK_GENERATOR "DEB")
SET(CPACK_PACKAGE_NAME ${PROJECT_NAME})
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "An .octree file generator designed for VIRUP.")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Florian Cabot <florian.cabot@epfl.ch>")
if(TAG)
	SET(CPACK_PACKAGE_VERSION ${TAG})
endif()
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libhdf5-100, liboctree")

INCLUDE(CPack)

#Create uninstall target
include(../cmake/common/Uninstall.cmake)

# Including extra cmake rules
#include(cmake/clang-dev-tools.cmake)
#include(cmake/doc.cmake)
