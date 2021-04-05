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

set_directory_properties(PROPERTIES EP_BASE ${CMAKE_BINARY_DIR}/third_party)
get_directory_property(THIRDPARTY_BASE_PATH EP_BASE)

set(THIRDPARTY_INSTALL_PATH ${THIRDPARTY_BASE_PATH})

# the RPATH to be used when installing, but only if it's not a system directory
#SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib:'$ORIGIN/lib'")
file(GLOB ARMO_HEADERS ${PROJECT_SOURCE_DIR}/ARMO/*.hpp, ${PROJECT_SOURCE_DIR}/ARMO/*.h)
file(GLOB ARMO_SOURCES ${PROJECT_SOURCE_DIR}/*.cpp)


#if (NOT Matplot)
#  list(REMOVE_ITEM HEADERS ${PROJECT_INCLUDE_DIR}/matplotlibcpp.h)
#endif()

# Find GUROBI (optional)
option(Gurobi "Link to GUROBI libraries" OFF)

# Find IPOPT (REQUIRED)
option(Ipopt "Link to IPOPT libraries" ON)

# Find LASlib (REQUIRED)
option(LASlib "Link to LASlib libraries" ON)

# Find QHULL (OPTIONAL)
option(QHULL "Link to QHULL libraries" OFF)

# Find VORO (REQUIRED)
option(VORO "Link to VORO libraries" ON)

# Find Gravity (REQUIRED)
option(Gravity "Link to Gravity libraries" ON)

include(ExternalProject)

# Find gurobi?
if (Gurobi)
    message(STATUS "Enable GUROBI")
    add_definitions(-DUSE_GUROBI)
    find_package(GUROBI)
    include_directories(${GUROBI_INCLUDE_DIRS})
    set(LIBS ${LIBS} ${GUROBI_LIBRARIES})
endif()

# Find Ipopt.
if (Ipopt)
message(STATUS "Enable IPOPT")
add_definitions(-DUSE_IPOPT)
find_package(IPOPT QUIET)
if (IPOPT_FOUND)
 message("-- Found IPOPT: ${IPOPT_INCLUDE_DIRS}")
 include_directories(${IPOPT_INCLUDE_DIRS})
 set(LIBS ${LIBS} ${IPOPT_LIBRARIES})
else(IPOPT_FOUND)
set(ADD_IPOPT TRUE)
if(APPLE)
 set(IPOPT_LIBRARIES ${THIRDPARTY_INSTALL_PATH}/Install/ipopt/build/lib/libipopt.dylib)
elseif(UNIX)
 set(IPOPT_LIBRARIES ${THIRDPARTY_INSTALL_PATH}/Install/ipopt/build/lib/libipopt.so)
endif(APPLE)
 include(ExternalIpopt)
endif (IPOPT_FOUND)
endif()


if (VORO)
message(STATUS "Enable VORO")
add_definitions(-DUSE_VORO)
find_package(VORO QUIET)
if (VORO_FOUND)
 message("-- Found VORO: ${VORO_INCLUDE_DIRS}")
 include_directories(${VORO_INCLUDE_DIRS})
 set(LIBS ${LIBS} ${VORO_LIBRARIES})
else(VORO_FOUND)
 set(ADD_VORO TRUE)
 message(STATUS "Will download VORO library from http://math.lbl.gov/voro++/download/")
 include(ExternalVORO)
 set(VORO_LIBRARIES ${THIRDPARTY_INSTALL_PATH}/Install/voro/src/libvoro++.a) 
endif()
endif()

if (Gravity)
message(STATUS "Enable Gravity")
add_definitions(-DUSE_Gravity)
find_package(Gravity QUIET)
if (Gravity_FOUND)
 message("-- Found Gravity: ${Gravity_INCLUDE_DIRS}")
 include_directories(${Gravity_INCLUDE_DIRS})
 set(LIBS ${LIBS} ${Gravity_LIBRARIES})
else(Gravity_FOUND)
 set(ADD_Gravity TRUE)
 message(STATUS "Will download Gravity library from https://github.com/coin-or/Gravity.git")
 include(ExternalGravity)
 set(Gravity_LIBRARIES ${THIRDPARTY_INSTALL_PATH}/Install/gravity/lib/libgravity.a) 
endif()
endif()

if (LASlib)
message(STATUS "Enable LASlib")
add_definitions(-DUSE_LASlib)
find_package(LASlib QUIET)
if (LASlib_FOUND)
 message("-- Found LASlib: ${LASlib_INCLUDE_DIRS}")
 include_directories(${LASlib_INCLUDE_DIRS})
 set(LIBS ${LIBS} ${LASlib_LIBRARIES})
else(LASlib_FOUND)
 set(ADD_LASlib TRUE)
 message(STATUS "Will download LASlib library from https://github.com/LAStools/LAStools")
 include(ExternalLASTOOLS)
 set(LASlib_LIBRARIES ${THIRDPARTY_INSTALL_PATH}/Install/lastools/LASlib/lib/liblas.a)
endif()
endif()

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

add_executable(ARMO ${ARMO_SOURCES} ${ARMO_HEADERS})

if(ADD_EIGEN)
 add_dependencies(ARMO eigen)
endif(ADD_EIGEN)
if(ADD_GoICP)
 add_dependencies(ARMO GoICP)
endif(ADD_GoICP)

if(ADD_QHULL)
 add_dependencies(ARMO qhull)
endif()

if(ADD_VORO)
 add_dependencies(ARMO voro)
endif()

add_dependencies(ARMO gravity)

if(OPT_PARSER)
add_dependencies(ARMO opt_parser)
endif()
target_link_libraries(ARMO ${LASlib_LIBRARIES} ${QHULL_LIBRARIES} ${VORO_LIBRARIES} ${Gravity_LIBRARIES} ${IPOPT_LIBRARIES} ${GUROBI_LIBRARIES})

