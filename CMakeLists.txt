include_directories(${PROJECT_INCLUDE_DIR})

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
#SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


# the RPATH to be used when installing, but only if it's not a system directory
#SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib:'$ORIGIN/lib'")
file(GLOB ARMO_HEADERS ${PROJECT_SOURCE_DIR}/ARMO/*.hpp)
file(GLOB ARMO_SOURCES ${PROJECT_SOURCE_DIR}/ARMO/*.cpp)


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
 set(Gravity_LIBRARIES ${THIRDPARTY_INSTALL_PATH}/Install/gravity/src/libgravity.a) 
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


add_executable(ARMO ${ARMO_SOURCES} ${ARMO_HEADERS})
target_link_libraries(ARMO gravity)
