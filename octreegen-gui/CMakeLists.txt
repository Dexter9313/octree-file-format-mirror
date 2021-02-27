#INIT
cmake_minimum_required(VERSION 3.0.0)

set(PROJECT_NAME octreegen-gui)
project(${PROJECT_NAME} C CXX)

#GET CMAKE_SYSTEM_NAME VARIABLE
include(../cmake/common/SystemName.cmake)
message(STATUS "Architecture : ${CMAKE_SYSTEM_NAME}")

#SET UP COMPILER
include(../cmake/common/SetupCompiler.cmake)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC") # Add this for Qt

#DEPENDENCIES
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/modules)

find_package(HDF5 REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Widgets)

message(STATUS "Qt5Widgets : ${Qt5Widgets_INCLUDE_DIRS}")

set(EXTERNAL_LIBS_INCLUDE_DIRS ${HDF5_INCLUDE_DIR} ${Qt5Widgets_INCLUDE_DIRS})

#FILES/DIRECTORIES

set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)
file(GLOB_RECURSE HPP_FILES ${PROJECT_SOURCE_DIR}/include/*.hpp)

file(GLOB_RECURSE MAIN_SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)

file(GLOB SRC_FILES ${MAIN_SRC_FILES})
list(REMOVE_ITEM SRC_FILES ${PROJECT_SOURCE_DIR}/src/main.cpp)
file(GLOB MAIN_FILE ${PROJECT_SOURCE_DIR}/src/main.cpp)


#INCLUDE

set(SYSTEM_INCLUDES ${EXTERNAL_LIBS_INCLUDE_DIRS})
include_directories(${COMMON_INCLUDES} SYSTEM ${SYSTEM_INCLUDES})

#LINKING SETUP
set(LD_LIBS ${HDF5_LIBRARIES} ${Qt5Widgets_LIBRARIES})

#RUN MOC (FOR QT5)
qt5_wrap_cpp(MOC_FILES ${HPP_FILES})

#EXECUTABLE : octreegen-gui
add_executable(${PROJECT_NAME} ${MAIN_FILE} ${SRC_FILES} ${MOC_FILES})
target_link_libraries(${PROJECT_NAME} ${LD_LIBS})


INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} DESTINATION bin)

SET(CPACK_GENERATOR "DEB")
SET(CPACK_PACKAGE_NAME ${PROJECT_NAME})
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A user-friendly GUI interface for octreegen.")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Florian Cabot <florian.cabot@epfl.ch>")
if(TAG)
	SET(CPACK_PACKAGE_VERSION ${TAG})
endif()
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libhdf5-103, libqt5widgets5, octreegen")

INCLUDE(CPack)

#Create uninstall target
include(../cmake/common/Uninstall.cmake)

# Including extra cmake rules
#include(cmake/clang-dev-tools.cmake)
#include(cmake/doc.cmake)
