if(CMAKE_XCODE_BUILD_SYSTEM VERSION_GREATER_EQUAL 12)
  cmake_policy(SET CMP0114 NEW)
else()
  cmake_policy(SET CMP0114 OLD)
endif()

# Create download URL derived from version number.
set(GRAVITY_DOWNLOAD_URL https://github.com/coin-or/Gravity.git)

# Download and build the GRAVITY library and add its properties to the third party arguments.
set(GRAVITY_ROOT_DIR ${THIRDPARTY_INSTALL_PATH}/Install/Gravity CACHE INTERNAL "")

if(WIN32)
# On Windows, download the Ipopt dlls from https://github.com/IDAES and the Gravity dlls from https://github.com/coin-or/Gravity
ExternalProject_Add(gravity
    DOWNLOAD_DIR ${THIRDPARTY_INSTALL_PATH}
    DOWNLOAD_COMMAND curl -k -L https://github.com/IDAES/idaes-ext/releases/download/2.4.1/idaes-solvers-windows-64.tar.gz -o idaes-solvers-windows-64.tar.gz && tar -xvzf idaes-solvers-windows-64.tar.gz -C ${GRAVITY_ROOT_DIR}
    URL ${GRAVITY_DOWNLOAD_URL}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${GRAVITY_ROOT_DIR}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)
set(IPOPT_ROOT_DIR ${THIRDPARTY_INSTALL_PATH}/Install/Gravity/thirdparty/Ipopt CACHE INTERNAL "")
add_custom_command(
  TARGET gravity POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy 
  ${GRAVITY_ROOT_DIR}/libipopt-3.dll ${PROJECT_SOURCE_DIR}/bin/Release/libipopt-3.dll)

add_custom_command(
TARGET gravity POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy 
${GRAVITY_ROOT_DIR}/libblas.dll ${PROJECT_SOURCE_DIR}/bin/Release/libblas.dll)

add_custom_command(
TARGET gravity POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy 
${GRAVITY_ROOT_DIR}/liblapack.dll ${PROJECT_SOURCE_DIR}/bin/Release/liblapack.dll)

add_custom_command(
TARGET gravity POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy 
${GRAVITY_ROOT_DIR}/libgfortran-5.dll ${PROJECT_SOURCE_DIR}/bin/Release/libgfortran-5.dll)

else()
ExternalProject_Add(gravity
    DOWNLOAD_DIR ${THIRDPARTY_INSTALL_PATH}
    DOWNLOAD_COMMAND export HTTPS_PROXY=$ENV{HTTPS_PROXY} && git clone -b Align --single-branch ${GRAVITY_DOWNLOAD_URL} && cd ./Gravity && mkdir build && cd build && cmake .. -DGJK=${GJK} -DEigen3=${Eigen3} -DOpenMPI=${OpenMPI} -DGurobi=${Gurobi} -DIpopt=${Ipopt} && make gravity
    URL ${MP_DOWNLOAD_URL}
    CMAKE_ARGS "${CMAKE_ARGS};-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)
endif()
set(GRAVITY_INCLUDE_DIRS ${THIRDPARTY_INSTALL_PATH}/Gravity/include)
include_directories(${GRAVITY_INCLUDE_DIRS})
link_directories(${THIRDPARTY_INSTALL_PATH}/Gravity/lib)
# if(APPLE)
set(GRAVITY_LIBRARY ${THIRDPARTY_INSTALL_PATH}/Gravity/lib/libgravity.a)
# elseif(UNIX)
# set(GRAVITY_LIBRARY ${THIRDPARTY_INSTALL_PATH}/Gravity/lib/libgravity.so)
# endif(APPLE)
set(LIBS ${LIBS} ${GRAVITY_LIBRARY})

list(APPEND GLOBAL_THIRDPARTY_LIB_ARGS "-DGRAVITY_ROOT_DIR:PATH=${GRAVITY_ROOT_DIR}")



