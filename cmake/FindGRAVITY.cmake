# Search supplied hint directories first if supplied.
set(GRAVITY_ROOT_DIR "$ENV{GRAVITY_ROOT_DIR}" CACHE PATH "GRAVITY root directory.")
if(GRAVITY_ROOT_DIR)
 message("Looking for GRAVITY in ${GRAVITY_ROOT_DIR}")
else(GRAVITY_ROOT_DIR)
 message("GRAVITY_ROOT_DIR not provided.")
endif(GRAVITY_ROOT_DIR)

find_path(GRAVITY_INCLUDE_DIR
  NAMES solver.h
  HINTS ${GRAVITY_ROOT_DIR}/include/gravity
  HINTS /usr/local/include
  HINTS /usr/include  
  HINTS ${THIRDPARTY_INSTALL_PATH}/Gravity/include/gravity
)
message("GRAVITY_INCLUDE_DIR  = ${GRAVITY_INCLUDE_DIR}.")
find_library(GRAVITY_LIBRARY 
	libgravity.a
	HINTS ${GRAVITY_ROOT_DIR}/lib
  HINTS ${GRAVITY_ROOT_DIR}/lib/Release
	HINTS /usr/local/lib
)
message("GRAVITY_LIBRARY  = ${GRAVITY_LIBRARY}.")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GRAVITY DEFAULT_MSG GRAVITY_LIBRARY GRAVITY_INCLUDE_DIR)

# Set standard CMake FindPackage variables if found.
if (GRAVITY_FOUND)
  set(GRAVITY_INCLUDE_DIRS ${GRAVITY_INCLUDE_DIR}/..)
  set(GRAVITY_LIBRARIES ${GRAVITY_LIBRARY})
else (GRAVITY_FOUND)
 message("Cannot find GRAVITY, will try pulling it from github.")
endif (GRAVITY_FOUND)
