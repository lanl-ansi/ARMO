set(VORO_ROOT_DIR "$ENV{VORO_ROOT_DIR}" CACHE PATH "VORO root directory.")
message("Looking for VORO in ${GRAVITY_ROOT_DIR}/build/third_party/Install/voro/src")


find_path(VORO_INCLUDE_DIR
	NAMES voro++.hh
	HINTS /usr/local/include/VORO
	HINTS ${VORO_ROOT_DIR}/src
	HINTS ${GRAVITY_ROOT_DIR}/build/third_party/Install/voro/src
	HINTS ${VORO_ROOT_DIR}
)


find_library(VORO_LIBRARY 
	libvoro++.a
	HINTS ${VORO_ROOT_DIR}/src
	HINTS ${VORO_ROOT_DIR}/lib
	HINTS ${GRAVITY_ROOT_DIR}/build/third_party/Install/voro/src
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VORO DEFAULT_MSG VORO_LIBRARY VORO_INCLUDE_DIR)

if(VORO_FOUND)
	message("—- Found VORO under ${VORO_INCLUDE_DIR}")
    set(VORO_INCLUDE_DIRS ${VORO_INCLUDE_DIR})
    set(VORO_LIBRARIES ${VORO_LIBRARY})
endif(VORO_FOUND)

mark_as_advanced(VORO_LIBRARIES VORO_INCLUDE_DIR)
