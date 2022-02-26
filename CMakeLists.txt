cmake_minimum_required(VERSION 3.2)

project(ARMO)

include_directories(${PROJECT_INCLUDE_DIR})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
set(MACOSX_RPATH 1) 
# Debug mode
option(Debug "Turn on debug mode" OFF)

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
#SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


set(THIRDPARTY_INSTALL_PATH ${CMAKE_BINARY_DIR}/third_party)

# the RPATH to be used when installing, but only if it's not a system directory
#SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib:'$ORIGIN/lib'")

#if (NOT Matplot)
#  list(REMOVE_ITEM HEADERS ${PROJECT_INCLUDE_DIR}/matplotlibcpp.h)
#endif()

# Find IPOPT (optional)
option(Ipopt "Link to IPOPT libraries" OFF)

# Find GUROBI (optional)
option(Gurobi "Link to GUROBI libraries" OFF)

include(ExternalProject)
# Download Gravity (REQUIRED)
message(STATUS "I will download GRAVITY library from https://github.com/coin-or/Gravity")
include(ExternalGravity)
set(GRAVITY_INCLUDE_DIRS ${THIRDPARTY_INSTALL_PATH}/Gravity/include)
include_directories(${GRAVITY_INCLUDE_DIRS})
link_directories(${THIRDPARTY_INSTALL_PATH}/Install/Gravity/lib)
# if(APPLE)
set(GRAVITY_LIBRARY ${THIRDPARTY_INSTALL_PATH}/Gravity/lib/libgravity.a)
# elseif(UNIX)
# set(GRAVITY_LIBRARY ${THIRDPARTY_INSTALL_PATH}/Gravity/lib/libgravity.so)
# endif(APPLE)
set(LIBS ${LIBS} ${GRAVITY_LIBRARY})

if (Gurobi)
    message(STATUS "Enable GUROBI")
    add_definitions(-DUSE_GUROBI)
    find_package(GUROBI)
    include_directories(${GUROBI_INCLUDE_DIRS})
    set(LIBS ${LIBS} ${GUROBI_LIBRARIES})
endif()


# Find IPOPT (REQUIRED)
if (Ipopt)
message(STATUS "Enable IPOPT")
add_definitions(-DUSE_IPOPT)
find_package(IPOPT QUIET)
if (IPOPT_FOUND)
 message("-- Found IPOPT: ${IPOPT_INCLUDE_DIRS}")
 include_directories(${IPOPT_INCLUDE_DIRS})
 set(LIBS ${LIBS} ${IPOPT_LIBRARIES})
else(IPOPT_FOUND)
 set(IPOPT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/thirdparty/Ipopt/include/coin)
# set(ADD_IPOPT TRUE)
if(APPLE)
 set(IPOPT_LIBRARIES ${PROJECT_SOURCE_DIR}/thirdparty/Ipopt/lib/libipopt.dylib)
elseif(UNIX)
 set(IPOPT_LIBRARIES ${PROJECT_SOURCE_DIR}/thirdparty/Ipopt/libipopt.so)
elseif(WIN32) 
 set(IPOPT_LIBRARIES ${PROJECT_SOURCE_DIR}/thirdparty/Ipopt/libipopt-3.lib)
endif(APPLE)
set(LIBS ${LIBS} ${IPOPT_LIBRARIES})
endif (IPOPT_FOUND)
endif (Ipopt)



# Find LASlib (REQUIRED)
include(ExternalLAStools)

# Find VORO (Optional)
option(VORO "Link to VORO libraries" OFF)
message(STATUS "Enable VORO")
add_definitions(-DUSE_VORO)
if (VORO_FOUND)
 message("-- Found VORO: ${VORO_INCLUDE_DIRS}")
 include_directories(${VORO_INCLUDE_DIRS})
 set(LIBS ${LIBS} ${VORO_LIBRARIES})
else(VORO_FOUND)
message(STATUS "Could not find VORO, check that VORO_ROOT_DIR=${VORO_ROOT_DIR} points to the right path")
endif()
include(ExternalProject)


# Compiler options
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O4 -Ofast")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,'\$ORIGIN/lib'")


if(LINUX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()
# Build options
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

set(CMAKE_BUILD_TYPE "Release")
if (Debug)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/Debug)
endif()

if(Cov)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fprofile-arcs -lgcov")
endif()


add_subdirectory(algos)

#Get user defined configuration including passing project source directory into c++ code
configure_file(ARMOConfig.h.in ${PROJECT_SOURCE_DIR}/algos/ARMOConfig.h)
