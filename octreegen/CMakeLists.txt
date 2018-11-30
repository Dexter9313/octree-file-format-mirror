#INIT
cmake_minimum_required(VERSION 3.0.0)

set(PROJECT_NAME octreegen)
set(PROJECT_TEST_NAME tests)
project(${PROJECT_NAME} C CXX)

if(ARCH)
	set(CMAKE_SYSTEM_NAME ${ARCH})
else()
	execute_process(COMMAND uname -i OUTPUT_VARIABLE CMAKE_SYSTEM_NAME)
	string(REGEX REPLACE "\n$" "" CMAKE_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}")
endif()

#C++11
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=gnu++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
	message(ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

if(MSVC)
	set(MSVC_COMPILER_DEFS "-D_VARIADIC_MAX=10")
endif()

#WARNINGS
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror")
endif()

#DEPENDENCIES
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/modules)

find_package(HDF5 REQUIRED)

set(EXTERNAL_LIBS_INCLUDE_DIRS ${HDF5_INCLUDE_DIR})

#FILES/DIRECTORIES

set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE MAIN_SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
list(REMOVE_ITEM MAIN_SRC_FILES ${PROJECT_SOURCE_DIR}/src/binaryrw.cpp)

file(GLOB SRC_FILES ${MAIN_SRC_FILES})
list(REMOVE_ITEM SRC_FILES ${PROJECT_SOURCE_DIR}/src/main.cpp)
file(GLOB MAIN_FILE ${PROJECT_SOURCE_DIR}/src/main.cpp)

file(GLOB_RECURSE TEST_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)

set(EXT_PROJECT_DIR ${PROJECT_SOURCE_DIR}/ext)


#INCLUDE

set(SYSTEM_INCLUDES ${EXTERNAL_LIBS_INCLUDE_DIRS})
include_directories(${COMMON_INCLUDES} SYSTEM ${SYSTEM_INCLUDES})

#LINKING SETUP
set(LD_LIBS ${HDF5_LIBRARIES})

#COMMON OBJECTS
add_library(objects OBJECT ${SRC_FILES})

#EXECUTABLE 1 : tests
add_executable(${PROJECT_TEST_NAME} $<TARGET_OBJECTS:objects> ${TEST_FILES})
target_link_libraries(${PROJECT_TEST_NAME} ${LD_LIBS})

#EXECUTABLE 2 : octreegen
add_executable(${PROJECTNAME} $<TARGET_OBJECTS:objects> ${MAIN_FILE})
target_link_libraries(${PROJECTNAME} ${LD_LIBS})

enable_testing ()
add_test (NAME Tests COMMAND ${PROJECT_TEST_NAME})

INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECTNAME} DESTINATION bin)

SET(CPACK_GENERATOR "DEB")
SET(CPACK_PACKAGE_NAME ${PROJECTNAME})
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "An .octree file generator designed for VIRUP.")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Florian Cabot <florian.cabot@epfl.ch>")
if(TAG)
	SET(CPACK_PACKAGE_VERSION ${TAG})
endif()
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libhdf5-100")

INCLUDE(CPack)

add_custom_target(uninstall
		COMMAND xargs rm < install_manifest.txt && rm install_manifest.txt
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Uninstalling executable..."
		VERBATIM)



# Including extra cmake rules
#include(cmake/clang-dev-tools.cmake)
#include(cmake/doc.cmake)
